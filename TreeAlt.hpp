// QuadTreeNode.h
#ifndef _TREE_ALT_HPP_
#define _TREE_ALT_HPP_
#include <vector>
#include <list>
#include <memory>
#include <algorithm>
#include "Geo.hpp" // Uses your Rect and RectLooseness

class PositionedObject
{
private:
    PointVector m_pos;
    bool m_valid;
public:
    PositionedObject()
        : m_pos(), m_valid(true){}
    PositionedObject(const PointVector &pos)
        : m_pos(pos), m_valid(true){}
    PositionedObject(const PositionedObject &rhs)
        : m_pos(rhs.m_pos), m_valid(true){}
    constexpr const PointVector &position(void) const
    {return m_pos;}
    void setPosition(const PointVector &pos)
    {m_pos = pos;}
    constexpr bool valid(void) const
    {return m_valid;}
    void deleteObject(void)
    {m_valid = false;}
    virtual ~PositionedObject(){}
};
typedef std::shared_ptr<PositionedObject> PositionedObjectPtr;

class QuadTreeNode
{
private:
    Rect m_region;
    std::list<PositionedObjectPtr> m_objects;
    std::unique_ptr<QuadTreeNode> m_children[4]{}; // 4 quadrants
    size_t m_capacity;
    bool m_divided;

public:
    QuadTreeNode(const Rect &region, size_t capacity = 8)
        : m_region(region), m_capacity(capacity), m_divided(false) {}

    bool insert(PositionedObjectPtr obj)
    {
        if (!m_region.contains(obj->position())) return false;

        if (m_objects.size() < m_capacity && !m_divided)
        {
            m_objects.push_back(obj);
            return true;
        }

        if (!m_divided)
            subdivide();

        for (auto &child : m_children)
        {
            if (child->insert(obj)) return true;
        }

        return false; // Should not reach here if regions are well defined
    }

    bool remove(PositionedObjectPtr obj)
    {
        auto it = std::find(m_objects.begin(), m_objects.end(), obj);
        if (it != m_objects.end())
        {
            m_objects.erase(it);
            if (m_divided)
                tryMerge();
            return true;
        }
    
        if (m_divided)
        {
            for (auto &child : m_children)
            {
                if (child && child->remove(obj))
                {
                    tryMerge();
                    return true;
                }
            }
        }
    
        return false;
    }
    
    void tryMerge()
    {
        if (!m_divided) return;
    
        bool canMerge = true;
        for (const auto &child : m_children)
        {
            if (!child || child->m_divided || !child->m_objects.empty())
            {
                canMerge = false;
                break;
            }
        }
    
        if (canMerge)
        {
            for (auto &child : m_children)
                child.reset();
            m_divided = false;
        }
    }

    template <typename Func>
    void query(const Rect &area, Func &&callback)
    {
        if (!m_region.intersects(area)) return;

        for (auto it = m_objects.begin(); it != m_objects.end(); )
        {
            const auto &obj = *it;
            if (area.contains(obj->position()))
            {
                callback(obj);

                if (!obj->valid())
                {
                    it = m_objects.erase(it);
                    continue;
                }

                if (!m_region.contains(obj->position()))
                {
                    auto toMove = obj;
                    it = m_objects.erase(it);
                    insert(toMove); // try to reinsert; may fail upward
                    continue;
                }
            }
            ++it;
        }

        if (m_divided)
        {
            for (const auto &child : m_children)
                child->query(area, callback);
        }
    }

    void subdivide()
    {
        PointVector c = m_region.center();
        const PointVector &min = m_region.minPoint();
        const PointVector &max = m_region.maxPoint();

        const auto &loose = m_region.looseness();

        // Define each child quadrant
        m_children[0] = std::make_unique<QuadTreeNode>(Rect(min, c, Rect::Looseness({
            {loose.looseMin(0), false},
            {loose.looseMin(1), false}})), m_capacity);

        m_children[1] = std::make_unique<QuadTreeNode>(Rect({c[0], min[1]}, {max[0], c[1]}, Rect::Looseness({
            {false, loose.looseMax(0)},
            {loose.looseMin(1), false}})), m_capacity);

        m_children[2] = std::make_unique<QuadTreeNode>(Rect({min[0], c[1]}, {c[0], max[1]}, Rect::Looseness({
            {loose.looseMin(0), false},
            {false, loose.looseMax(1)}})), m_capacity);

        m_children[3] = std::make_unique<QuadTreeNode>(Rect(c, max, Rect::Looseness({
            {false, loose.looseMax(0)},
            {false, loose.looseMax(1)}})), m_capacity);

        m_divided = true;
    }

    void clear()
    {
        m_objects.clear();
        for (auto &child : m_children)
        {
            if (child) child->clear();
            child.reset();
        }
        m_divided = false;
    }
};
#endif