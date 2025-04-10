#ifndef PERSISTENT_TREAP_HPP
#define PERSISTENT_TREAP_HPP

// A persistent treap can perform insert and look-up very quickly with expected time of O(log(number of nodes))
// The basic idea is whenever a new change or delete is performed on the structure only O(height of the tree) nodes change
// Since the expected height of tree is log(number of nodes) a small number of nodes change.

//                              IDEAS TO TACKLE CHALLENGES

// for snapshot storing the DB to disk is very inefficient and loading it from disk while performing operations for previous versions is also very slow
        // solution : use persistent treap which has very fast acess in memory (O(log(size of treap)))

// each node change requires value copy overhead which can be large
        // solution : store values in a serparate vector which can be pointed to by the node. Hence we only need to store index of values.

// storing on disk is not easy with pointers (loading will not be feasible)
        // solution : store everything in vectors and then access by index (the index basically acts like a pointer)

// we are considering keys as string since all operations like get and set require key comparision, this process will be slow for large strings
        // solution : has strings to integers, only compare strings in case of collisions

#include <iostream>
#include <optional>
#include <utility>
#include <random>
#include <chrono>
#include <fstream>
#include "Values.hpp"   // values class that stores all values of pointed by keys (nodes)
#include "Nodes.hpp"    // nodes class that a vector to store all nodes of treap
#include "hash.hpp"     // Fowler-Noll-Vo hash function

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

static FNV1aHasher hasher;

template<typename Key, typename Value>
Nodes<Key, Value> nodes;

template<typename Value>
Values<Value> values;

// node structure to store keys and pointers to values
template<typename Key, typename Value>
struct Node{
    Key key;
    uint64_t hkey;
    int vID;
    int y;
    std :: pair<int, int>p;
    Node() : y(rng()), p({0, 0}) {} 
    Node(Key k, Value v) : key(k), hkey(hasher(key)), y(rng()), p({0, 0}) { vID = values<Value>.add(v); }
    Node(int id) : key(nodes<Key, Value>[id].key), hkey(nodes<Key, Value>[id].key), vID(nodes<Key, Value>[id].vID), y(nodes<Key, Value>[id].y), p(nodes<Key, Value>[id].p) {}

    friend ostream& operator<<(std::ostream& os, const Node<Key, Value> &node) {
        os << node.key << " " << node.hkey << " " <<  node.vID << " " << node.y << " " << node.p.first << " " << node.p.second;
        return os;
    }

    friend istream& operator>>(std::istream& is, Node<Key, Value> &node) {
        is >> node.key >> node.hkey >> node.vID >> node.y >> node.p.first >> node.p.second;
        return is;
    }
};

// standard treap merge function
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

// treap split function
template<typename Key, typename Value>
pair<int, int> split(int T, const Key &k, const uint64_t &hk){
    if(!T) return{0, 0};
    int id = nodes<Key, Value>.add(T);
    if(nodes<Key, Value>[T].hkey > hk || (nodes<Key, Value>[T].hkey == hk && nodes<Key, Value>[T].key > k)){
        auto res = split<Key, Value>(nodes<Key, Value>[id].p.first, k, hk);
        nodes<Key, Value>[id].p.first = res.second;
        return {res.first, id};
    }
    else
    {
        auto res = split<Key, Value>(nodes<Key, Value>[id].p.second, k, hk);
        nodes<Key, Value>[id].p.second = res.first;
        return {id, res.second};
    }
}

template<typename Key, typename Value>
struct Treap;

// versions to store snapshots
template<typename Key, typename Value>
vector<Treap<Key, Value>>versions;

// this treap class is like a wrapper around our node which gives get, set, find functionality.
template<typename Key, typename Value>
struct Treap{
    int root;
    Treap() : root(0) {}
    Treap(int ROOT) : root(ROOT) {}

    optional<Value> find(int T, const Key &key, const uint64_t &hkey){
        if(!T)  return nullopt;
        if(nodes<Key, Value>[T].hkey == hkey && nodes<Key, Value>[T].key == key)
            return values<Value>[nodes<Key, Value>[T].vID];
        if(nodes<Key, Value>[T].hkey > hkey || (nodes<Key, Value>[T].hkey == hkey && nodes<Key, Value>[T].key > key)){
            return find(nodes<Key, Value>[T].p.first, key, hkey);
        }
        return find(nodes<Key, Value>[T].p.second, key, hkey);
    }

    // used when deleting to get the previous key
    optional<Key> find_lessThan(int T, const Key &key, const uint64_t &hkey){
        if(!T)  return nullopt;
        if(nodes<Key, Value>[T].hkey < hkey || (nodes<Key, Value>[T].hkey == hkey && nodes<Key, Value>[T].key < key)){
            optional<Value>val = find_lessThan(nodes<Key, Value>[T].p.second, key, hkey);
            if(val.has_value()){
                return val;
            }
            return nodes<Key, Value>[T].key;
        }
        else
        {
            return find_lessThan(nodes<Key, Value>[T].p.first, key, hkey);
        }
    }

    int insert(int T, const Key &key, const Value &value){
        optional<Value> v;
        uint64_t hkey = hasher(key);
        if((v = find(T, key, hkey)).has_value()){
            if(v == value)
                return T;
            cerr << "Value already present !\n";
            return T;
        }
        auto Split = split<Key, Value>(T, key, hkey);        
        int id = nodes<Key, Value>.add(key, value);
        return merge<Key, Value>(Split.first, merge<Key, Value>(id, Split.second)); 
    }

    int remove(int T, const Key &key, const uint64_t &hkey){
        if(!find(T, key, hkey).has_value())   return T;
        optional<Key> lt = find_lessThan(T, key, hkey);
        auto sp1 = split<Key, Value>(T, key, hkey);
        if(lt.has_value()){
            auto sp2 = split<Key, Value>(sp1.first, *lt, hasher(*lt));
            return merge<Key, Value>(sp2.first, sp1.second);
        }
        else
        {
            return sp1.second;
        }
    }

    void insert(const Key &key, const Value &value){
        root = insert(root, key, value);
    }

    optional<Value> find(const Key &key){
        uint64_t hkey = hasher(key);
        return find(root, key, hkey);
    }

    void remove(const Key &key){
        uint64_t hkey = hasher(key);
        root = remove(root, key, hkey);
    }

    void edit(const Key &key, const Value &value){
        uint64_t hkey = hasher(key);
        root = insert(remove(root, key, hkey), key, value);
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
        return num++;
    }

    void save(ostream &os){
        os << size(root) << "\n";
        int sz = 1;
        int ROOT = save(os, root, sz);
        os << ROOT << '\n';
    }

    void load(istream &is){
        nodes<Key, Value>.clear();
        values<Value>.clear();
        versions<Key, Value>.clear();
        nodes<Key, Value>.add(Node<Key, Value>());
        int n {};
        is >> n;
        while(n--){
            Node<Key, Value>node;
            is >> node.vID;
            node.vID -= 1;
            is >> node.key >> node.y >> node.p.first >> node.p.second;
            node.hkey = hasher(node.key);
            Value v;
            is >> v;
            nodes<Key, Value>.add(node);
            values<Value>.add(v);
        }
        int ROOT {};
        is >> ROOT;
        this->root = ROOT;
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
void save(std::ostream& os, int root) {
    os << root << '\n';
    os << nodes<Key, Value>.size() - 1 << '\n';
    for(int i=1;i<nodes<Key, Value>.size();++i) {
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
int load(std::istream& is) {
    nodes<Key, Value>.clear();
    values<Value>.clear();
    versions<Key, Value>.clear();

    nodes<Key, Value>.add(Node<Key, Value>());
    int root {};
    is >> root;
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
        int ROOT;
        is >> ROOT;
        versions<Key, Value>.push_back(Treap<Key, Value>(ROOT));
    }
    return root;
}

#endif
