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
    VirusGenealogy(const typename Virus::id_type& stem_id);

    //
    typename Virus::id_type get_stem_id() const;

    //
    std::vector<typename Virus::id_type> get_childen(const typename Virus::id_type& id) const;

    //
    std::vector<typename Virus::id_type> get_parents(const typename Virus::id_type& id) const;

    //
    bool exists(const typename Virus::id_type& id) const;

    //
    Virus& operator[](const typename Virus::id_type& id) const;

    //
    void create(const typename Virus::id_type& id, const typename Virus::id_type& parent_id);

    //
    void create(const typename Virus::id_type& id, const std::vector<typename Virus::id_type>& parent_ids);

    //
    void connect(const typename Virus::id_type& child_id, const typename Virus::id_type& parent_id);

    //
    void remove(const typename Virus::id_type& id);


  private:
};

#endif //VIRUS_VIRUS_GENEALOGY_H
