#ifndef _TREE_HPP_
#define _TREE_HPP_

#include "Geo.hpp"
#include <stdio.h>
#include <list>

// Is it 2D version of segment tree?
// Copilot, ask there.
// QuadTreeBase is a base class for a quadtree structure, which is used for spatial partitioning in 2D space.
// So, is it a variation of segment tree? Please answer in a single line.
// Yes, QuadTreeBase is a variation of segment tree designed for spatial partitioning in 2D space.

namespace _qt
{
    template<typename T>
    struct _QTData
    {
        PointVector pos;
        T data;
    };
    struct _NodeBase
    {
        bool m_isLeaf;
        Rect m_boundingBox;
        size_t m_count; // Number of elements in this node
        constexpr _NodeBase(const Rect &boundingBox)
            : m_isLeaf(true), m_boundingBox(boundingBox), m_count(0){}
        constexpr bool isLeaf() const { return m_isLeaf; }
        constexpr const Rect &boundingBox() const { return m_boundingBox; }
        constexpr size_t count() const{return m_count;}
        constexpr bool contains(const PointVector &pos) const
        {return m_boundingBox.contains(pos);}
        constexpr bool intersects(const Rect &rect) const{return m_boundingBox.intersects(rect);}
    };
    constexpr bool _vertexAttrs[4][2] = {
        {true, true},
        {false, true},
        {false, false},
        {true, false}
    };
    constexpr RectLooseness _mask[4] = {
        RectLooseness({{false, true},
            {false, true}}),
        RectLooseness({{true, false},
            {false, true}}),
        RectLooseness({{true, false},
            {true, false}}),
        RectLooseness({{false, true},
            {true, false}})
    };
    template<typename T>
    struct _Node : _NodeBase
    {
        _Node *m_parent;
        _Node *m_children[4]; // NE, NW, SW, SE
        std::list<_QTData<T>> m_data;
        _Node(const Rect &boundingBox, _Node *parent)
            : _NodeBase(boundingBox), m_parent(parent), m_data(){}
        constexpr _Node *parent() const { return m_parent; }
        constexpr _Node *const(&children() const)[4]{ return m_children; }
    private:
        void _clearSubtree(void)
        {
            if(isLeaf())
                m_data.clear();
            else for(_Node *child : m_children)
            {
                child->_clearSubtree();
                delete child; // Delete the child node
            }
        }
    public:
        void divideNode(void)
        {
            if(isLeaf())
            {
                PointVector center = m_boundingBox.center();
                for(size_t i = 0; i < 4; ++i)
                {
                    Rect r = Rect(m_boundingBox.vertex(_vertexAttrs[i]), center, m_boundingBox.looseness() & _mask[i]);
                    m_children[i] = new _Node<T>(r, this);
                    //printf("(%lf, %lf) - (%lf, %lf), {{%d, %d}, {%d, %d}}\n", r.minPoint()[0], r.minPoint()[1], r.maxPoint()[0], r.maxPoint()[1], r.looseness().looseMin(0), r.looseness().looseMax(0), r.looseness().looseMin(1), r.looseness().looseMax(1));
                }
            }
            else return;
            while(!m_data.empty())
            {
                auto bg = m_data.begin();
                for(_Node<T> *child : children()) if(child->contains(bg->pos))
                {
                    child->m_data.splice(child->m_data.end(), m_data, bg);
                    break;
                }
            }
            m_isLeaf = false; // Mark this as an internal node now
            m_data.clear();
        }
        size_t clearSubtree(void)
        {
            _clearSubtree();
            m_isLeaf = true; // Mark this as a leaf node
            for(_Node *&child : m_children)
                child = nullptr; // Set to nullptr to avoid dangling pointers
            size_t countTmp = m_count;
            m_count = 0;
            return countTmp;
        }
    };

    constexpr RectLooseness _completelyLoose({{true, true}, {true, true}});

    template<typename T>
    void _applyInsert(_Node<T> *node, size_t _inserted = 1)
    {
        while((node = node->m_parent)) // Moving up to the parent first because we already took decrement of count when we checked the parent
            node->m_count += _inserted;
    }

    template<typename T>
    void _applyDelete(_Node<T> *node, size_t countLimit, size_t _deleted = 1)
    {
        while((node = node->m_parent)) // Moving up to the parent first because we already took decrement of count when we checked the parent
            node->m_count -= _deleted;
    }

    template<typename T>
    _Node<T> *_queryLeafByPoint(_Node<T> *node, PointVector pos)
    {
        if(!node || !node->contains(pos))
            return nullptr; // No intersection, return early
        while(!node->isLeaf())
        {
            bool found = false; // For security.
            for(_Node<T> *child : node->children()) if(child->contains(pos))
            {
                node = child; // Move to the child node
                found = true;
                break; // Found the child, no need to check others
            }
            if(!found)
                return nullptr;
        }
        return node;
    }

    template<typename T>
    void _reinsertNode(_Node<T> *node, typename std::list<_QTData<T>>::iterator itr, size_t countLimit)
    {
        _Node<T> *existing = node;
        PointVector pos = itr->pos;
        size_t i = 0;
        --(node->m_count);
        for(node = node->parent(); node; node = node->parent())
        {
            if(node->contains(pos))
            {
                while(!node->isLeaf())
                {
                    bool found = false;
                    for(_Node<T> *child : node->children()) if(child->contains(pos))
                    {
                        node = child;
                        ++(node->m_count);
                        found = true;
                        break;
                    }
                    if(!found)
                    {
                        existing->m_data.erase(itr);
                        return;
                    }
                }
                node->m_data.splice(node->m_data.end(), existing->m_data, itr);
                if(node->m_count > countLimit)
                    node->divideNode();
                return;
            }
            --(node->m_count);
        }
        existing->m_data.erase(itr);
    }
    template<typename T>
    struct _MergeByRect
    {
        size_t countLimit;
        const Rect &rect;
        void operator()(_Node<T> *node)
        {
            if(!node || node->isLeaf() || !node->intersects(rect))
                return;
            for(_Node<T> *child : node->children())
                (*this)(child);
            if(node->m_count <= countLimit)
            {
                for(_Node<T> *&child : node->m_children)
                {
                    node->m_data.splice(node->m_data.end(), child->m_data); // Move data from child to parent
                    delete child; // Delete all children
                    child = nullptr;
                }
                node->m_isLeaf = true;
            }
        }
    };
    template<typename T, typename LeafQuery>
    struct _QueryLeafByRect
    {
        LeafQuery lq;
        size_t countLimit;
        const Rect &rect;
        void _leaf(_Node<T> *node)
        {
            if(!node->intersects(rect))
                return;
            size_t before = node->m_count;
            lq(node->m_data);
            size_t after = (node->m_count = node->m_data.size());
            if(after < before)
                _applyDelete(node, countLimit, before - after);
            else if(after > before)
                _applyInsert(node, after - before);
            auto i = node->m_data.begin();
            while(i != node->m_data.end())
            {
                auto tail = i;
                ++i;
                if(!node->contains(tail->pos))
                    _reinsertNode(node, tail, countLimit);
            }
        }
        void operator()(_Node<T> *node)
        {
            if(!node || !node->intersects(rect))
                return;
            else if(node->isLeaf())
                _leaf(node);
            else for(_Node<T> *child : node->children())
                (*this)(child);
        }
    };
}

template<typename T>
class QuadTree
{
public:
    typedef _qt::_QTData<T> LocatedData;
    typedef std::list<_qt::_QTData<T>> DataList;
private:
    typedef _qt::_Node<T> NodeType;
    NodeType m_root;
    size_t m_countLimit;
public:
    QuadTree(const Rect &boundingBox, size_t countLimit)
        : m_root(boundingBox, nullptr), m_countLimit(countLimit)
    {}
    template<typename LeafQuery>
    void queryRect(const Rect &searchArea, LeafQuery &&func)
    {
        {
            _qt::_QueryLeafByRect<T, std::add_rvalue_reference_t<LeafQuery> > _temp = {std::forward<LeafQuery>(func), m_countLimit, searchArea};
            _temp(&m_root);
        }
        {
            _qt::_MergeByRect<T> _temp = {m_countLimit, searchArea};
            _temp(&m_root);
        }
    }

    void insert(const PointVector &pos, const T &data)
    {
        NodeType *node = _queryLeafByPoint(&m_root, pos);
        node->m_data.push_back(LocatedData{pos, data}); // Insert data into the leaf node
        if(++(node->m_count) > m_countLimit)
            node->divideNode();
        _qt::_applyInsert(node); // Apply insert logic
    }
    void insert(const PointVector &pos, T &&data)
    {
        NodeType *node = _queryLeafByPoint(&m_root, pos);
        node->m_data.push_back(LocatedData{pos, std::move(data)}); // Insert data into the leaf node
        if(++(node->m_count) > m_countLimit)
            node->divideNode();
        _qt::_applyInsert(node); // Apply insert logic
    }
    void clear(void)
    {m_root.clearSubtree();}
    ~QuadTree()
    {
        clear();
    }
};
#endif // _TREE_HPP_