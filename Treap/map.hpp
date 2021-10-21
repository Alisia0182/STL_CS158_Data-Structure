#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

    typedef unsigned long long ull;

    class Rand {
        ull _rand;
    public:
        Rand() : _rand(2333ull) {}

        ull getRand() {
            _rand *= 101;
            _rand += 12071;
            return _rand;
        }
    };

    Rand randGen;

    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >
    class map {
    public:
        typedef pair<const Key, T> value_type;
    private:
        struct TreapNode {
            value_type *data;
            ull rand;
            TreapNode *LeftChild;
            TreapNode *RightChild;
            TreapNode *PrevNode;
            TreapNode *NextNode;

            TreapNode() :
                    data(nullptr),
                    LeftChild(this),
                    RightChild(this),
                    PrevNode(this),
                    NextNode(this) {}

            TreapNode(const value_type &_data) {
                data = new value_type(_data);
                rand = randGen.getRand();
                LeftChild = RightChild = nullptr;
                PrevNode = NextNode = nullptr;
            }

            TreapNode(const TreapNode &oth) : rand(oth.rand),
                                              LeftChild(nullptr), RightChild(nullptr),
                                              PrevNode(nullptr), NextNode(nullptr) {
                data = new value_type(*oth.data);
            }
            ~TreapNode(){
                delete data;
            }

        };

        TreapNode *root;
        TreapNode *NIL;
        size_t TreapSize;
        Compare Cmp;


        void midOrder(TreapNode *rt, TreapNode *&tmp) {
            if (rt->LeftChild != NIL) midOrder(rt->LeftChild, tmp);
            rt->PrevNode = tmp;
            if (tmp != NIL) tmp->NextNode = rt;
            tmp = rt;
            if (rt->RightChild != NIL) midOrder(rt->RightChild, tmp);
        }

        //NIL
        //左儿子 max 右儿子 min
        //Prev max Next min

        void copy(TreapNode *&des, TreapNode *oth, TreapNode *NIL, TreapNode *othNIL) {
            if (oth == othNIL) {
                des = NIL;
                return;
            }
            des = new TreapNode(*oth);
            copy(des->LeftChild, oth->LeftChild, NIL, othNIL);
            copy(des->RightChild, oth->RightChild, NIL, othNIL);
        }

        TreapNode *getMin(TreapNode *rt) {
            if (rt == NIL) return NIL;
            TreapNode *rst = rt;
            while (rst->LeftChild != NIL) {
                rst = rst->LeftChild;
            }
            return rst;
        }

        TreapNode *getMax(TreapNode *rt) {
            if (rt == NIL) return NIL;
            TreapNode *rst = rt;
            while (rst->RightChild != NIL) {
                rst = rst->RightChild;
            }
            return rst;
        }

        TreapNode *find(TreapNode *rt, const Key &key) const {
            if (rt == NIL) return NIL;
            if (Cmp(key, rt->data->first))
                return find(rt->LeftChild, key);
            if (Cmp(rt->data->first, key))
                return find(rt->RightChild, key);
            return rt;
        }

        void LR(TreapNode *&rt) {
            TreapNode *OrgRight = rt->RightChild;
            rt->RightChild = OrgRight->LeftChild;
            OrgRight->LeftChild = rt;
            rt = OrgRight;
        }

        void RR(TreapNode *&rt) {
            TreapNode *OrgLeft = rt->LeftChild;
            rt->LeftChild = OrgLeft->RightChild;
            OrgLeft->RightChild = rt;
            rt = OrgLeft;
        }

        TreapNode *insert(TreapNode *&rt, const value_type &value) {
            if(root == NIL){
                root = new TreapNode(value);
                NIL->LeftChild = NIL->RightChild = NIL->PrevNode = NIL->NextNode = root;
                root->LeftChild = root->RightChild = root->PrevNode = root->NextNode = NIL;
                return root;
            }
            if (rt == NIL) {
                rt = new TreapNode(value);
/*
            if(rt == NIL->PrevNode->RightChild) //update max
                NIL->PrevNode = NIL->LeftChild = rt;
            if(rt == NIL->NextNode->LeftChild )//update min
                NIL->NextNode = NIL->RightChild = rt;
*/
                rt->LeftChild = rt->RightChild = NIL;
                return rt;
            } else if (Cmp(value.first, rt->data->first)) {//去左边插
                bool NextTo = (rt->LeftChild == NIL);
                TreapNode *NewNode = insert(rt->LeftChild, value);
                if (NextTo) {
                    NewNode->PrevNode = rt->PrevNode;
                    NewNode->NextNode = rt;

                    rt->PrevNode->NextNode = NewNode;//NIL->NextNode 也可以这么处理
                    rt->PrevNode = NewNode;
                }
                if (rt->LeftChild->rand > rt->rand)
                    RR(rt);
                return NewNode;
            } else if (Cmp(rt->data->first, value.first)) {//去右边插
                bool NextTo = (rt->RightChild == NIL);
                TreapNode *NewNode = insert(rt->RightChild, value);
                if (NextTo) {
                    NewNode->PrevNode = rt;
                    NewNode->NextNode = rt->NextNode;
                    rt->NextNode = NewNode;
                    NewNode->NextNode->PrevNode = NewNode;
                }
                if (rt->RightChild->rand > rt->rand)
                    LR(rt);
                return NewNode;
            }
            return NIL;
        }

        void remove(TreapNode *&rt, const Key &key) {
            if (rt == NIL) return;
            if (Cmp(key, rt->data->first))
                remove(rt->LeftChild, key);
            else if (Cmp(rt->data->first, key))
                remove(rt->RightChild, key);
            else {
                if(rt->LeftChild != NIL || rt->RightChild != NIL){
                    if(rt->RightChild == NIL || (rt->LeftChild != NIL && rt->LeftChild->rand > rt->RightChild->rand)){
                        RR(rt);
                        remove(rt->RightChild,key);
                    }else{
                        LR(rt);
                        remove(rt->LeftChild,key);
                    }
                }else{
                    rt->PrevNode->NextNode = rt->NextNode;
                    rt->NextNode->PrevNode = rt->PrevNode;
                    if(rt == NIL->PrevNode)
                        NIL->LeftChild = rt->PrevNode;
                    if(rt == NIL->NextNode)
                        NIL->RightChild = rt->NextNode;
                    delete rt;
                    rt = NIL;
                }
            }
        }

        void clear(TreapNode *rt) {
            if (rt == NIL) return;
            if (rt->LeftChild != NIL) clear(rt->LeftChild);
            if (rt->RightChild != NIL) clear(rt->RightChild);
            delete rt;
        }

    public:
        class const_iterator;

        class iterator {
            friend class map;

            friend class const_iterator;

        private:
            TreapNode *NodePtr;
            const map *MapPtr;

        public:
            iterator(TreapNode *_NodePtr = nullptr,
                     const map *_MapPtr = nullptr)
                    : NodePtr(_NodePtr),
                      MapPtr(_MapPtr) {}

            iterator(const iterator &other) :
                    NodePtr(other.NodePtr),
                    MapPtr(other.MapPtr) {}


            iterator operator++(int) {
                iterator rst(*this);
                if (NodePtr == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->NextNode;
                return rst;
            }

            iterator &operator++() {
                if (NodePtr == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->NextNode;
                return *this;
            }

            iterator operator--(int) {
                iterator rst(*this);
                if (NodePtr->PrevNode == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->PrevNode;
                return rst;
            }

            iterator &operator--() {
                if (NodePtr->PrevNode == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->PrevNode;
                return *this;
            }

            value_type &operator*() const {
                return *(NodePtr->data);
            }

            bool operator==(const iterator &rhs) const {
                return MapPtr == rhs.MapPtr && NodePtr == rhs.NodePtr;
            }

            bool operator==(const const_iterator &rhs) const {
                return MapPtr == rhs.MapPtr && NodePtr == rhs.NodePtr;
            }

            bool operator!=(const iterator &rhs) const {
                return MapPtr != rhs.MapPtr || NodePtr != rhs.NodePtr;
            }

            bool operator!=(const const_iterator &rhs) const {
                return MapPtr != rhs.MapPtr || NodePtr != rhs.NodePtr;
            }

            value_type *operator->() const noexcept {
                return NodePtr->data;
            }
        };

        class const_iterator {
            friend class map;

            friend class iterator;

        private:
            const TreapNode *NodePtr;
            const map *MapPtr;

        public:
            const_iterator
                    (const TreapNode *_NodePtr = nullptr,
                     const map *_MapPtr = nullptr)
                    : NodePtr(_NodePtr),
                      MapPtr(_MapPtr) {}

            const_iterator(const iterator &other) :
                    NodePtr(other.NodePtr),
                    MapPtr(other.MapPtr) {}


            const_iterator operator++(int) {
                const_iterator rst(*this);
                if (NodePtr == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->NextNode;
                return rst;
            }

            const_iterator &operator++() {
                if (NodePtr == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->NextNode;
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator rst(*this);
                if (NodePtr->PrevNode == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->PrevNode;
                return rst;
            }

            const_iterator &operator--() {
                if (NodePtr->PrevNode == MapPtr->NIL)
                    throw invalid_iterator();
                NodePtr = NodePtr->PrevNode;
                return *this;
            }

            value_type &operator*() const {
                return *(NodePtr->data);
            }

            bool operator==(const iterator &rhs) const {
                return MapPtr == rhs.MapPtr && NodePtr == rhs.NodePtr;
            }

            bool operator==(const const_iterator &rhs) const {
                return MapPtr == rhs.MapPtr && NodePtr == rhs.NodePtr;
            }

            bool operator!=(const iterator &rhs) const {
                return MapPtr != rhs.MapPtr || NodePtr != rhs.NodePtr;
            }

            bool operator!=(const const_iterator &rhs) const {
                return MapPtr != rhs.MapPtr || NodePtr != rhs.NodePtr;
            }

            value_type *operator->() const noexcept {
                return NodePtr->data;
            }
        };

        map() {
            NIL = new TreapNode();
            root = NIL;
            TreapSize = 0;
        }

        //a copy constructor
        map(const map &oth) {
            NIL = new TreapNode();
            TreapSize = oth.TreapSize;
            copy(root, oth.root, NIL, oth.NIL);

            NIL->LeftChild = NIL->PrevNode = getMax(root);
            NIL->RightChild = NIL->NextNode = getMin(root);

            //get next prev
            TreapNode *tmp = NIL;
            midOrder(root, tmp);
            tmp = getMax(root);
            tmp->NextNode = NIL;
        }

        map &operator=(const map &oth) {
            if(&oth == this) return *this;
            clear();
            delete NIL;
            NIL = new TreapNode();
            TreapSize = oth.TreapSize;
            copy(root, oth.root, NIL, oth.NIL);

            NIL->LeftChild = NIL->PrevNode = getMax(root);
            NIL->RightChild = NIL->NextNode = getMin(root);

            //get next prev
            TreapNode *tmp = NIL;
            midOrder(root, tmp);
            tmp = getMax(root);
            tmp->NextNode = NIL;
            return *this;
        }

        ~map() {
            clear(root);
            delete NIL;
        }

        T &at(const Key &key) {
            TreapNode *rst = find(root, key);
            if (rst == NIL) throw index_out_of_bound();
            return rst->data->second;
        }

        const T &at(const Key &key) const {
            TreapNode *rst = find(root, key);
            if (rst == NIL) throw index_out_of_bound();
            return rst->data->second;
        }

        T &operator[](const Key &key) {
            pair<iterator, bool> rst = insert(pair<const Key, T>(key, T()));
            return (*(rst.first)).second;
        }

        //throw index_out_of_bound if such key does not exist.
        const T &operator[](const Key &key) const {
            TreapNode *rst = find(root, key);
            if (rst == NIL) throw index_out_of_bound();
            return rst->data->second;
        }

        iterator begin() {
            return iterator(NIL->NextNode, this);
        }

        const_iterator cbegin() const {
            return const_iterator(NIL->NextNode, this);
        }

        iterator end() {
            return iterator(NIL, this);
        }

        const_iterator cend() const {
            return const_iterator(NIL, this);
        }

        bool empty() const {
            return TreapSize == 0;
        }

        size_t size() const {
            return TreapSize;
        }

        void clear() {
            clear(root);
            delete NIL;
            NIL = new TreapNode();
            root = NIL;
            TreapSize = 0;
        }

        pair<iterator, bool> insert(const value_type &value) {//维护next prev
            iterator it = find(value.first);
            if (it.NodePtr != NIL)//插入失败
                return pair<iterator, bool>(it, false);
            TreapNode *NewNode = insert(root, value);
            it = iterator(NewNode, this);
            ++TreapSize;
            return pair<iterator, bool>(it, true);
        }

        void erase(iterator pos) {
            if(pos.NodePtr == NIL || pos.MapPtr != this)
                throw invalid_iterator();
            --TreapSize;
            remove(root,pos.NodePtr->data->first);
        }


        size_t count(const Key &key) const {
            TreapNode *rst = find(root, key);
            if (rst == NIL) return 0;
            return 1;
        }

        iterator find(const Key &key){
            TreapNode *rst = find(root, key);
            if (rst == NIL) return end();//cannot find
            return iterator(rst, this);
        }

        const_iterator find(const Key &key) const {
            TreapNode *rst = find(root, key);
            if (rst == NIL) return cend();
            return const_iterator(rst, this);
        }
    };
}
#endif