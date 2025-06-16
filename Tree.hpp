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
template<typename T>
class _QuadTreeBase
{
protected:
    struct NodeType
    {
        bool m_isLeaf;
        Rect m_boundingBox;
        NodeType *m_parent;
        NodeType *m_children[4]; // NE, NW, SW, SE
        size_t m_count; // Number of elements in this node
        std::list<T> m_data;
        inline NodeType(const Rect &boundingBox, NodeType *parent, bool isLeaf = false)
            : m_isLeaf(isLeaf), m_boundingBox(boundingBox), m_parent(parent), m_count(0), m_data(){}
        constexpr bool isLeaf() const { return m_isLeaf; }
        constexpr const Rect &boundingBox() const { return m_boundingBox; }
        constexpr NodeType *parent() const { return m_parent; }
        constexpr size_t count() const{return m_count;}
    }m_root;
    size_t m_countLimit;

    constexpr NodeType *_rootNodePtr(void) const
    {return &m_root;}

    NodeType *_mergeNode(NodeType *node, size_t _deleted)
    {
        node->m_count -= _deleted;
        NodeType *ret = node;
        for(node = node->m_parent; node && (node->m_count - _deleted) <= m_countLimit; node = node->m_parent)
        {
            node->m_count -= _deleted; // Decrement the count of the parent node
            for(NodeType *&child : node->m_children)
            {
                node->m_data.splice(node->m_data.end(), child->m_data); // Move data from child to parent
                delete child; // Delete all children
                child = nullptr; // Set to nullptr to avoid dangling pointers
            }
            node->m_isLeaf = true; // Mark this as a leaf now
            ret = node;
        }
        return ret;
    }

    NodeType *_applyDelete(NodeType *node, size_t _deleted = 1)
    {
        if(_deleted > node->m_count)
            _deleted = node->m_count; // Ensure we don't delete more than available
        NodeType *ret = (node = _mergeNode(node, _deleted));
        while((node = node->m_parent)) // Moving up to the parent first because we already took decrement of count when we checked the parent
            node->m_count -= _deleted;
        return ret;
    }

    static NodeType *_queryLeafByPoint(NodeType *node, PointVector pos)
    {
        if(!node || !node->boundingBox().contains(pos))
            return nullptr; // No intersection, return early
        while(!node->isLeaf())
        {
            bool found = false; // For security.
            for(NodeType *child : node->m_children) if(child->boundingBox().contains(pos))
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

    void __clearAll(NodeType *node)
    {
        if(!node)
            return;
        else if(!node->isLeaf()) for(NodeType *child : node->m_children)
        {
            __clearAll(child);
            delete child; // Delete the child node
        }
    }
    NodeType *_clearAll(NodeType *node)
    {
        if(!node)
            return nullptr;
        __clearAll(node);
        node->m_isLeaf = true; // Mark this as a leaf node
        node->m_data.clear(); // Clear the data in this node
        for(NodeType *&child : node->m_children)
            child = nullptr; // Set to nullptr to avoid dangling pointers
        return _applyDelete(node, node->m_count);
    }
    _QuadTreeBase(const Rect &boundingBox, size_t countLimit)
        : m_root(boundingBox, nullptr, true), m_countLimit(countLimit)
    {}
public:
    void clear(void)
    {
        _clearAll(_rootNodePtr());
    }
protected:
    ~_QuadTreeBase()
    {
        _clearAll(_rootNodePtr());
    }
};
template<typename T, typename InterpretCoordinateFunc>
class QuadTree : public _QuadTreeBase<T>
{
private:
    InterpretCoordinateFunc m_icf;
    typedef typename _QuadTreeBase<T>::NodeType NodeType;
public:
    void _divideNode(NodeType *node)
    {
        constexpr bool vertexAttrs[4][2] = {
            {true, true},
            {true, false},
            {false, false},
            {false, true}
        };
        PointVector center = node->boundingBox().center();
        for(size_t i = 0; i < 4; ++i)
            node->m_children[i] = new NodeType(Rect(node->boundingBox().vertex(vertexAttrs[i]), center), node, true);
        for(auto i = node->m_data; i != node->m_data.end(); ++i)
        {
            for(NodeType *child : node->m_children) if(child->boundingBox().contains(m_icf(*i)))
            {
                child->m_data.splice(child->m_data.end(), node->m_data, i);
                ++(child->m_count);
                break; // Data is inserted, no need to check other children
            }
        }
        node->m_isLeaf = false; // Mark this as an internal node now
        node->m_data.clear();
    }
    void _applyInsert(NodeType *node, size_t _inserted = 1)
    {
        if((node->count() += _inserted) > this->m_countLimit)
            _divideNode(node);
        do
            node->m_count += _inserted;
        while((node = node->m_parent)); // Move up to the parent and increment count
    }
    template<typename LeafNodeQueryFunc>
    void _queryRect(const NodeType *ptr, const Rect &searchArea, LeafNodeQueryFunc &&lcq) const
    {
        if(!ptr || ptr->m_boundingBox.discrete(searchArea))
            return; // No intersection, return early
        else if(ptr->m_isLeaf) for(const auto &i : ptr->m_data) if(searchArea.contains(m_icf(i)))
            lcq(i); // Call the function with the leaf node
        else for(const NodeType *child : ptr->m_children) // If it's an internal node, search its children
            _queryRect(child, searchArea, std::forward<LeafNodeQueryFunc>(lcq));
    }
public:
    QuadTree(const Rect &boundingBox, size_t countLimit, InterpretCoordinateFunc icf)
        : _QuadTreeBase<T>(boundingBox, countLimit), m_icf(icf){}

    template<typename LeafNodeQueryFunc>
    void queryRect(const Rect &searchArea, LeafNodeQueryFunc &&func) const
    {_queryRect(this->_rootNodePtr(), searchArea, std::forward<LeafNodeQueryFunc>(func));}

    void insert(const T &data)
    {
        NodeType *node = _queryLeafByPoint(this->_rootNodePtr(), m_icf(data));
        node->m_data.push_back(data); // Insert data into the leaf node
        _applyInsert(node); // Apply insert logic
    }
    void insert(T &&data)
    {
        NodeType *node = _queryLeafByPoint(this->_rootNodePtr(), m_icf(data));
        node->m_data.push_back(std::move(data)); // Insert data into the leaf node
        _applyInsert(node); // Apply insert logic
    }
private:
};
#endif // _TREE_HPP_