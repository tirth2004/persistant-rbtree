#ifndef PERSISTENT_TREAP_HPP
#define PERSISTENT_TREAP_HPP

#include <iostream>
#include <optional>
#include <utility>
#include <random>
#include <chrono>
#include "Values.hpp"
#include "Nodes.hpp"

using namespace std;
std :: mt19937 static rng(std :: chrono::steady_clock::now().time_since_epoch().count());
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt) {
    if (opt.has_value()) {
        os << *opt;  // or opt.value();
    } else {
        os << "Not present";
    }
    return os;
}

template<typename Key, typename Value>
Nodes<Key, Value> nodes;

template<typename Value>
Values<Value> values;

template<typename Key, typename Value>
struct Node{
    Key key;
    int vID;
    int y;
    std :: pair<int, int>p;
    Node() : y(rng()), p({0, 0}) {}
    Node(Key k, Value v) : key(k), y(rng()), p({0, 0}) { vID = values<Value>.add(v); }
    Node(int id) : key(nodes<Key, Value>[id].key), vID(nodes<Key, Value>[id].vID), y(nodes<Key, Value>[id].y), p(nodes<Key, Value>[id].p) {}
};

template<typename Key, typename Value>
int merge(int T1, int T2){
    if(!T2) return T1;
    if(!T1) return T2;
    if(nodes<Key, Value>[T1].y > nodes<Key, Value>[T2].y)
    {
        int id = nodes<Key, Value>.add(T1);
        nodes<Key, Value>[id].p.second = merge<Key, Value>(nodes<Key, Value>[id].p.second, T2);
        return id;
    }
    else
    {
        int id = nodes<Key, Value>.add(T2);
        nodes<Key, Value>[id].p.first = merge<Key, Value>(T1, nodes<Key, Value>[id].p.first);
        return id;
    }
}

template<typename Key, typename Value>
pair<int, int> split(int T, Key k){
    if(!T) return{0, 0};
    int id = nodes<Key, Value>.add(T);
    if(nodes<Key, Value>[T].key > k){
        auto res = split<Key, Value>(nodes<Key, Value>[id].p.first, k);
        nodes<Key, Value>[id].p.first = res.second;
        return {res.first, id};
    }
    else
    {
        auto res = split<Key, Value>(nodes<Key, Value>[id].p.second, k);
        nodes<Key, Value>[id].p.second = res.first;
        return {id, res.second};
    }
}

template<typename Key, typename Value>
struct Treap{
    int root;
    Treap() : root(0) {}

    optional<Value> find(int T, const Key &key){
        if(!T)  return nullopt;
        if(nodes<Key, Value>[T].key == key)
            return values<Value>[nodes<Key, Value>[T].vID];
        if(nodes<Key, Value>[T].key > key){
            return find(nodes<Key, Value>[T].p.first, key);
        }
        return find(nodes<Key, Value>[T].p.second, key);
    }

    optional<Key> find_lessThan(int T, const Key &key){
        if(!T)  return nullopt;
        if(nodes<Key, Value>[T].key < key){
            optional<Value>val = find_lessThan(nodes<Key, Value>[T].p.second, key);
            if(val.has_value()){
                return val;
            }
            return nodes<Key, Value>[T].key;
        }
        else
        {
            return find_lessThan(nodes<Key, Value>[T].p.first, key);
        }
    }

    int insert(int T, Key key, Value value){
        optional<Value> v;
        if((v = find(T, key)).has_value()){
            if(v == value)
                return T;
            cerr << "Value already present !\n";
            return T;
        }
        auto Split = split<Key, Value>(T, key);        int id = nodes<Key, Value>.add(key, value);
        return merge<Key, Value>(Split.first, merge<Key, Value>(id, Split.second)); 
    }

    int remove(int T, Key key){
        if(!find(T, key).has_value())   return T;
        optional<Key> lt = find_lessThan(T, key);
        auto sp1 = split<Key, Value>(T, key);
        if(lt.has_value()){
            auto sp2 = split<Key, Value>(sp1.first, *lt);
            return merge<Key, Value>(sp2.first, sp1.second);
        }
        else
        {
            return sp1.second;
        }
    }

    void insert(Key key, Value value){
        root = insert(root, key, value);
    }

    optional<Value> find(Key key){
        return find(root, key);
    }

    void remove(Key key){
        root = remove(root, key);
    }

    void edit(Key key, Value value){
        root = insert(remove(root, key), key, value);
    }

};
template<typename Key, typename Value>
vector<Treap<Key, Value>>versions;

template<typename Key, typename Value>
void snapshot(Treap<Key, Value> T){
    versions<Key, Value>.push_back(T);
}

template<typename Key, typename Value>
Treap<Key, Value> rollback(int i){
    if(i >= versions<Key, Value>.size())
    {
        cerr << "There are not so many versions\n";
        return Treap<Key, Value>();
    }
    return versions<Key, Value>[i];
}

#endif // PERSISTENT_TREAP_HPP
