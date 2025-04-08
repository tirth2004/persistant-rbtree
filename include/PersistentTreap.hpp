#ifndef PERSISTENT_TREAP_HPP
#define PERSISTENT_TREAP_HPP

#include <bits/stdc++.h>

using namespace std;
inline mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

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
struct Node{
    Key key;
    Value value;
    int y;
    pair<Node* , Node*>p;
    Node(Key k, Value v) : key(k), value(v), y(rng()), p({0, 0}) {};
    Node(Node* u) : key(u->key), value(u->value), y(u->y), p(u->p) {};  

    ~Node();
};
template<typename Key, typename Value>
struct Treap;

template<typename Key, typename Value>
Node<Key, Value>* merge(Node<Key, Value>* T1, Node<Key, Value>* T2){
    if(!T2) return T1;
    if(!T1) return T2;
    if(T1->y > T2->y)
    {
        Node<Key, Value>* newNode = new Node<Key, Value>(T1);
        newNode->p.second = merge(newNode->p.second, T2);
        return newNode;
    }
    else
    {
        Node<Key, Value>* newNode = new Node<Key, Value>(T2);
        newNode->p.first = merge(T1, newNode->p.first);
        return newNode;
    }
}
template<typename Key, typename Value>
pair<Node<Key, Value>*, Node<Key, Value>* > split(Node<Key, Value>* T, Key k){
    if(!T) return{0, 0};
    Node<Key, Value>* newNode = new Node<Key, Value>(T);
    if(T->key > k){
        auto res = split(newNode->p.first, k);
        newNode->p.first = res.second;
        return {res.first, newNode};
    }
    else
    {
        auto res = split(newNode->p.second, k);
        newNode->p.second = res.first;
        return {newNode, res.second};
    }
}

template<typename Key, typename Value>
struct Treap{
    Node<Key, Value>* root;
    Treap() : root(0) {}
    Treap(Treap<Key, Value>* T) : root(T->root) {}

    optional<Value> find(Node<Key, Value>* T, Key key){
        if(!T)  return nullopt;
        if(T->key == key)
            return T->value;
        if(T->key > key){
            return find(T->p.first, key);
        }
        return find(T->p.second, key);
    }

    optional<Key> find_lessThan(Node<Key, Value>* T, Key key){
        if(!T)  return nullopt;
        if(T->key < key){
            optional<Value>val = find_lessThan(T->p.second, key);
            if(val.has_value()){
                return val;
            }
            return T->key;
        }
        else
        {
            return find_lessThan(T->p.first, key);
        }
    }

    Node<Key, Value>* insert(Node<Key, Value>* T, Key key, Value value){
        optional<Value> v;
        if((v = find(T, key)).has_value()){
            if(v == value)
                return T;
            cerr << "Value already present !\n";
            return T;
        }
        auto Split = split(T, key);
        Node<Key, Value>* newNode = new Node<Key, Value>(key, value);
        return merge(Split.first, merge(newNode, Split.second)); 
    }

    Node<Key, Value>* remove(Node<Key, Value>* T, Key key){
        if(!find(T, key).has_value())   return T;
        optional<Key> lt = find_lessThan(T, key);
        auto sp1 = split(T, key);
        if(lt.has_value()){
            auto sp2 = split(sp1.first, *lt);
            return merge(sp2.first, sp1.second);
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
        remove(key);
        insert(key, value);
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
        return NULL;
    }
    return versions<Key, Value>[i];
}

#endif // PERSISTENT_TREAP_HPP

