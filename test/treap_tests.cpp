#include <gtest/gtest.h>
#include "../include/PersistentTreap.hpp"

class TreapTest : public :: testing::Test {
protected: 
    Treap<int, int> treap;
    void SetUp() override {
        versions<int, int>.clear();
        treap.insert(69,690);
    }
};

TEST_F(TreapTest, Find) {
    EXPECT_EQ(treap.find(69), 690);
    EXPECT_EQ(treap.find(100), nullopt);
}

TEST_F(TreapTest, InsertAndFind) {
    
    treap.insert(1, 10);
    EXPECT_EQ(treap.find(1), 10);

    treap.insert(2,20);
    EXPECT_EQ(treap.find(2), 20);

    
}

TEST_F(TreapTest, Remove) {
    treap.remove(69);
    EXPECT_EQ(treap.find(69), nullopt);
}

TEST_F(TreapTest, Edit) {
    treap.edit(69, 6900);
    EXPECT_EQ(treap.find(69), 6900);
}

TEST_F(TreapTest, RollbackEditAndFind){
    snapshot(treap);
    treap.edit(69, 6900);
    EXPECT_EQ(treap.find(69), 6900);
    int oldValue = rollback<int, int>(0).find(69).value();
    EXPECT_EQ(oldValue, 690);
}

TEST_F(TreapTest, RollbackRemoveAndFind){
    snapshot(treap);
    treap.remove(69);
    EXPECT_EQ(treap.find(69), nullopt);
    int oldValue = rollback<int, int>(0).find(69).value();
    EXPECT_EQ(oldValue, 690);
}

TEST_F(TreapTest, RollbackInsertAndFind){
    snapshot(treap);
    treap.insert(420, 4200);
    EXPECT_EQ(treap.find(420), 4200);
    
    Treap<int, int> oldTreap = rollback<int, int>(0);
    EXPECT_EQ(oldTreap.find(420), nullopt);
}

TEST_F(TreapTest, RollbackTwiceAndEdit){
    snapshot(treap);
    Treap<int, int> version0 = rollback<int, int>(0);
    treap.edit(69, 6900);
    snapshot(treap);
    Treap<int, int> version1 = rollback<int, int>(1);
    treap.edit(69, 69000);

    EXPECT_EQ(version0.find(69), 690);
    EXPECT_EQ(version1.find(69), 6900);
    EXPECT_EQ(treap.find(69), 69000);
}





