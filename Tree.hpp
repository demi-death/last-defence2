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
        void _clearSubtree(void);
    public:
        void divideNode(void);
        size_t clearSubtree(void);
    };

    constexpr RectLooseness _completelyLoose({{true, true}, {true, true}});
}

template<typename T>
class EntityTree
{
private:
    typedef _qt::_Node<T> NodeType;
    NodeType m_root;
    size_t m_countLimit;
public:
    QuadTree(const Rect &boundingBox, size_t countLimit)
        : m_root(boundingBox, nullptr), m_countLimit(countLimit){}
    template<typename LeafQuery>
    void queryRect(const Rect &searchArea, LeafQuery &&func);
    void insert(const PointVector &pos, const T &data);
    void clear(void)
    {m_root.clearSubtree();}
    ~QuadTree()
    {clear();}
};
#include "imp/Tree.tpp"
#endif // _TREE_HPP_