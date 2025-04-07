#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <string>
#include <memory>
#include <functional>

namespace kvdb {

// Node color enum
enum class Color { RED, BLACK };

// Node structure for Red-Black Tree
template<typename K, typename V>
struct Node {
    K key;
    V value;
    Color color;
    std::shared_ptr<Node<K, V>> left;
    std::shared_ptr<Node<K, V>> right;
    std::shared_ptr<Node<K, V>> parent;

    Node(const K& k, const V& v, Color c = Color::RED)
        : key(k), value(v), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
};

// Red-Black Tree class
template<typename K, typename V>
class RedBlackTree {
public:
    RedBlackTree();
    ~RedBlackTree() = default;

    // Basic operations
    bool insert(const K& key, const V& value);
    bool remove(const K& key);
    std::optional<V> get(const K& key) const;
    bool contains(const K& key) const;
    size_t size() const;
    bool empty() const;

    // Iterator support
    class Iterator {
    public:
        Iterator(std::shared_ptr<Node<K, V>> node = nullptr);
        std::pair<K, V> operator*() const;
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
    private:
        std::shared_ptr<Node<K, V>> current;
    };

    Iterator begin() const;
    Iterator end() const;

private:
    std::shared_ptr<Node<K, V>> root;
    size_t nodeCount;

    // Helper functions
    void leftRotate(std::shared_ptr<Node<K, V>> x);
    void rightRotate(std::shared_ptr<Node<K, V>> y);
    void insertFixup(std::shared_ptr<Node<K, V>> z);
    void removeFixup(std::shared_ptr<Node<K, V>> x);
    std::shared_ptr<Node<K, V>> findMin(std::shared_ptr<Node<K, V>> node) const;
    std::shared_ptr<Node<K, V>> findMax(std::shared_ptr<Node<K, V>> node) const;
    std::shared_ptr<Node<K, V>> find(const K& key) const;
    void transplant(std::shared_ptr<Node<K, V>> u, std::shared_ptr<Node<K, V>> v);
};

} // namespace kvdb

#include "rbtree_impl.hpp"

#endif // RBTREE_HPP 