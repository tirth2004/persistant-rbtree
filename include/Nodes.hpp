#ifndef _NODES_HPP_
#define _NODES_HPP_
#include<vector>

template<typename Key, typename Value>
struct Node;

template<typename Key, typename Value>
class Nodes {
private:
    std :: vector<Node<Key, Value>> nodes;
    int sz;
public:
    Nodes() : sz(0) {nodes.push_back(Node<Key, Value>());}
    Nodes(const std :: vector<Node<Key, Value>>& initialNodes) : nodes(initialNodes), sz(initialNodes.size()) {}


    int add(const Node<Key, Value>& node) {
        nodes.push_back(node);
        return ++sz;
    }

    int add(const Key key, const Value value){
        return add(Node<Key, Value>(key, value));
    }

    int add(int id){
        nodes.push_back(nodes[id]);
        return ++sz;
    }

    int size() const {
        return sz;
    }

    Node<Key, Value>& operator[](int index) {
        return nodes[index];
    }
};
#endif