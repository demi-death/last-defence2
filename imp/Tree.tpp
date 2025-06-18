#ifndef _TREE_HPP_
#else

#ifndef _IMP_TREE_TPP_
#define _IMP_TREE_TPP_

namespace _qt
{
    template<typename T>
    void _Node<T>::_clearSubtree(void)
    {
        if(isLeaf())
            m_data.clear();
        else for(_Node *child : m_children)
        {
            child->_clearSubtree();
            delete child; // Delete the child node
        }
    }

    template<typename T>
    size_t _Node<T>::clearSubtree(void)
    {
        _clearSubtree();
        m_isLeaf = true; // Mark this as a leaf node
        for(_Node *&child : m_children)
            child = nullptr; // Set to nullptr to avoid dangling pointers
        size_t countTmp = m_count;
        m_count = 0;
        return countTmp;
    }

    template<typename T>
    void _Node<T>::divideNode(void)
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
template<typename LeafQuery>
void QuadTree<T>::queryRect(const Rect &searchArea, LeafQuery &&func)
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

template<typename T>
void QuadTree<T>::insert(const PointVector &pos, const T &data)
{
    NodeType *node = _queryLeafByPoint(&m_root, pos);
    node->m_data.push_back(LocatedData{pos, data}); // Insert data into the leaf node
    if(++(node->m_count) > m_countLimit)
        node->divideNode();
    _qt::_applyInsert(node); // Apply insert logic
}


#endif // _IMP_TREE_TPP_

#endif // _TREE_HPP_