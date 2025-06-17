#ifndef _TREE_HPP_
#define _TREE_HPP_

#include "Geo.hpp"
#include <memory>
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
        PointVector position;
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
        constexpr bool contains(const PointVector &pos, const RectLooseness &looseness) const
        {return m_boundingBox.contains(pos, looseness);}
    };
    template<typename T>
    struct _Node : _NodeBase
    {
        _Node *m_parent;
        _Node *m_children[4]; // NE, NW, SW, SE
        std::list<_QTData<T>> m_data;
        _Node(const Rect &boundingBox, _Node *parent)
            : _NodeBase(boundingBox), m_parent(parent), m_data(){}
        constexpr NodeType *parent() const { return m_parent; }
        constexpr _Node *(const &children())[4] const { return m_children; }
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
        size_t clearSubtree(void)
        {
            _clearSubtree(this);
            m_isLeaf = true; // Mark this as a leaf node
            for(_Node *&child : m_children)
                child = nullptr; // Set to nullptr to avoid dangling pointers
            size_t countTmp = m_count;
            m_count = 0;
            return countTmp;
        }
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

    constexpr RectLooseness _completelyLoose = {{true, true}, {true, true}};

    template<typename T>
    void _divideNode(_Node<T> *node)
    {
        constexpr bool vertexAttrs[4][2] = {
            {true, true},
            {true, false},
            {false, false},
            {false, true}
        };
        PointVector center = node->boundingBox().center();
        for(size_t i = 0; i < 4; ++i)
            node->m_children[i] = new NodeType(Rect(node->boundingBox().vertex(vertexAttrs[i]), center), node);
        while(!node->m_data.empty())
        {
            auto bg = node->m_data.begin();
            const PointVector &pos = bg->pos;
            size_t idx = 0;
            if(pos[0] < center[0])
            {
                if(pos[1] < center[1])
                    idx = 2;
                else
                    idx = 1;
            }
            else
            {
                if(pos[1] < center[1])
                    idx = 3;
                else
                    idx = 0;
            }
            _Node<T> *child = node->children()[idx];
            child->m_data.splice(child->m_data.end(), node->m_data, bg);
            ++(child->m_count);
        }
        node->m_isLeaf = false; // Mark this as an internal node now
        node->m_data.clear();
    }

    template<typename T>
    _Node<T> *_mergeNode(_Node<T> *node, size_t countLimit, size_t _deleted = 1, size_t *rewindCount = nullptr)
    {
        _Node<T> *ret = node;
        size_t rewCount = 0;
        for(node = node->m_parent; node && (node->m_count - _deleted) <= countLimit; node = node->m_parent)
        {
            node->m_count -= _deleted; // Decrement the count of the parent node
            for(_Node<T> *&child : node->children())
            {
                node->m_data.splice(node->m_data.end(), child->m_data); // Move data from child to parent
                delete child; // Delete all children
                child = nullptr; // Set to nullptr to avoid dangling pointers
            }
            ret = node;
            ++rewCount;
        }
        ret->m_isLeaf = true; // Mark this as a leaf now
        if(rewindCount)
            *rewindCount = rewCount;
        return ret;
    }

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
    _Node<T> *_queryLeafByPoint(_Node<T> *node, PointVector pos, const RectLooseness &looseMode = {})
    {
        if(!node || !node->contains(pos, looseMode))
            return nullptr; // No intersection, return early
        while(!node->isLeaf())
        {
            bool found = false; // For security.
            for(size_t i = 0; i < 4; ++i) if(node->children()[i].contains(pos, looseMode & mask[i]))
            {
                node = node->children()[i]; // Move to the child node
                found = true;
                looseMode = looseMode & mask[i];
                break; // Found the child, no need to check others
            }
            if(!found)
                return nullptr;
        }
        return node;
    }
    template<typename T>
    _Node<T> *_queryLeafByRect(_Node<T> *node, PointVector pos, const RectLooseness &looseMode = {})
    {
        if(!node || !node->contains(pos, looseMode))
            return nullptr;
        return node;
    }
    template<typename T, typename LeafQuery>
    _Node<T> *_leaf(_Node<T> *node, LeafQuery &&lq, size_t countLimit)
    {
        size_t before = node->m_count;
        lq(node->m_data);
        size_t after = node->m_count;
        if(after < before)
            _applyDelete(node, countLimit, before - after);
        else if(after > before)
            _applyInsert(node, countLimit, after - before);
        
    }
    template<typename T, typename LeafQuery>
    size_t _dfs(_Node<T> *node, LeafQuery &&lq, size_t countLimit)
    {
        if(node.isLeaf())
            _leaf(node, std::forward<LeafQuery>(lq), countLimit);
        else for(_Node<T> *child : node->children())
        {
            size_t rewind = _dfs(child, std::forward<LeafQuery>(lq), countLimit);
            if(rewind)
                return rewind - 1;
        }
        return 0;
    }
}

template<typename T>
class QuadTree
{
public:
    typedef _qt::_QTData LocatedData;
protected:
    typedef _qt::_Node NodeType;
    NodeType m_root;
    size_t m_countLimit;

    constexpr NodeType *_rootNodePtr(void) const
    {return &m_root;}
    QuadTree(const Rect &boundingBox, size_t countLimit)
        : m_root(boundingBox, nullptr, true), m_countLimit(countLimit)
    {}
public:
    void clear(void)
    {_rootNodePtr()->clearSubtree();}
public:
    QuadTree(const Rect &boundingBox, size_t countLimit, InterpretCoordinateFunc icf)
        : _QuadTreeBase<T>(boundingBox, countLimit), m_icf(icf){}

    template<typename LeafNodeQueryFunc>
    void queryRect(const Rect &searchArea, LeafNodeQueryFunc &&func) const
    {_queryRect(this->_rootNodePtr(), searchArea, std::forward<LeafNodeQueryFunc>(func));}

    void insert(const PointVector &pos, const T &data)
    {
        NodeType *node = _queryLeafByPoint(this->_rootNodePtr(), pos, _qt::_completelyLoose);
        node->m_data.push_back(data); // Insert data into the leaf node
        ++(node->m_count);
        _qt::_applyInsert(node); // Apply insert logic
    }
    void insert(const PointVector &pos, T &&data)
    {
        NodeType *node = _queryLeafByPoint(this->_rootNodePtr(), pos, _qt::_completelyLoose);
        node->m_data.push_back(std::move(data)); // Insert data into the leaf node
        ++(node->m_count);
        _qt::_applyInsert(node); // Apply insert logic
    }

    template<typename _LeafQuery>
    void 

    template<typename _LeafQuery>
    void dfs(_LeafQuery &&lq)
    {

    }
protected:
    ~QuadTree()
    {
        _clearAll(_rootNodePtr());
    }
};
#endif // _TREE_HPP_