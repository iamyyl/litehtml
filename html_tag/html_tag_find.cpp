//
//  html_tag_find.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"


litehtml::element::ptr litehtml::html_tag::find_ancestor(const css_selector& selector, bool apply_pseudo, bool* is_pseudo)
{
    element::ptr el_parent = parent();
    if (!el_parent)
    {
        return nullptr;
    }
    int res = el_parent->select(selector, apply_pseudo);
    if(res != select_no_match)
    {
        if(is_pseudo)
        {
            if(res & select_match_pseudo_class)
            {
                *is_pseudo = true;
            } else
            {
                *is_pseudo = false;
            }
        }
        return el_parent;
    }
    return el_parent->find_ancestor(selector, apply_pseudo, is_pseudo);
}

int litehtml::html_tag::find_next_line_top( int top, int width, int def_right )
{
    if(is_floats_holder())
    {
        int new_top = top;
        int_vector points;

        for(const auto& fb : m_floats_left)
        {
            if(fb.pos.top() >= top)
            {
                if(find(points.begin(), points.end(), fb.pos.top()) == points.end())
                {
                    points.push_back(fb.pos.top());
                }
            }
            if (fb.pos.bottom() >= top)
            {
                if (find(points.begin(), points.end(), fb.pos.bottom()) == points.end())
                {
                    points.push_back(fb.pos.bottom());
                }
            }
        }

        for (const auto& fb : m_floats_right)
        {
            if (fb.pos.top() >= top)
            {
                if (find(points.begin(), points.end(), fb.pos.top()) == points.end())
                {
                    points.push_back(fb.pos.top());
                }
            }
            if (fb.pos.bottom() >= top)
            {
                if (find(points.begin(), points.end(), fb.pos.bottom()) == points.end())
                {
                    points.push_back(fb.pos.bottom());
                }
            }
        }

        if(!points.empty())
        {
            sort(points.begin(), points.end(), std::less<int>( ));
            new_top = points.back();

            for(auto pt : points)
            {
                int pos_left    = 0;
                int pos_right    = def_right;
                get_line_left_right(pt, def_right, pos_left, pos_right);

                if(pos_right - pos_left >= width)
                {
                    new_top = pt;
                    break;
                }
            }
        }
        return new_top;
    }
    element::ptr el_parent = parent();
    if (el_parent)
    {
        int new_top = el_parent->find_next_line_top(top + m_pos.y, width, def_right + m_pos.x);
        return new_top - m_pos.y;
    }
    return 0;
}

litehtml::element::ptr litehtml::html_tag::find_adjacent_sibling( const element::ptr& el, const css_selector& selector, bool apply_pseudo /*= true*/, bool* is_pseudo /*= 0*/ )
{
    element::ptr ret;
    for(auto& e : m_children)
    {
        if(e->get_display() != display_inline_text)
        {
            if(e == el)
            {
                if(ret)
                {
                    int res = ret->select(selector, apply_pseudo);
                    if(res != select_no_match)
                    {
                        if(is_pseudo)
                        {
                            if(res & select_match_pseudo_class)
                            {
                                *is_pseudo = true;
                            } else
                            {
                                *is_pseudo = false;
                            }
                        }
                        return ret;
                    }
                }
                return 0;
            } else
            {
                ret = e;
            }
        }
    }
    return 0;
}

litehtml::element::ptr litehtml::html_tag::find_sibling(const element::ptr& el, const css_selector& selector, bool apply_pseudo /*= true*/, bool* is_pseudo /*= 0*/)
{
    element::ptr ret = 0;
    for(auto& e : m_children)
    {
        if(e->get_display() != display_inline_text)
        {
            if(e == el)
            {
                return ret;
            } else if(!ret)
            {
                int res = e->select(selector, apply_pseudo);
                if(res != select_no_match)
                {
                    if(is_pseudo)
                    {
                        if(res & select_match_pseudo_class)
                        {
                            *is_pseudo = true;
                        } else
                        {
                            *is_pseudo = false;
                        }
                    }
                    ret = e;
                }
            }
        }
    }
    return 0;
}
