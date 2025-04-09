#ifndef PERSISTENT_TREAP_HPP
#define PERSISTENT_TREAP_HPP

#include <iostream>
#include <optional>
#include <utility>
#include <random>
#include <chrono>
#include <fstream>
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

    friend ostream& operator<<(std::ostream& os, const Node<Key, Value> &node) {
        os << node.key << " " << node.vID << " " << node.y << " " << node.p.first << " " << node.p.second;
        return os;
    }

    friend istream& operator>>(std::istream& is, Node<Key, Value> &node) {
        is >> node.key >> node.vID >> node.y >> node.p.first >> node.p.second;
        return is;
    }
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
struct Treap;

template<typename Key, typename Value>
vector<Treap<Key, Value>>versions;

template<typename Key, typename Value>
struct Treap{
    int root;
    Treap() : root(0) {}
    Treap(int ROOT) : root(ROOT) {}

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

    int size(int T){
        if(!T)  return 0;
        return size(nodes<Key, Value>[T].p.first) + size(nodes<Key, Value>[T].p.second) + 1;
    }

    int save(ostream &os, int T, int &num){
        if(!T)  return 0;
        int left = save(os, nodes<Key, Value>[T].p.first, num);
        int right = save(os, nodes<Key, Value>[T].p.second, num);
        os << num << " " << nodes<Key, Value>[T].key << " " << nodes<Key, Value>[T].y << " " << left << " " << right << " " << values<Value>[nodes<Key, Value>[T].vID] << "\n";
        num++;
        return num;
    }

    void save(ostream &os){
        os << size(root) << "\n";
        int sz = 0;
        save(os, root, sz);
    }

    void load(istream &is){
        nodes<Key, Value>.clear();
        values<Value>.clear();
        versions<Key, Value>.clear();
        int n {};
        is >> n;
        while(n--){
            Node<Key, Value>node;
            is >> node.vID;
            is >> node.key >> node.y >> node.p.first >> node.p.second;
            Value v;
            is >> v;
            nodes<Key, Value>.add(node);
            values<Value>.add(v);
        }
    }

    // save function will do inorder traversal

};

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

template<typename Key, typename Value>
void save(std::ostream& os) {
    os << nodes<Key, Value>.size() << '\n';
    for(int i=0;i<nodes<Key, Value>.size();++i) {
        os << nodes<Key, Value>[i] << '\n';
    }

    os << values<Value>.size() << '\n';
    for (int i=0;i<values<Value>.size(); ++i) {
        os << values<Value>[i] << '\n';
    }

    os << versions<Key, Value>.size() << '\n';
    for(auto &T : versions<Key, Value>){
        os << T.root << "\n";
    }
}

template<typename Key, typename Value>
void load(std::istream& is) {
    nodes<Key, Value>.clear();
    values<Value>.clear();
    versions<Key, Value>.clear();

    int n {};
    is >> n;
    // is.ignore();

    while (n--) {
        Node<Key, Value> temp;
        is >> temp;
        nodes<Key, Value>.add(temp);
    }

    is >> n;
    // is.ignore();
    while (n--) {
        Value v;
        is >> v;
        values<Value>.add(v);
    }

    is >> n;
    while(n--){
        int root;
        is >> root;
        versions<Key, Value>.push_back(Treap<Key, Value>(root));
    }
}

#endif // PERSISTENT_TREAP_HPP
