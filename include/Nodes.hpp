#ifndef _NODES_HPP_
#define _NODES_HPP_
#include<vector>

template<typename Key, typename Value>
struct Node;

template<typename Key, typename Value>
class Nodes {
private:
    std :: vector<Node<Key, Value>> nodes;
public:
    Nodes() {nodes.push_back(Node<Key, Value>());}
    Nodes(const std :: vector<Node<Key, Value>>& initialNodes) : nodes(initialNodes) {}


    int add(const Node<Key, Value>& node) {
        nodes.push_back(node);
        return (int)nodes.size() - 1;
    }

    int add(const Key key, const Value value){
        return add(Node<Key, Value>(key, value));
    }

    int add(int id){
        nodes.push_back(nodes[id]);
        return (int)nodes.size() - 1;
    }

    int size() const {
        return nodes.size();
    }

    void clear(){
        nodes.clear();
    }

    Node<Key, Value>& operator[](int index) {
        return nodes[index];
    }
};
#endif