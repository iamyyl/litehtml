//
//  html_tag_child.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"

bool litehtml::html_tag::appendChild(const element::ptr &el)
{
    if(el)
    {
        el->parent(shared_from_this());
        m_children.push_back(el);
        return true;
    }
    return false;
}

bool litehtml::html_tag::removeChild(const element::ptr &el)
{
    if(el && el->parent() == shared_from_this())
    {
        el->parent(nullptr);
        m_children.erase(std::remove(m_children.begin(), m_children.end(), el), m_children.end());
        return true;
    }
    return false;
}

void litehtml::html_tag::clearRecursive()
{
    for(auto& el : m_children)
    {
        el->clearRecursive();
        el->parent(nullptr);
    }
    m_children.clear();
}

bool litehtml::html_tag::is_first_child_inline(const element::ptr& el) const
{
    if(!m_children.empty())
    {
        for (const auto& this_el : m_children)
        {
            if (!this_el->is_white_space())
            {
                if (el == this_el)
                {
                    return true;
                }
                if (this_el->get_display() == display_inline)
                {
                    if (this_el->have_inline_child())
                    {
                        return false;
                    }
                } else
                {
                    return false;
                }
            }
        }
    }
    return false;
}

bool litehtml::html_tag::is_last_child_inline(const element::ptr& el)
{
    if(!m_children.empty())
    {
        for (auto this_el = m_children.rbegin(); this_el < m_children.rend(); ++this_el)
        {
            if (!(*this_el)->is_white_space())
            {
                if (el == (*this_el))
                {
                    return true;
                }
                if ((*this_el)->get_display() == display_inline)
                {
                    if ((*this_el)->have_inline_child())
                    {
                        return false;
                    }
                } else
                {
                    return false;
                }
            }
        }
    }
    return false;
}

size_t litehtml::html_tag::get_children_count() const
{
    return m_children.size();
}

litehtml::element::ptr litehtml::html_tag::get_child( int idx ) const
{
    return m_children[idx];
}

bool litehtml::html_tag::is_nth_child(const element::ptr& el, int num, int off, bool of_type) const
{
    int idx = 1;
    for(const auto& child : m_children)
    {
        if(child->get_display() != display_inline_text)
        {
            if( (!of_type) || (of_type && !t_strcmp(el->get_tagName(), child->get_tagName())) )
            {
                if(el == child)
                {
                    if(num != 0)
                    {
                        if((idx - off) >= 0 && (idx - off) % num == 0)
                        {
                            return true;
                        }

                    } else if(idx == off)
                    {
                        return true;
                    }
                    return false;
                }
                idx++;
            }
            if(el == child) break;
        }
    }
    return false;
}

bool litehtml::html_tag::is_nth_last_child(const element::ptr& el, int num, int off, bool of_type) const
{
    int idx = 1;
    for(elements_vector::const_reverse_iterator child = m_children.rbegin(); child != m_children.rend(); child++)
    {
        if((*child)->get_display() != display_inline_text)
        {
            if( !of_type || (of_type && !t_strcmp(el->get_tagName(), (*child)->get_tagName())) )
            {
                if(el == (*child))
                {
                    if(num != 0)
                    {
                        if((idx - off) >= 0 && (idx - off) % num == 0)
                        {
                            return true;
                        }

                    } else if(idx == off)
                    {
                        return true;
                    }
                    return false;
                }
                idx++;
            }
            if(el == (*child)) break;
        }
    }
    return false;
}

bool litehtml::html_tag::is_only_child(const element::ptr& el, bool of_type) const
{
    int child_count = 0;
    for(const auto& child : m_children)
    {
        if(child->get_display() != display_inline_text)
        {
            if( !of_type || (of_type && !t_strcmp(el->get_tagName(), child->get_tagName())) )
            {
                child_count++;
            }
            if(child_count > 1) break;
        }
    }
    if(child_count > 1)
    {
        return false;
    }
    return true;
}

void litehtml::html_tag::remove_before_after()
{
    if(!m_children.empty())
    {
        if( !t_strcmp(m_children.front()->get_tagName(), _t("::before")) )
        {
            m_children.erase(m_children.begin());
        }
    }
    if(!m_children.empty())
    {
        if( !t_strcmp(m_children.back()->get_tagName(), _t("::after")) )
        {
            m_children.erase(m_children.end() - 1);
        }
    }
}

bool litehtml::html_tag::have_inline_child() const
{
    if(!m_children.empty())
    {
        for(const auto& el : m_children)
        {
            if(!el->is_white_space())
            {
                return true;
            }
        }
    }
    return false;
}
