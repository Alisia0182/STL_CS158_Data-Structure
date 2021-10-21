//512
//
//512
//
#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP
//#define DEBUG
#define printMergeSplitNodeSize
#include "exceptions.hpp"
#include <cstddef>
#ifdef printMergeSplitNodeSize
#include <iostream>
#endif

namespace sjtu {

    template<class T>
    class deque {
        typedef uint32_t type_size;
        static const type_size SqrtN = 5;
        static const type_size MergePoint = SqrtN;
        static const type_size SplitPoint = 2 * SqrtN;
        static const type_size NodeSize = 2 * SqrtN;
        static const type_size SplitLeft = SqrtN;
        // if a and b live next to each other,
        // and used_a + used_b >= MergePoint && < SplitPoint,
        // then we'll merge the two to a(the pre one)!!

        struct node;

        type_size deque_used_size;
        node *head;
        node *tail;
#ifdef DEBUG
        void printNodeSize(){
            node * p;
            int i = 0;
            for(p=head;p->data_ptr;++i,p=p->nxt)
            {
                std::cout << p->node_used_size << '\t';
                if(i % 10 == 0)
                    std::cout << "\n";
            }
        }
#endif
        struct node {
            node *prv;
            node *nxt;
            deque *deque_ptr;/// deque<T>????
            type_size deque_pos;//data_ptr[0]在deque（0标）中对应的下标

            type_size node_used_size;
            T **data_ptr;

            node(const node &other) {
                data_ptr = new T *[NodeSize];
                node_used_size = other.node_used_size;
                for (type_size i = 0; i < node_used_size; ++i)
                    data_ptr[i] = new T(*(other.data_ptr[i]));
            }

            node(node *_prv, node *_nxt, deque *_deque_ptr, type_size _deque_pos) :
                    prv(_prv), nxt(_nxt), deque_ptr(_deque_ptr),
                    deque_pos(_deque_pos), node_used_size(0) {
                data_ptr = new T *[NodeSize];
            }

            node(node *_prv, deque *_deque_ptr, type_size _deque_pos, bool is_tail) ://用于tail节点
                    prv(_prv),
                    nxt(nullptr),
                    deque_ptr(_deque_ptr),
                    deque_pos(_deque_pos),
                    node_used_size(0),
                    data_ptr(nullptr) {}

            ~node() {
                delete[]data_ptr;
            }

            ////后续结点的deque_pos记得减1
            void erase(type_size pos) {
                delete data_ptr[pos];
                for (type_size i = pos + 1; i < node_used_size; ++i)
                    data_ptr[i - 1] = data_ptr[i];
                --node_used_size;
            }

            ///insert erase调用前确认合法
            void insert(type_size pos, const T &value) {
                if (node_used_size)
                    for (int i = node_used_size - 1; i >= (int) pos; --i)
                        data_ptr[i + 1] = data_ptr[i];
                data_ptr[pos] = new T(value);
                ++node_used_size;
            }
        };

        void modify_node_deque_pos(node *a, int adder) {
            for (node *tmp = a->nxt; tmp; tmp = tmp->nxt)
                tmp->deque_pos += adder;
        }
/*
        void modify_node(node * a)
        {
            if(a->node_used_size >= MergePoint && a->node_used_size < SplitPoint || a->deque_ptr->deque_used_size <= MergePoint)
                return;
            if(a->node_used_size == 0)
            {
                a->prv->nxt = a->nxt;
                a->nxt->prv = a->prv;
                delete a;
            }
            if(a->node_used_size  < MergePoint && a->nxt &&
            a->nxt->node_used_size + a->node_used_size < SplitPoint)
                merge(a);
            else if(a->node_used_size == SplitPoint)
                split(a);
        }*/

        void merge(node *a) {

#ifdef DEBUG
            std::cout << "Before Merged: L122";
            printNodeSize();
#endif
#ifdef printMergeSplitNodeSize
      std::cout << "Before Merged:";
      if(a->prv)
          std::cout << "a->prv: " << a->prv->node_used_size << '\t';
      std::cout << "a: " << a->node_used_size << '\t';

#endif


            node *nxt_ptr = a->nxt;
            //向后merge
            while (nxt_ptr && nxt_ptr->data_ptr
                   //&& nxt_ptr->node_used_size + a->node_used_size >= MergePoint
                   && nxt_ptr->node_used_size + a->node_used_size < SplitPoint) {
#ifdef printMergeSplitNodeSize
                std::cout << "Merging nxt...:";

#endif
                for (type_size i = 0; i < nxt_ptr->node_used_size; ++i)
                    a->data_ptr[a->node_used_size + i] = nxt_ptr->data_ptr[i];
                a->node_used_size += nxt_ptr->node_used_size;
                if (nxt_ptr->nxt) {
                    nxt_ptr->nxt->prv = a;
                }
                a->nxt = nxt_ptr->nxt;
                delete nxt_ptr;
                nxt_ptr = a->nxt;//更新nxt_ptr;
            }

            // if(a != a->deque_ptr->head) {
            node *prv_ptr = a->prv;
            if(prv_ptr
               //&& prv_ptr->node_used_size + a->node_used_size >= MergePoint
               && prv_ptr->node_used_size + a->node_used_size < SplitPoint) {
                for (type_size i = 0; i < a->node_used_size; ++i)
                    prv_ptr->data_ptr[prv_ptr->node_used_size + i] = a->data_ptr[i];
                prv_ptr->node_used_size += a->node_used_size;
                if (nxt_ptr) {
                    nxt_ptr->prv = prv_ptr;
                }
                prv_ptr->nxt = nxt_ptr;
                delete a;
            }
            //  }
#ifdef DEBUG
            std::cout << "After Merged: L157";
            printNodeSize();
#endif
        }

        void split(node *a) {
#ifdef DEBUG
            std::cout << "Before Splited: L164";
            printNodeSize();
#endif
            node *nxt_ptr = a->nxt,
                    *tmp = new node(a, nxt_ptr, a->deque_ptr, a->deque_pos + SplitLeft);

            for (type_size i = SplitLeft; i < a->node_used_size; ++i)
                tmp->data_ptr[i - SplitLeft] = a->data_ptr[i];

            tmp->node_used_size = SplitPoint - SplitLeft;
            a->node_used_size = SplitLeft;

            if (nxt_ptr)
                nxt_ptr->prv = tmp;
            a->nxt = tmp;
#ifdef DEBUG
            std::cout << "After Splited: L180";
            printNodeSize();
#endif
        }

    public:
        class iterator;

        class const_iterator;

    private:
        iterator pos_iterator(const type_size &pos) {
            if (pos >= deque_used_size)
                return iterator(tail, 0);
            if (pos == deque_used_size - 1)
                return iterator(tail->prv, tail->prv->node_used_size - 1);
            node *p = head;
            while (p->nxt->deque_pos <= pos)
                p = p->nxt;
            return iterator(p, (pos - p->deque_pos));
        }

        const_iterator pos_const_iterator(const type_size &pos) {
            if (pos == deque_used_size)
                return const_iterator(tail, 0);
            if (pos == deque_used_size - 1)
                return const_iterator(tail->prv, tail->prv->node_used_size - 1);
            node *p = head;
            while (p->nxt->deque_pos <= pos)
                p = p->nxt;
            return const_iterator(p, (pos - p->deque_pos));
        }

    public:
        class const_iterator;

        class iterator {
        private:
            node *node_ptr;
            type_size node_pos;//0 based
            friend class deque<T>;

        public:
            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, the behaviour is **undefined**.
             * as well as operator-
             */
            iterator() :
                    node_ptr(nullptr), node_pos(0) {}

            iterator(const iterator &rhs) :
                    node_ptr(rhs.node_ptr),
                    node_pos(rhs.node_pos) {}

            iterator(node *_node_ptr, type_size _node_pos) :
                    node_ptr(_node_ptr), node_pos(_node_pos) {}

            iterator operator+(const int &n) const {
                type_size rst_deque_pos = (node_ptr->deque_pos + node_pos + n);
                return (node_ptr->deque_ptr)->pos_iterator(rst_deque_pos);
            }

            iterator operator-(const int &n) const {
                type_size rst_deque_pos = (node_ptr->deque_pos + node_pos - n);
                return (node_ptr->deque_ptr)->pos_iterator(rst_deque_pos);
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const {
                if (node_ptr->deque_ptr != rhs.node_ptr->deque_ptr)
                    throw invalid_iterator();
                type_size pos1 = (node_ptr->deque_pos + node_pos);
                type_size pos2 = (rhs.node_ptr->deque_pos + rhs.node_pos);
                return (pos1 - pos2);
            }

            iterator &operator+=(const int &n) {

                type_size rst_deque_pos = node_ptr->deque_pos + node_pos + n;
                if (rst_deque_pos >= node_ptr->deque_ptr->deque_used_size) {
                    node_ptr = node_ptr->deque_ptr->tail;
                    node_pos = 0;
                } else if (rst_deque_pos == node_ptr->deque_ptr->deque_used_size - 1) {
                    node_ptr = node_ptr->deque_ptr->tail->prv;
                    node_pos = node_ptr->node_used_size - 1;
                } else {
                    node_ptr = (node_ptr->deque_ptr)->head;
                    while (node_ptr && node_ptr->nxt->deque_pos <= rst_deque_pos)
                        node_ptr = node_ptr->nxt;
                    node_pos = rst_deque_pos - node_ptr->deque_pos;
                }
                return (*this);
            }

            iterator &operator-=(const int &n) {
                type_size rst_deque_pos = node_ptr->deque_pos + node_pos - n;
                if (rst_deque_pos >= node_ptr->deque_ptr->deque_used_size) {
                    node_ptr = node_ptr->deque_ptr->tail;
                    node_pos = 0;
                } else if (rst_deque_pos == node_ptr->deque_ptr->deque_used_size - 1) {
                    node_ptr = node_ptr->deque_ptr->tail->prv;
                    node_pos = node_ptr->node_used_size - 1;
                } else {
                    node_ptr = (node_ptr->deque_ptr)->head;
                    while (node_ptr && node_ptr->nxt->deque_pos <= rst_deque_pos)
                        node_ptr = node_ptr->nxt;
                    node_pos = rst_deque_pos - node_ptr->deque_pos;
                }
                return (*this);
            }

            //iter++
            iterator operator++(int) {
                if (node_ptr->data_ptr == nullptr)
                    return (*this);
                iterator rst(*this);

                if (node_pos < node_ptr->node_used_size - 1)
                    ++node_pos;
                else {
                    node_ptr = node_ptr->nxt;
                    node_pos = 0;
                }
                return rst;
            }

            //++iter
            iterator &operator++() {
                if (node_ptr->data_ptr == nullptr)
                    return (*this);
                if (node_pos < node_ptr->node_used_size - 1)
                    ++node_pos;
                else {
                    node_ptr = node_ptr->nxt;
                    node_pos = 0;
                }
                return (*this);
            }

            //iter--
            iterator operator--(int) {
                iterator rst(*this);
                if (node_pos > 0)
                    --node_pos;
                else if (node_ptr == node_ptr->deque_ptr->head) {
                    throw invalid_iterator();//begin()--;
                } else {
                    node_ptr = node_ptr->prv;
                    node_pos = node_ptr->node_used_size - 1;
                }
                return rst;
            }

            //--iter
            iterator &operator--() {
                if (node_pos > 0)
                    --node_pos;
                else if (node_ptr == node_ptr->deque_ptr->head)
                    throw invalid_iterator();//--begin()
                else {
                    node_ptr = node_ptr->prv;
                    node_pos = node_ptr->node_used_size - 1;
                }
                return (*this);

            }

            // *it
            T &operator*() const {
                if (node_ptr->data_ptr == nullptr)
                    throw invalid_iterator();
                return *(node_ptr->data_ptr[node_pos]);
            }

            //it->field
            T *operator->() const noexcept /////
            {
                //if(node_ptr->data_ptr == nullptr)throw invalid_iterator();
                return (node_ptr->data_ptr[node_pos]);
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                return (node_ptr == rhs.node_ptr && node_pos == rhs.node_pos) ||
                       (node_ptr->deque_ptr == rhs.node_ptr->deque_ptr &&
                        node_ptr->data_ptr == nullptr && rhs.node_ptr->data_ptr == nullptr);
            }

            bool operator==(const const_iterator &rhs) const {
                return (node_ptr == rhs.node_ptr && node_pos == rhs.node_pos) ||
                       (node_ptr->deque_ptr == rhs.node_ptr->deque_ptr &&
                        node_ptr->data_ptr == nullptr && rhs.node_ptr->data_ptr == nullptr);
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return (node_ptr != rhs.node_ptr || node_pos != rhs.node_pos) &&
                       (node_ptr->deque_ptr != rhs.node_ptr->deque_ptr ||
                        node_ptr->data_ptr != nullptr || rhs.node_ptr->data_ptr != nullptr);
            }

            bool operator!=(const const_iterator &rhs) const {

                return (node_ptr != rhs.node_ptr || node_pos != rhs.node_pos) &&
                       (node_ptr->deque_ptr != rhs.node_ptr->deque_ptr ||
                        node_ptr->data_ptr != nullptr || rhs.node_ptr->data_ptr != nullptr);
            }
        };

        class const_iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            const node *node_ptr;
            type_size node_pos;//0 based
            friend class deque<T>;

        public:
            const_iterator() : node_ptr(nullptr), node_pos(0) {}

            const_iterator(const const_iterator &other) :
                    node_ptr(other.node_ptr),
                    node_pos(other.node_pos) {}

            const_iterator(const iterator &other) :
                    node_ptr(other.node_ptr),
                    node_pos(other.node_pos) {}

            const_iterator(const node *_node_ptr, type_size _node_pos) :
                    node_ptr(_node_ptr), node_pos(_node_pos) {}

            const_iterator operator+(const int &n) const {
                type_size rst_deque_pos = (node_ptr->deque_pos + node_pos + n);
                return (node_ptr->deque_ptr)->pos_const_iterator(rst_deque_pos);
            }

            const_iterator operator-(const int &n) const {
                type_size rst_deque_pos = (node_ptr->deque_pos + node_pos - n);
                return (node_ptr->deque_ptr)->pos_const_iterator(rst_deque_pos);
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.
            int operator-(const const_iterator &rhs) const {
                if (node_ptr->deque_ptr != rhs.node_ptr->deque_ptr)
                    throw invalid_iterator();
                type_size pos1 = (node_ptr->deque_pos + node_pos);
                type_size pos2 = (rhs.node_ptr->deque_pos + rhs.node_pos);
                return (pos1 - pos2);
            }

            const_iterator &operator+=(const int &n) {
                type_size rst_deque_pos = node_ptr->deque_pos + node_pos + n;
                node_ptr = (node_ptr->deque_ptr)->head;
                while (node_ptr->nxt->deque_pos < rst_deque_pos)
                    node_ptr = node_ptr->nxt;
                node_pos = rst_deque_pos - node_ptr->deque_pos;
                return (*this);
            }

            const_iterator &operator-=(const int &n) {
                type_size rst_deque_pos = node_ptr->deque_pos + node_pos - n;
                node_ptr = (node_ptr->deque_ptr)->head;
                while (node_ptr->nxt->deque_pos < rst_deque_pos)
                    node_ptr = node_ptr->nxt;
                node_pos = rst_deque_pos - node_ptr->deque_pos;
                return (*this);
            }

            //iter++
            const_iterator operator++(int) {
                if (node_ptr->data_ptr == nullptr)
                    return (*this);
                const_iterator rst(*this);
                if (node_pos < node_ptr->node_used_size - 1)
                    ++node_pos;
                else {
                    node_ptr = node_ptr->nxt;
                    node_pos = 0;
                }
                return rst;
            }

            //++iter
            const_iterator &operator++() {
                if (node_ptr->data_ptr == nullptr)
                    return (*this);
                if (node_pos < node_ptr->node_used_size - 1)
                    ++node_pos;
                else {
                    node_ptr = node_ptr->nxt;
                    node_pos = 0;
                }
                return (*this);
            }

            //iter--
            const_iterator operator--(int) {
                iterator rst(*this);
                if (node_pos > 0)
                    --node_pos;
                else {
                    node_ptr = node_ptr->prv;
                    node_pos = node_ptr->node_used_size - 1;
                }
                return rst;
            }

            //--iter
            const_iterator &operator--() {
                if (node_pos > 0)
                    --node_pos;
                else {
                    node_ptr = node_ptr->prv;
                    node_pos = node_ptr->node_used_size - 1;
                }
                return (*this);

            }

            // *it
            const T &operator*() const {
                return *(node_ptr->data_ptr[node_pos]);
            }

            //it->field
            const T *operator->() const noexcept /////
            {

                return (node_ptr->data_ptr[node_pos]);
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator==(const iterator &rhs) const {
                return (node_ptr == rhs.node_ptr && node_pos == rhs.node_pos) ||
                       (node_ptr->deque_ptr == rhs.node_ptr->deque_ptr &&
                        node_ptr->data_ptr == nullptr && rhs.node_ptr->data_ptr == nullptr);
            }

            bool operator==(const const_iterator &rhs) const {
                return (node_ptr == rhs.node_ptr && node_pos == rhs.node_pos) ||
                       (node_ptr->deque_ptr == rhs.node_ptr->deque_ptr &&
                        node_ptr->data_ptr == nullptr && rhs.node_ptr->data_ptr == nullptr);
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {
                return (node_ptr != rhs.node_ptr || node_pos != rhs.node_pos) &&
                       (node_ptr->deque_ptr != rhs.node_ptr->deque_ptr ||
                        node_ptr->data_ptr != nullptr || rhs.node_ptr->data_ptr != nullptr);
            }

            bool operator!=(const const_iterator &rhs) const {

                return (node_ptr != rhs.node_ptr || node_pos != rhs.node_pos) &&
                       (node_ptr->deque_ptr != rhs.node_ptr->deque_ptr ||
                        node_ptr->data_ptr != nullptr || rhs.node_ptr->data_ptr != nullptr);
            }

        };

        deque() : deque_used_size(0) {
            head = new node(nullptr, nullptr, this, 0);
            tail = new node(nullptr, this, 0, 1);
            head->nxt = tail;
            tail->prv = head;
        }

        deque(const deque &other) {
            node *p1 = other.head;
            head = new node(*p1);
            node *p2 = head;
            p2->prv = nullptr;////
            p2->deque_ptr = this;
            p2->deque_pos = 0;
            while (p1->nxt->data_ptr) {
                p2->nxt = new node(*(p1->nxt));
                p2->nxt->prv = p2;
                p2->nxt->deque_ptr = this;
                p2->nxt->deque_pos = p1->nxt->deque_pos;

                p1 = p1->nxt;
                p2 = p2->nxt;
            }
            //tail
            tail = new node(p2, this, other.deque_used_size, 1);
            p2->nxt = tail;

            deque_used_size = other.deque_used_size;
        }

        ~deque() {
            node *p = head, *q;
            while (p && p->data_ptr) {////???
                q = p->nxt;
                for (type_size i = 0; i < p->node_used_size; ++i)
                    delete p->data_ptr[i];
                delete p;
                p = q;
            }
            delete tail;
        }

        //assignment operator
        deque &operator=(const deque &other) {
            if (&other == this) return *this;

            node *p = head, *q;
            while (p->data_ptr) {
                q = p->nxt;
                for (type_size i = 0; i < p->node_used_size; ++i)
                    delete p->data_ptr[i];
                delete p;
                p = q;
            }
            delete tail;

            node *p1 = other.head;
            head = new node(*p1);
            node *p2 = head;
            p2->deque_ptr = this;
            p2->deque_pos = 0;
            while (p1->nxt->data_ptr) {
                p2->nxt = new node(*(p1->nxt));
                p2->nxt->prv = p2;
                p2->nxt->deque_ptr = this;
                p2->nxt->deque_pos = p1->nxt->deque_pos;

                p1 = p1->nxt;
                p2 = p2->nxt;
            }
            //tail
            tail = new node(p2, this, other.deque_used_size, 1);
            p2->nxt = tail;

            deque_used_size = other.deque_used_size;

            return *this;
        }

        /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
        T &at(const size_t &pos) {
            if (pos < 0 || pos >= deque_used_size)
                throw index_out_of_bound();
            node *p = head;
            while (p->nxt->deque_pos <= pos)
                p = p->nxt;
            return *(p->data_ptr[pos - p->deque_pos]);
        }

        const T &at(const size_t &pos) const {
            if (pos < 0 || pos >= deque_used_size)
                throw index_out_of_bound();
            node *p = head;
            while (p->nxt->deque_pos <= pos)
                p = p->nxt;
            return *(p->data_ptr[pos - p->deque_pos]);
        }

        T &operator[](const size_t &pos) {
            if (pos < 0 || pos >= deque_used_size)
                throw index_out_of_bound();
            node *p = head;
            while (p->nxt && p->nxt->deque_pos <= pos)
                p = p->nxt;
            return *(p->data_ptr[pos - p->deque_pos]);

        }

        const T &operator[](const size_t &pos) const {
            if (pos < 0 || pos >= deque_used_size)
                throw index_out_of_bound();
            node *p = head;
            while (p->nxt->deque_pos <= pos)
                p = p->nxt;
            return *(p->data_ptr[pos - p->deque_pos]);
        }

        /** access the first element
         *  throw container_is_empty when the container is empty.
         */
        const T &front() const {
            if (deque_used_size == 0)
                throw container_is_empty();
            return *(head->data_ptr[0]);
        }

        /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
        const T &back() const {
            if (deque_used_size == 0)
                throw container_is_empty();
            node *p = tail->prv;
            return *(p->data_ptr[p->node_used_size - 1]);
        }

        //returns an iterator to the beginning.
        iterator begin() {
            if (deque_used_size == 0)
                return iterator(tail, 0);
            return iterator(head, 0);
        }

        const_iterator cbegin() const {
            return const_iterator(head, 0);
        }

        //returns an iterator to the end.
        iterator end() {
            return iterator(tail, 0);
        }

        const_iterator cend() const {
            return const_iterator(tail, 0);
        }

        // checks whether the container is empty.
        bool empty() const {
            return deque_used_size == 0;
        }

        /**
         * returns the number of elements
         */
        size_t size() const {
            return deque_used_size;
        }

        /**
         * clears the contents
         */
        void clear() {
            node *p = head, *q;
            while (p->data_ptr) {
                q = p->nxt;
                for (type_size i = 0; i < p->node_used_size; ++i)
                    delete p->data_ptr[i];
                delete p;
                p = q;
            }
            delete tail;
            deque_used_size = 0;
            head = new node(nullptr, nullptr, this, 0);
            tail = new node(nullptr, this, 0, 1);
            head->nxt = tail;
            tail->prv = head;
        }

        /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
        iterator insert(iterator pos, const T &value) {   //插入完可能需要分裂
            if (pos.node_ptr->deque_ptr != this)
                throw invalid_iterator();
            if (pos.node_pos < 0 || pos.node_pos > pos.node_ptr->node_used_size)
                throw invalid_iterator();
            if (pos.node_ptr->data_ptr) {
                pos.node_ptr->insert(pos.node_pos, value);
                modify_node_deque_pos(pos.node_ptr, 1);

                if (pos.node_ptr->node_used_size == SplitPoint)
                    split(pos.node_ptr);

               // modify_node(pos.node_ptr);
                ////
                type_size rst_pos = pos.node_pos + (pos.node_ptr->deque_pos);
                ++deque_used_size;
                return pos_iterator(rst_pos);
            } else {//end() iterator
                node *p = pos.node_ptr->prv;
                type_size rst_node_pos = pos.node_ptr->prv->node_used_size;
                p->insert(rst_node_pos, value);
                modify_node_deque_pos(p, 1);

                if (p->node_used_size == SplitPoint)
                    split(p);

                //modify_node(pos.node_ptr);
                ++deque_used_size;
                return pos_iterator(deque_used_size - 1);
            }
        }

        /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
        iterator erase(iterator pos) {//删除完可能需要合并
            if (deque_used_size == 0 || pos.node_pos < 0 || pos.node_pos > pos.node_ptr->node_used_size)
                throw invalid_iterator();
            pos.node_ptr->erase(pos.node_pos);
            modify_node_deque_pos(pos.node_ptr, -1);
            --deque_used_size;//////位置？？
            type_size rst_pos = pos.node_pos + (pos.node_ptr->deque_pos);

            if(pos.node_ptr->node_used_size < MergePoint && (
            (pos.node_ptr->nxt && pos.node_ptr->nxt->data_ptr && pos.node_ptr->node_used_size + pos.node_ptr->nxt->node_used_size < SplitPoint)
            ||
            (pos.node_ptr->prv && pos.node_ptr->node_used_size + pos.node_ptr->prv->node_used_size < SplitPoint)
                                                            )
            )
                merge(pos.node_ptr);
            ////
            //modify_node(pos.node_ptr);
            return pos_iterator(rst_pos);
        }

        /**
         * adds an element to the end
         */

        void push_back(const T &value) {
            node *p = tail->prv;
            p->data_ptr[p->node_used_size] = new T(value);
            ++p->node_used_size;
            ++tail->deque_pos;

            if (p->node_used_size == SplitPoint)
                split(p);

            //modify_node(p);
            ++deque_used_size;
        }

        /**
         * removes the last element
         *     throw when the container is empty.
         */
        void pop_back() {
            //erase((--end()));
            if (deque_used_size == 0)
                throw container_is_empty();
            node *p = tail->prv;
            delete p->data_ptr[p->node_used_size - 1];
            --p->node_used_size;
             if(p->node_used_size < MergePoint && p->prv && p->prv->node_used_size + p->node_used_size < SplitPoint)merge(p);
           //modify_node(p);
            --deque_used_size;
            --tail->deque_pos;
        }

        /**
         * inserts an element to the beginning.
         */
        void push_front(const T &value) {
            //insert(begin(),value);

            if (head->node_used_size)
                for (int i = head->node_used_size - 1; i >= (int) 0; --i)
                    head->data_ptr[i + 1] = head->data_ptr[i];
            head->data_ptr[0] = new T(value);
            ++head->node_used_size;
            ++deque_used_size;
            modify_node_deque_pos(head, 1);
            if (head->node_used_size == SplitPoint) split(head);
           //modify_node(head);
        }

        /**
         * removes the first element.
         *     throw when the container is empty.
         */
        void pop_front() {
            if (deque_used_size == 0)
                throw container_is_empty();
            if (deque_used_size == 0 || head->node_used_size <= 0)
                throw invalid_iterator();
            head->erase(0);
            modify_node_deque_pos(head, -1);
            --deque_used_size;//////位置？？
           merge(head);
          //modify_node(head);
            ////
        }
    };

}

#endif

