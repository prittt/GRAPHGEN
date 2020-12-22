// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_DRAG_H_
#define GRAPHGEN_DRAG_H_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <istream>
#include <vector>


/** @brief A BinaryDrag is the GRAPHGEN implementation of a Binary Directed Rooted Acyclic Graph (DRAG in short)

The implementation A BinaryDrag is template on the node data type. A BinaryDrag can be used to model/implement
a simple binary tree, a multi-rooted binary tree, a directed acyclic graph with just one root, or a multi-rooted
drag. A BinareDrag is the core data structure of GRAPHGEN. 

*/
template<typename T>
class BinaryDrag {
public:
    /** @brief Defines a node of the BinaryDrag */
    struct node
    {
        T data;
        node *left = nullptr, *right = nullptr;
        //std::vector<node *> parents_;

        node() {}
        node(T d) : data(std::move(d)) {}
        node(T d, node* l, node* r) : data(std::move(d)), left(l), right(r) {}
        //node(T d, node* l, node* r, std::vector<node *> parents) : data(std::move(d)), left(l), right(r), parents_(parents) {}

        bool isleaf() const {
            return left == nullptr && right == nullptr;
        }
    };

    /** @brief Vector of unique pointers to BinaryDrag nodes. This is useful to automatically free 
    the memory of the BinaryDrag, without requiring a recursive exploration of its structure*/
    std::vector<std::unique_ptr<node>> nodes_;

    /** @brief Creates and returns a new node updating the nodes_ vector */
    template<typename... Args>
    node* make_node(Args&&... args) {
        nodes_.emplace_back(std::make_unique<node>(std::forward<Args>(args)...));
        return nodes_.back().get();
    }

    std::vector<node *> roots_;

    /** @brief Adds a new root to the vector of roots */
    void AddRoot(node* r) {
        roots_.push_back(r);
    }

    /** @brief Returns the last root of the BinaryDrag */
    node* GetRoot() const {
        return roots_.front();
    }

    /** Swap member bd1 <-> bd2 */
    friend void swap(BinaryDrag& bd1, BinaryDrag& bd2) {
        using std::swap;
        swap(bd1.roots_, bd2.roots_);
        swap(bd1.nodes_, bd2.nodes_);
    }

    /** @brief Recursive function to copy a BinaryDrag.
    
    This member function is also required by the copy constructor.

    @param[in] n The node address of the BinaryDrag from which start the recursive copy
    @param[in,out] copies Maps every node address to be copied with its copy address.

    @return Root address of the new tree.
    */
    node *MakeCopyRecursive(node* n, std::unordered_map<node*, node*>& copies) {
        if (n == nullptr)
            return nullptr;

        if (!copies[n]) {
            node *nn = make_node();
            nn->data = n->data;
            nn->left = MakeCopyRecursive(n->left, copies);
            nn->right = MakeCopyRecursive(n->right, copies);
            return copies[n] = nn;
        }
        return copies[n];
    }

    /** @brief Empty constructor */
    BinaryDrag() {}

    /** @brief Copy constructor */
    BinaryDrag(const BinaryDrag& bd) {
        std::unordered_map<node*, node*> copies;
        for (const auto& x : bd.roots_) {
            roots_.push_back(MakeCopyRecursive(x, copies));
        }
        /*for (const auto& n : bd.nodes_) {
            auto& nn = copies[n.get()];
            for (const auto& p : n->parents_) {
                nn->parents_.push_back(copies[p]);
            }
        }*/
    }

    /** @brief Special copy constructor that allows to track where the nodes in a tree have been copied to */
    BinaryDrag(const BinaryDrag& bd, std::vector<node*>& tracked_nodes) { 
        std::unordered_map<node*, node*> copies;
        for (const auto& x : bd.roots_) {
            roots_.push_back(MakeCopyRecursive(x, copies));
        }
        /*for (const auto& n : bd.nodes_) {
            auto& nn = copies[n.get()];
            for (const auto& p : n->parents_) {
                nn->parents_.push_back(copies[p]);
            }
        }*/
        for (auto& n : tracked_nodes) {
            n = copies[n];
        }
    }

    BinaryDrag(BinaryDrag&& t) {
        swap(*this, t);
    }
    
    /** @brief Copy assignment */
    BinaryDrag& operator=(BinaryDrag t) {
        swap(*this, t);
        return *this;
    }

    /** @brief Creates a new root updating the roots_ vector */
    node* make_root() {
        roots_.push_back(make_node());
        return roots_.back();
    }
};

//class ConactBinaryDrag : BinaryDrag<conact> {
//public:
//    ConactBinaryDrag(std::istream& is) {
//        Load(is, *this);
//    }
//
//    bool Serialize(const std::string& filename) {
//        std::ofstream os(filename);
//
//        if (!os) {
//            return false;
//        }
//
//        return Serialize(os);
//    }
//
//    bool Serialize(std::ostream& os = std::cout) {
//        if (!os) {
//            return false;
//        }
//
//        Save(os, *this);
//        return true;
//    }
//
//private:
//    struct Save {
//        std::ostream& os_;
//        std::unordered_set<ConactBinaryDrag::node*> visited_;
//        std::unordered_map<ConactBinaryDrag::node*, int> nodes_with_refs_;
//        int id_ = 0;
//
//        void CheckNodesTraversalRec(BinaryDrag<conact>::node *n)
//        {
//            if (nodes_with_refs_.find(n) != end(nodes_with_refs_)) {
//                if (!nodes_with_refs_[n])
//                    nodes_with_refs_[n] = ++id_;
//            }
//            else {
//                nodes_with_refs_[n] = 0;
//                if (!n->isleaf()) {
//                    CheckNodesTraversalRec(n->left);
//                    CheckNodesTraversalRec(n->right);
//                }
//            }
//        }
//
//        Save(std::ostream& os, ConactBinaryDrag& bd) : os_(os)
//        {
//            for (auto& x : bd.roots_) {
//                CheckNodesTraversalRec(x);
//            }
//            for (auto& x : bd.roots_) {
//                SaveRec(x);
//            }
//        }
//
//        void SaveRec(ConactBinaryDrag::node* n, int tab = 0)
//        {
//            os_ << std::string(tab, '\t');
//
//            if (!visited_.insert(n).second) {
//                os_ << "@ " << nodes_with_refs_[n] << "\n";
//                return;
//            }
//
//            if (nodes_with_refs_[n]) {
//                os_ << "^ " << nodes_with_refs_[n] << " ";
//            }
//
//            if (n->isleaf()) {
//                assert(n->data.t == conact::type::ACTION);
//                auto a = n->data.actions();
//                os_ << ". " << a[0];
//                for (size_t i = 1; i < a.size(); ++i) {
//                    os_ << "," << a[i];
//                }
//                os_ << " - " << n->data.next << "\n";
//            }
//            else {
//                assert(n->data.t == conact::type::CONDITION);
//                os_ << n->data.condition << "\n";
//                SaveRec(n->left, tab + 1);
//                SaveRec(n->right, tab + 1);
//            }
//        }
//    };
//
//    struct Load {
//        std::istream& is_;
//        ConactBinaryDrag& bd_;
//        std::vector<ConactBinaryDrag::node*> np_;
//
//        Load(std::istream& is, ConactBinaryDrag& bd) : is_(is), bd_(bd)
//        {
//            np_.push_back(nullptr);
//            while (true) {
//                ConactBinaryDrag::node* n = LoadConactTreeRec();
//                if (n == nullptr)
//                    break;
//                bd_.AddRoot(n);
//            }
//        }
//
//        ConactBinaryDrag::node* LoadConactTreeRec()
//        {
//            std::string s;
//            while (is_ >> s) {
//                if (s[0] == '#') {
//                    getline(is_, s);
//                }
//                else {
//                    break;
//                }
//            }
//
//            if (!is_) {
//                return nullptr;
//            }
//
//            if (s == "@") {
//                int pos;
//                is_ >> pos;
//                return np_[pos];
//            }
//
//            auto n = bd_.make_node();
//
//            if (s == "^") {
//                int pos;
//                is_ >> pos >> s;
//                if (pos != np_.size()) {
//                    throw;
//                }
//                np_.push_back(n);
//            }
//
//            if (s == ".") {
//                // leaf
//                n->data.t = conact::type::ACTION;
//                do {
//                    int action;
//                    is_ >> action >> std::ws;
//                    n->data.action.set(action - 1);
//                } while (is_.peek() == ',' && is_.get());
//
//                if (is_.get() != '-') {
//                    throw;
//                }
//                is_ >> n->data.next;
//            }
//            else {
//                // real node with branches
//                n->data.t = conact::type::CONDITION;
//                n->data.condition = s;
//
//                n->left = LoadConactTreeRec();
//                n->right = LoadConactTreeRec();
//            }
//
//            return n;
//        }
//    };
//};

#endif // !GRAPHGEN_DRAG_H_
