#pragma once

#include <exception>
#include <functional>
#include <iterator>
#include <list>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    typedef std::list<std::pair<const KeyType, ValueType>> Cell;

    HashMap(Hash h = Hash()): hasher(h) {}
    template <class Iter>
    HashMap(const Iter& begin, const Iter& end, Hash h = Hash()): hasher(h) {
        size_t cnt = std::distance(begin, end);
        table.resize(cnt << 1, end_table);
        for (Iter it = begin; it != end; ++it)
            _insert(*it);
    }
    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init, Hash h = Hash()): hasher(h) {
        table.resize(init.size() << 1, end_table);
        for (const auto& it : init)
            _insert(it);
    }
    HashMap(const HashMap& mp) {
        table.resize(mp.size() << 1, end_table);
        for (const auto& it : mp)
            _insert(it);
    }
    HashMap& operator=(const HashMap& other) {
        if (&other == this) {
            return *this;
        }
        table.clear();
        table.resize(other.size() << 1, end_table);
        for (Cell* link : links) {
            delete link;
        }
        len = 0;
        links.clear();
        for (auto it : other)
            _insert(it);
        return *this;
    }
    ~HashMap() {
        for (Cell* link : links) {
            delete link;
        }
        delete list_iter;
        delete list_iter_end;
        links.clear();
    }

    size_t size() const {
        return len;
    }
    bool empty() const {
        return len == 0;
    }
    Hash hash_function() const {
        return hasher;
    }

    class iterator : public std::iterator<std::input_iterator_tag, ValueType> {
        std::list<Cell*>* links;
        typename std::list<Cell*>::iterator link;
        typename Cell::iterator elem;
        friend void HashMap::erase(const KeyType& key);
    public:
        iterator() : links(nullptr) {}
        iterator(std::list<Cell*>* links, typename std::list<Cell*>::iterator link, typename Cell::iterator elem) : links(links), link(link), elem(elem) {}
        std::pair<const KeyType, ValueType>& operator*() {
            return *elem;
        }
        typename Cell::iterator operator->() {
            return elem;
        }
        iterator& operator++() {
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return *this;
        }
        iterator operator++(int) {
            iterator buff = *this;
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return buff;
        }
        bool operator!=(const iterator& other) {
            return links != other.links || link != other.link ||
                   (link != links->end() && *elem != *(other.elem));
        }
        bool operator==(const iterator& other) {
            return links == other.links && link == other.link &&
                   (link == links->end() || *elem == *(other.elem));
        }
    };
    iterator begin() {
        return {&links, links.begin(), links.empty() ? end_iter : (*links.begin())->begin()};
    }
    iterator end() {
        return {&links, links.end(), end_iter};
    }

    class const_iterator : public std::iterator<std::input_iterator_tag, ValueType> {
        const std::list<Cell*>* links;
        typename std::list<Cell*>::const_iterator link;
        typename Cell::const_iterator elem;
    public:
        const_iterator() : links(nullptr) {}
        const_iterator(const std::list<Cell*>* links, typename std::list<Cell*>::const_iterator link, typename Cell::const_iterator elem) : links(links), link(link), elem(elem) {}
        const std::pair<const KeyType, ValueType>& operator*() const {
            return *elem;
        }
        typename Cell::const_iterator operator->() {
            return elem;
        }
        const_iterator& operator++() {
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator buff = *this;
            ++elem;
            if (elem == (*link)->end()) {
                ++link;
                if (link != links->end())
                    elem = (*link)->begin();
            }
            return buff;
        }
        bool operator!=(const const_iterator& other) {
            return links != other.links || link != other.link ||
                   (link != links->end() && elem != other.elem);
        }
        bool operator==(const const_iterator& other) {
            return links == other.links && link == other.link &&
                   (link == links->end() || elem == other.elem);
        }
    };
    const_iterator begin() const {
        return {&links, links.begin(), links.empty() ? end_const_iter : (*(links.begin()))->begin()};
    }
    const_iterator end() const {
        return {&links, links.end(), end_const_iter};
    }

    void insert(const std::pair<KeyType, ValueType>& pr) {
        _insert(pr);
        _check_len();
    }
    void erase(const KeyType& key) {
        iterator ind = find(key);
        if (ind == end())
            return;
        (*ind.link)->erase(ind.elem);
        --len;
        if ((*ind.link)->empty()) {
            size_t k = hasher(key) % table.size();
            table[k] = end_table;
            delete *ind.link;
            links.erase(ind.link);
        }
    }
    iterator find(const KeyType& key) {
        if (table.size() == 0)
            return end();
        size_t k = hasher(key) % table.size();
        if (table[k] == end_table) {
            return end();
        }
        for (typename Cell::iterator ind = (*table[k])->begin(); ind != (*table[k])->end(); ++ind)
            if (ind->first == key)
                return {&links, table[k], ind};
        return end();
    }
    const_iterator find(const KeyType& key) const {
        if (table.size() == 0)
            return end();
        size_t k = hasher(key) % table.size();
        if (table[k] == end_table) {
            return end();
        }
        for (typename Cell::const_iterator ind = (*table[k])->begin(); ind != (*table[k])->end(); ++ind)
            if (ind->first == key)
                return {&links, table[k], ind};
        return end();
    }
    ValueType& operator[](const KeyType& key) {
        auto ind = find(key);
        if (ind == end()) {
            insert({key, ValueType()});
            ind = find(key);
        }
        return ind->second;
    }
    const ValueType& at(const KeyType& key) const {
        auto ind = find(key);
        if (ind == end())
            throw std::out_of_range("method at don't found key");
        return ind->second;
    }
    void clear() {
        table.clear();
        for (const auto& el : links) {
            delete el;
        }
        links.clear();
        len = 0;
    }

private:
    Cell* list_iter = new Cell();
    const typename Cell::iterator end_iter = list_iter->begin();
    const typename Cell::const_iterator end_const_iter = list_iter->begin();
    std::list<Cell*>* list_iter_end = new std::list<Cell*>();
    const typename std::list<Cell*>::iterator end_table = list_iter_end->begin();
    const double max_load = 1.0 / 2;

    Hash hasher;
    size_t len = 0;
    std::vector<typename std::list<Cell*>::iterator> table;
    std::list<Cell*> links;
    inline void _insert(const std::pair<KeyType, ValueType>& pr) {
        if (table.size() == 0) {
            table.resize(1, end_table);
        }
        size_t k = hasher(pr.first) % table.size();
        if (table[k] == end_table) {
            links.push_back(new Cell());
            auto it = links.end();
            table[k] = --it;
        }
        if (find(pr.first) == end()) {
            (*table[k])->push_back(pr);
            ++len;
        }
    }
    inline bool _check_len() {
        if (len < max_load * table.size())
            return false;
        size_t k = table.size();
        table.clear();
        table.resize(k << 1, end_table);
        std::list<Cell*> buff = std::move(links);
        links.clear();
        len = 0;
        for (typename std::list<Cell*>::iterator ind = buff.begin(); ind != buff.end(); ++ind) {
            for (const auto& elem : **ind) {
                _insert(elem);
            }
            delete *ind;
        }
        return true;
    }
};
