#ifndef VIRUS_VIRUS_GENEALOGY_H
#define VIRUS_VIRUS_GENEALOGY_H

#include <memory>
#include <map>
#include <exception>
#include <vector>

class VirusAlreadyCreated : public std::exception {
    const char*  what() const override {
        return "VirusAlreadyCreated";
    }
};

class VirusNotFound : public std::exception {
    const char* what() const override {
        return "VirusNotFound";
    }
};

class TriedToRemoveStemVirus : public std::exception {
    const char* what() const override {
        return "TriedToRemoveStemVirus";
    }
};

template <typename Virus>
class VirusGenealogy {
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
    std::vector<id_type> get_childen(const id_type& id) const;

    //
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


  private:
    typedef typename Virus::id_type id_type;
    typedef std::shared_ptr<Node> node_sh_ptr;
    typedef std::weak_ptr<Node> node_w_ptr;

    // TODO ustalic kolejnosc, moze byc istotna dla kolejnosci destrukcji

    // Node in viruses' graph
    class Node;

    // All nodes mapped to their viruses' ids
    std::map<id_type, node_w_ptr> nodes;

    // Shared pointer to stem virus' node
    node_sh_ptr stem_node_ptr;



};

template <typename Virus>
class VirusGenealogy::Node {
    friend class VirusGenealogy<Virus>;
  public:
    Node(const id_type& id)
        : value{id},
          self_map_iter{}
    {}

    // Moze rzucac wyjatkiem takim jak getId() wirusa, czyli ok
    id_type getId() const {
        return value->getId();
    }
  private:
    //TODO ustalic kolejnosc

    std::unique_ptr<Virus> value;

    std::map<id_type, node_sh_ptr> children{};

    std::map<id_type, node_w_ptr> parents{};


    // Node's iterator in Genealogy's node map
    std::map<id_type, node_w_ptr>::iterator self_map_iter;
};


// Moze byc wyjatek braku pamieci, wowczas destruktory odpowiednich atrybutow wyczyszcza pamiec
template <typename Virus>
VirusGenealogy::VirusGenealogy(const id_type& stem_id)
    : nodes{},stem_node_ptr{std::make_shared<Node>(stem_id)}
{}

// Moze rzucac wyjatkiem, patrz Node
template <typename Virus>
id_type VirusGenealogy::get_stem_id() const
{
    return stem_node_ptr->getId();
}

template <typename Virus>
std::vector<id_type> VirusGenealogy::get_childen(const id_type& id) const {

}

template <typename Virus>
std::vector<id_type> VirusGenealogy::get_parents(const id_type& id) const {

}

// Mapa moze rzucac wyjatkiem, to nic
bool VirusGenealogy::exists(const id_type& id) const {
    return nodes.count(id) != 0;
}

// Wyjatkiem moze rzucic exist, throw, mapa[].
// lock() i * nie rzuca
template <typename Virus>
Virus& VirusGenealogy::operator[](const id_type& id) const {
    if(!exists(id)) throw VirusNotFound{};
    return *(nodes[id].lock());
}

//
void VirusGenealogy::create(const id_type& id, const id_type& parent_id) {

}

//
void VirusGenealogy::create(const id_type& id, const std::vector<id_type>& parent_ids) {

}

//
void VirusGenealogy::connect(const id_type& child_id, const id_type& parent_id) {

}

//
void VirusGenealogy::remove(const id_type& id) {

}



#endif //VIRUS_VIRUS_GENEALOGY_H
