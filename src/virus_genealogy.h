#ifndef VIRUS_VIRUS_GENEALOGY_H
#define VIRUS_VIRUS_GENEALOGY_H

#include <memory>
#include <map>
#include <exception>
#include <vector>

class VirusAlreadyCreated : public std::exception {
    virtual const char* what() const noexcept override {
        return "VirusAlreadyCreated";
    }
};

class VirusNotFound : public std::exception {
    virtual const char* what() const noexcept override {
        return "VirusNotFound";
    }
};

class TriedToRemoveStemVirus : public std::exception {
    virtual const char* what() const noexcept override {
        return "TriedToRemoveStemVirus";
    }
};

template <typename Virus>
class VirusGenealogy {

  private:
    // Node in viruses' graph
    class Node;

    typedef typename Virus::id_type id_type;
    typedef std::shared_ptr<Node> node_sh_ptr;
    typedef std::weak_ptr<Node> node_w_ptr;
    typedef std::map<id_type, node_sh_ptr> children_map;
    typedef std::map<id_type, node_w_ptr> parents_map;
    typedef parents_map global_map;
    typedef typename children_map::iterator children_iterator;
    typedef typename parents_map::iterator parents_iterator;
    typedef typename global_map::iterator global_iterator;

    // TODO ustalic kolejnosc, moze byc istotna dla kolejnosci destrukcji

    // All nodes mapped to their viruses' ids
    global_map nodes;

    // Shared pointer to stem virus' node
    node_sh_ptr stem_node_ptr;

  public:

    // Copy constructor deleted
    VirusGenealogy(const VirusGenealogy<Virus>&) = delete;

    // Copy assignment deleted
    VirusGenealogy<Virus>& operator=(const VirusGenealogy<Virus>&) = delete;

    // Constructor
    VirusGenealogy(const id_type& stem_id);

    //
    id_type get_stem_id() const;

    //
    std::vector<id_type> get_children(const id_type& id) const;


    std::vector<id_type> get_parents(const id_type& id) const;

    //
    bool exists(const id_type& id) const;

    //
    Virus& operator[](const id_type& id) const;

    //
    void create(const id_type& id, const id_type& parent_id);

    //
    void create(const id_type& id, const std::vector<id_type>& parent_ids);

    //
    void connect(const id_type& child_id, const id_type& parent_id);

    //
    void remove(const id_type& id);




};


/*
 * TODO ta klasa jest super istotna, zaleznie od memberow, konstruktorow i destruktorow
 * wplywa na sposob pracy z wieloma funkcjami, mysle ze dobrze tu zrobione RAII moze zalatwic wiekszosc
 * roboty, ale nie udalo mi sie go zrobic wedlug mojego planu.
 * children, parents, value, get_id sa ok. Cala reszta do przerobienia
 *
 */
template <typename Virus>
class VirusGenealogy<Virus>::Node {
    friend class VirusGenealogy<Virus>;
  public:
    /*
    Node(const id_type& id, global_map& nodes)
        : value{id},
          nodes{nodes},
          self_map_iter{this->nodes.end()}
    {}

    ~Node() {
        if(self_map_iter != nodes.end()) nodes.erase(self_map_iter);
    }*/

    // Moze rzucac wyjatkiem takim jak get_id() wirusa, czyli ok
    id_type get_id() const {
        return value->get_id();
    }
  private:
    //TODO ustalic kolejnosc

    std::unique_ptr<Virus> value;

    children_map children{};

    parents_map parents{};

    /*Probowalem zrobic w node'ach iteratory do swoich miejsc w global_mapie,
     * tak jak mozna wnioskowac po konstruktorze i destruktorze, ale nie udalo sie,
     * self_map_iter{this->nodes.end()} sie wywala. Trzeba sie zastanowic jak zrobic
     * wyrzucanie z global_mapy(nie mozna tego zostawiac).
     *
    global_map& nodes;

    // Node's iterator in Genealogy's node map
    global_iterator self_map_iter;
     */
};


// TODO konstruktor grafu tez musi byc przemyslany, zeby wspolgral
// z konstruktorem node'a i kolejnoscia konstrukcji
template <typename Virus>
VirusGenealogy<Virus>::VirusGenealogy(const id_type& stem_id)
    //: nodes{},
      //stem_node_ptr{std::make_shared<Node>(stem_id, nodes)}
{}

// Moze rzucac wyjatkiem, patrz Node(raczej gotowe)
template <typename Virus>
typename Virus::id_type VirusGenealogy<Virus>::get_stem_id() const {
    return stem_node_ptr->value->get_id();
}

// Jest silna odpornosc(raczej gotowe)
template <typename Virus>
std::vector<typename Virus::id_type>
VirusGenealogy<Virus>::get_children(const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    children_map& children = nodes.at(id).lock()->children;
    std::vector<id_type> result(children.size());
    size_t result_index = 0;
    for(children_iterator child = children.begin(); child != children.end(); ++child) {
        result[result_index] = child->first;
        ++result_index;
    }
    return std::move(result);
}

// Jest silna odporność. Usuwamy tu martwe weak_ptry.(raczej gotowe)
// mozna pomyslec czy gdzies jeszcze czyscimy
template <typename Virus>
std::vector<typename Virus::id_type>
VirusGenealogy<Virus>::get_parents(const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    parents_map& parents = nodes.at(id).lock()->parents;
    std::vector<id_type> result;
    result.reserve(parents.size());
    parents_iterator parent = parents.begin();
    while(parent != parents.end()) {
        if(!parent->second.expired()) {
            result.push_back(parent->first);
            ++parent;
        }
        else parent = parents.erase(parent);
        // CLionowi nie podoba sie powyzsza instrukcja, bo skasowalem operator= dla genealogii.
        // Nie widze zwiazku, mozliwe ze bug IDE
    }

    return std::move(result);
}

// Mapa moze rzucac wyjatkiem, to nic
// TODO TA WERSJA DZIALA TYLKO PRZY USUWANIU Z GLOBAL_MAPY NA BIEZACO
// MOCNO ZALEZY OD NASZYCH NIEZMIENNIKOW
template <typename Virus>
bool VirusGenealogy<Virus>::exists(const id_type& id) const {
    return nodes.count(id) != 0;
}

// Wyjatkiem moze rzucic exist, throw, at.
// lock() i * nie rzuca(raczej gotowe)
template <typename Virus>
Virus& VirusGenealogy<Virus>::operator[](const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    return *(nodes.at(id).lock()->value);
}

//
template <typename Virus>
void VirusGenealogy<Virus>::create(const id_type& id, const id_type& parent_id) {
    if(exists(id)) throw VirusAlreadyCreated{};

}

// TODO Trzeba sprawdzic informacje z forum o mozliwych wyjatkach w porownaniach, wplywaja na gwarancje map
// TODO Jeden create powinien korzystac z drugiego.
// TODO Trzeba sie zastanowic nad silna gwarancja, rowniez w zaleznosci od porownan.
//Zwykly idiom pracy na kopii(temp&swap) wydaje sie byc ciezki, bo zmiany obejmuja rozne mapy
template <typename Virus>
void VirusGenealogy<Virus>::create(const id_type& id, const std::vector<id_type>& parent_ids) {
    if(exists(id)) throw VirusAlreadyCreated{};

}

// TODO powinno nie byc trudne, ale watpie zeby sie przydalo do create
template <typename Virus>
void VirusGenealogy<Virus>::connect(const id_type& child_id, const id_type& parent_id) {

}

// TODO latwe z bezpiecznymi porownaniami, bo erase jest wtedy nothrow. Z niepewnymi jest gorzej..
template <typename Virus>
void VirusGenealogy<Virus>::remove(const id_type& id) {
}



#endif //VIRUS_VIRUS_GENEALOGY_H
