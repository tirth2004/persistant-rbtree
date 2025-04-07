#ifndef RBTREE_IMPL_HPP
#define RBTREE_IMPL_HPP

#include "rbtree.hpp"
#include <optional>

namespace kvdb {

// Constructor
template<typename K, typename V>
RedBlackTree<K, V>::RedBlackTree() : root(nullptr), nodeCount(0) {}

// Insert operation
template<typename K, typename V>
bool RedBlackTree<K, V>::insert(const K& key, const V& value) {
    // If key already exists, update the value
    auto existing = find(key);
    if (existing) {
        existing->value = value;
        return true;
    }

    // Create new node
    auto z = std::make_shared<Node<K, V>>(key, value);
    
    // Find insertion position
    std::shared_ptr<Node<K, V>> y = nullptr;
    auto x = root;
    
    while (x != nullptr) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    
    // Set parent
    z->parent = y;
    
    // Attach to tree
    if (y == nullptr) {
        root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }
    
    // Fix Red-Black properties
    insertFixup(z);
    nodeCount++;
    return true;
}

// Get operation
template<typename K, typename V>
std::optional<V> RedBlackTree<K, V>::get(const K& key) const {
    auto node = find(key);
    if (node) {
        return node->value;
    }
    return std::nullopt;
}

// Contains operation
template<typename K, typename V>
bool RedBlackTree<K, V>::contains(const K& key) const {
    return find(key) != nullptr;
}

// Size operation
template<typename K, typename V>
size_t RedBlackTree<K, V>::size() const {
    return nodeCount;
}

// Empty operation
template<typename K, typename V>
bool RedBlackTree<K, V>::empty() const {
    return nodeCount == 0;
}

// Find helper function
template<typename K, typename V>
std::shared_ptr<Node<K, V>> RedBlackTree<K, V>::find(const K& key) const {
    auto current = root;
    while (current != nullptr) {
        if (key == current->key) {
            return current;
        } else if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return nullptr;
}

// Left rotation
template<typename K, typename V>
void RedBlackTree<K, V>::leftRotate(std::shared_ptr<Node<K, V>> x) {
    auto y = x->right;
    x->right = y->left;
    
    if (y->left != nullptr) {
        y->left->parent = x;
    }
    
    y->parent = x->parent;
    
    if (x->parent == nullptr) {
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    
    y->left = x;
    x->parent = y;
}

// Right rotation
template<typename K, typename V>
void RedBlackTree<K, V>::rightRotate(std::shared_ptr<Node<K, V>> y) {
    auto x = y->left;
    y->left = x->right;
    
    if (x->right != nullptr) {
        x->right->parent = y;
    }
    
    x->parent = y->parent;
    
    if (y->parent == nullptr) {
        root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    
    x->right = y;
    y->parent = x;
}

// Insert fixup
template<typename K, typename V>
void RedBlackTree<K, V>::insertFixup(std::shared_ptr<Node<K, V>> z) {
    while (z->parent != nullptr && z->parent->color == Color::RED) {
        if (z->parent == z->parent->parent->left) {
            auto y = z->parent->parent->right;
            if (y != nullptr && y->color == Color::RED) {
                z->parent->color = Color::BLACK;
                y->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(z);
                }
                z->parent->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                rightRotate(z->parent->parent);
            }
        } else {
            auto y = z->parent->parent->left;
            if (y != nullptr && y->color == Color::RED) {
                z->parent->color = Color::BLACK;
                y->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(z);
                }
                z->parent->color = Color::BLACK;
                z->parent->parent->color = Color::RED;
                leftRotate(z->parent->parent);
            }
        }
    }
    root->color = Color::BLACK;
}

// Remove operation
template<typename K, typename V>
bool RedBlackTree<K, V>::remove(const K& key) {
    auto z = find(key);
    if (!z) {
        return false;
    }
    
    auto y = z;
    auto yOriginalColor = y->color;
    std::shared_ptr<Node<K, V>> x;
    
    if (z->left == nullptr) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nullptr) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = findMin(z->right);
        yOriginalColor = y->color;
        x = y->right;
        
        if (y->parent == z) {
            if (x) {
                x->parent = y;
            }
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    
    if (yOriginalColor == Color::BLACK) {
        removeFixup(x);
    }
    
    nodeCount--;
    return true;
}

// Transplant helper function
template<typename K, typename V>
void RedBlackTree<K, V>::transplant(std::shared_ptr<Node<K, V>> u, std::shared_ptr<Node<K, V>> v) {
    if (u->parent == nullptr) {
        root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    
    if (v) {
        v->parent = u->parent;
    }
}

// Find minimum helper function
template<typename K, typename V>
std::shared_ptr<Node<K, V>> RedBlackTree<K, V>::findMin(std::shared_ptr<Node<K, V>> node) const {
    if (!node) {
        return nullptr;
    }
    
    while (node->left) {
        node = node->left;
    }
    
    return node;
}

// Find maximum helper function
template<typename K, typename V>
std::shared_ptr<Node<K, V>> RedBlackTree<K, V>::findMax(std::shared_ptr<Node<K, V>> node) const {
    if (!node) {
        return nullptr;
    }
    
    while (node->right) {
        node = node->right;
    }
    
    return node;
}

// Remove fixup
template<typename K, typename V>
void RedBlackTree<K, V>::removeFixup(std::shared_ptr<Node<K, V>> x) {
    while (x != root && (!x || x->color == Color::BLACK)) {
        if (x == x->parent->left) {
            auto w = x->parent->right;
            if (w && w->color == Color::RED) {
                w->color = Color::BLACK;
                x->parent->color = Color::RED;
                leftRotate(x->parent);
                w = x->parent->right;
            }
            
            if ((!w->left || w->left->color == Color::BLACK) &&
                (!w->right || w->right->color == Color::BLACK)) {
                w->color = Color::RED;
                x = x->parent;
            } else {
                if (!w->right || w->right->color == Color::BLACK) {
                    if (w->left) {
                        w->left->color = Color::BLACK;
                    }
                    w->color = Color::RED;
                    rightRotate(w);
                    w = x->parent->right;
                }
                
                w->color = x->parent->color;
                x->parent->color = Color::BLACK;
                if (w->right) {
                    w->right->color = Color::BLACK;
                }
                leftRotate(x->parent);
                x = root;
            }
        } else {
            auto w = x->parent->left;
            if (w && w->color == Color::RED) {
                w->color = Color::BLACK;
                x->parent->color = Color::RED;
                rightRotate(x->parent);
                w = x->parent->left;
            }
            
            if ((!w->right || w->right->color == Color::BLACK) &&
                (!w->left || w->left->color == Color::BLACK)) {
                w->color = Color::RED;
                x = x->parent;
            } else {
                if (!w->left || w->left->color == Color::BLACK) {
                    if (w->right) {
                        w->right->color = Color::BLACK;
                    }
                    w->color = Color::RED;
                    leftRotate(w);
                    w = x->parent->left;
                }
                
                w->color = x->parent->color;
                x->parent->color = Color::BLACK;
                if (w->left) {
                    w->left->color = Color::BLACK;
                }
                rightRotate(x->parent);
                x = root;
            }
        }
    }
    
    if (x) {
        x->color = Color::BLACK;
    }
}

// Iterator implementation
template<typename K, typename V>
RedBlackTree<K, V>::Iterator::Iterator(std::shared_ptr<Node<K, V>> node) : current(node) {}

template<typename K, typename V>
std::pair<K, V> RedBlackTree<K, V>::Iterator::operator*() const {
    return {current->key, current->value};
}

template<typename K, typename V>
typename RedBlackTree<K, V>::Iterator& RedBlackTree<K, V>::Iterator::operator++() {
    if (current->right != nullptr) {
        current = current->right;
        while (current->left != nullptr) {
            current = current->left;
        }
    } else {
        auto parent = current->parent;
        while (parent != nullptr && current == parent->right) {
            current = parent;
            parent = parent->parent;
        }
        current = parent;
    }
    return *this;
}

template<typename K, typename V>
bool RedBlackTree<K, V>::Iterator::operator!=(const Iterator& other) const {
    return current != other.current;
}

template<typename K, typename V>
typename RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::begin() const {
    if (root == nullptr) {
        return Iterator(nullptr);
    }
    auto node = root;
    while (node->left != nullptr) {
        node = node->left;
    }
    return Iterator(node);
}

template<typename K, typename V>
typename RedBlackTree<K, V>::Iterator RedBlackTree<K, V>::end() const {
    return Iterator(nullptr);
}

} // namespace kvdb

#endif // RBTREE_IMPL_HPP 