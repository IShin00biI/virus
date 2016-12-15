#ifndef VIRUS_VIRUS_GENEALOGY_H
#define VIRUS_VIRUS_GENEALOGY_H

#include <memory>
#include <map>
#include <exception>
#include <vector>
#include <utility>
#include <cstddef>

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
    // Wierzcholek grafu wirusow
    class Node;

    // Aliasy uzywanych typow
    typedef typename Virus::id_type id_type;

    typedef typename std::shared_ptr<Node> node_sh_ptr;
    typedef typename std::weak_ptr<Node> node_w_ptr;

    typedef typename std::map<id_type, node_sh_ptr> children_map;
    typedef typename children_map::iterator children_iterator;

    typedef typename
    std::map<id_type, std::pair<node_w_ptr, children_iterator> > parents_map;
    typedef typename parents_map::iterator parents_iterator;

    typedef typename std::map<id_type, node_w_ptr> global_map;
    typedef typename global_map::iterator global_iterator;

    // Globalna mapa wskaznikow do wirusow
    global_map nodes;

    // Wskaznik do wirusa macierzystego
    node_sh_ptr stem_node_ptr;

    // Funkcja usuwajaca z grafu wirus pod danym wskaznikiem
    void remove(node_sh_ptr& node_ptr) noexcept;

  public:

    // Usuniety konstruktor kopiujacy
    VirusGenealogy(const VirusGenealogy<Virus>&) = delete;

    // Usuniety kopiujący operator przypisania
    VirusGenealogy<Virus>& operator=(const VirusGenealogy<Virus>&) = delete;

    // Tworzy nowa genealogie.
    // Tworzy takze wezel wirusa macierzystego o identyfikatorze stem_id.
    VirusGenealogy(const id_type& stem_id);

    // Zwraca identyfikator wirusa macierzystego.
    id_type get_stem_id() const;

    // Zwraca liste identyfikatorow bezposrednich nastepnikow wirusa
    // o podanym identyfikatorze.
    // Zglasza wyjatek VirusNotFound, jesli dany wirus nie istnieje.
    std::vector<id_type> get_children(const id_type& id) const;

    // Zwraca liste identyfikatorow bezposrednich poprzednikow wirusa
    // o podanym identyfikatorze.
    // Zglasza wyjatek VirusNotFound, jesli dany wirus nie istnieje.
    std::vector<id_type> get_parents(const id_type& id) const;

    // Sprawdza, czy wirus o podanym identyfikatorze istnieje.
    bool exists(const id_type& id) const;

    // Zwraca referencje do obiektu reprezentujacego wirus o podanym
    // identyfikatorze.
    // Zglasza wyjatek VirusNotFound, jesli zadany wirus nie istnieje.
    Virus& operator[](const id_type& id) const;

    // Tworzy wezel reprezentujacy nowy wirus o identyfikatorze id
    // powstaly z wirusow o podanym identyfikatorze parent_id.
    // Zglasza wyjatek VirusAlreadyCreated, jesli wirus o identyfikatorze
    // id juz istnieje.
    // Zglasza wyjatek VirusNotFound, jesli wyspecyfikowany poprzednik nie istnieje.
    void create(const id_type& id, const id_type& parent_id);

    // Tworzy wezel reprezentujacy nowy wirus o identyfikatorze id
    // powstaly z wirusow o podanych identyfikatorach parent_ids.
    // Zglasza wyjatek VirusAlreadyCreated, jesli wirus o identyfikatorze
    // id już istnieje.
    // Zglasza wyjatek VirusNotFound, jesli którys z wyspecyfikowanych
    // poprzednikow nie istnieje.
    void create(const id_type& id, const std::vector<id_type>& parent_ids);

    // Dodaje nowa krawedz w grafie genealogii.
    // Zglasza wyjatek VirusNotFound, jesli ktorys z podanych wirusow nie istnieje.
    void connect(const id_type& child_id, const id_type& parent_id);

    // Usuwa wirus o podanym identyfikatorze.
    // Zglasza wyjatek VirusNotFound, jesli zadany wirus nie istnieje.
    // Zglasza wyjatek TriedToRemoveStemVirus przy probie usuniecia
    // wirusa macierzystego.
    void remove(const id_type& id);

};


template <typename Virus>
class VirusGenealogy<Virus>::Node {

    friend class VirusGenealogy<Virus>;
  public:

    // Konstruktor
    Node(const id_type& id, global_map& nodes)
        : value{std::make_unique<Virus>(id)},
          nodes{nodes}
    {
        self_map_iter = nodes.end();
    }

    // Destruktor.
    ~Node() {
        if(self_map_iter != nodes.end()) nodes.erase(self_map_iter);
    }

    // Zwraca id przechowywanego wirusa
    id_type get_id() const {
        return value->get_id();
    }

    // Ustawiaja iterator wirusa w mapie globalnej
    void set_global_iter(global_iterator& iter) {
        self_map_iter = iter;
    }
    void set_global_iter(global_iterator&& iter) {
        self_map_iter = iter;
    }

  private:
    // Wskaznik na wirusa w wierzcholku
    std::unique_ptr<Virus> value;

    // Mapa nastepnikow wierzcholka
    children_map children{};

    // Mapa poprzednikow wierzcholka
    parents_map parents{};

    // Referencja do globalnej mapy wierzcholkow
    global_map& nodes;

    // Iterator node'a w globalnej mapie wierzcholkow
    global_iterator self_map_iter{};

};


template <typename Virus>
void VirusGenealogy<Virus>::remove(node_sh_ptr& node_ptr) noexcept {
    for(parents_iterator iter = node_ptr->parents.begin();
        iter != node_ptr->parents.end(); ++iter) {

        if(!iter->second.first.expired()) {
            node_sh_ptr locked_parent = iter->second.first.lock();
            locked_parent->children.erase(iter->second.second);
        }
    }
}


template <typename Virus>
VirusGenealogy<Virus>::VirusGenealogy(const id_type& stem_id)
    : nodes{},
      stem_node_ptr{std::make_shared<Node>(stem_id, nodes)}
{
    global_iterator stem_iter =
        nodes.insert(std::make_pair(stem_id, node_w_ptr{stem_node_ptr})).first;
    stem_node_ptr->set_global_iter(stem_iter);
}


template <typename Virus>
typename Virus::id_type VirusGenealogy<Virus>::get_stem_id() const {
    return stem_node_ptr->value->get_id();
}


template <typename Virus>
std::vector<typename Virus::id_type>
VirusGenealogy<Virus>::get_children(const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    children_map& children = nodes.at(id).lock()->children;
    std::vector<id_type> result(children.size());
    size_t result_index = 0;
    for(children_iterator child = children.begin();
        child != children.end(); ++child) {

        result[result_index] = child->first;
        ++result_index;
    }
    return std::move(result);
}


template <typename Virus>
std::vector<typename Virus::id_type>
VirusGenealogy<Virus>::get_parents(const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    parents_map& parents = nodes.at(id).lock()->parents;
    std::vector<id_type> result;
    result.reserve(parents.size());
    parents_iterator parent = parents.begin();
    while(parent != parents.end()) {
        if(!parent->second.first.expired()) {
            result.push_back(parent->first);
            ++parent;
        }
        else parent = parents.erase(parent);
    }

    return std::move(result);
}


template <typename Virus>
bool VirusGenealogy<Virus>::exists(const id_type& id) const {
    return nodes.count(id) != 0;
}


template <typename Virus>
Virus& VirusGenealogy<Virus>::operator[](const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    return *(nodes.at(id).lock()->value);
}

//
template <typename Virus>
void VirusGenealogy<Virus>::create(const id_type& id, const id_type& parent_id) {
    create(id, std::vector<id_type>{1, parent_id});
}


template <typename Virus>
void VirusGenealogy<Virus>::create
    (const id_type& id, const std::vector<id_type>& parent_ids) {
    if(exists(id)) throw VirusAlreadyCreated{};
    for(size_t i = 0; i < parent_ids.size(); ++i)
        if(!exists(parent_ids[i])) throw VirusNotFound{};

    node_sh_ptr new_node_ptr = std::make_shared<Node>(id, nodes);
    new_node_ptr->set_global_iter
        (nodes.insert
            (std::pair<id_type, node_w_ptr>
                 {id, node_w_ptr{new_node_ptr}}).first);

    children_iterator recent_child_iter{};
    node_sh_ptr parent_ptr;

    for(size_t i = 0; i < parent_ids.size(); ++i) {
        try {
            parent_ptr = nodes.at(parent_ids[i]).lock();
            recent_child_iter = (parent_ptr->children.insert
                (std::pair<id_type, node_sh_ptr>
                     {id, node_sh_ptr{new_node_ptr}})).first;

            try {
                new_node_ptr->parents.insert
                    (std::pair<id_type, std::pair<node_sh_ptr, children_iterator> >
                         {parent_ids[i], std::pair<node_sh_ptr, children_iterator>
                             {parent_ptr, recent_child_iter}});
            }
            catch (std::exception& e) {
                parent_ptr->children.erase(recent_child_iter);
                throw;
            }
        }
        catch(std::exception& e) {
            remove(new_node_ptr);
        }
    }

}


template <typename Virus>
void VirusGenealogy<Virus>::connect
    (const id_type& child_id, const id_type& parent_id) {
    if(!exists(child_id) || !exists(parent_id)) throw VirusNotFound{};

    node_sh_ptr child_node_ptr = nodes.at(child_id).lock();
    node_sh_ptr parent_node_ptr = nodes.at(parent_id).lock();

    if(parent_node_ptr->children.find(child_id) !=
       parent_node_ptr->children.end()) return;

    parents_iterator old_parent_iter =
        child_node_ptr->parents.find(parent_id);
    if(old_parent_iter != child_node_ptr->parents.end())
        child_node_ptr->parents.erase(old_parent_iter);

    children_iterator added_child_iter =
        parent_node_ptr->children.insert
            (std::make_pair(child_id, node_sh_ptr{child_node_ptr})).first;

    try {
        child_node_ptr->parents.insert
            (std::make_pair(parent_id, std::pair<node_w_ptr, children_iterator>
                (node_w_ptr(parent_node_ptr), added_child_iter)));
    }
    catch(std::exception& e) {
        parent_node_ptr->children.erase(added_child_iter);
        throw;
    }
}


template <typename Virus>
void VirusGenealogy<Virus>::remove(const id_type& id) {
    if(!exists(id)) throw VirusNotFound{};
    if(get_stem_id() == id) throw TriedToRemoveStemVirus{};

    node_sh_ptr node_ptr = nodes.at(id).lock();

    remove(node_ptr);
}

#endif //VIRUS_VIRUS_GENEALOGY_H