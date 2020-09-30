//
//  html_tag_get_element.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "el_before_after.h"

litehtml::element::ptr litehtml::html_tag::get_child_by_point(int x, int y, int client_x, int client_y, draw_flag flag, int zindex)
{
    element::ptr ret = 0;

    if(m_overflow > overflow_visible)
    {
        if(!m_pos.is_point_inside(x, y))
        {
            return ret;
        }
    }

    position pos = m_pos;
    pos.x    = x - pos.x;
    pos.y    = y - pos.y;

    for(elements_vector::reverse_iterator i = m_children.rbegin(); i != m_children.rend() && !ret; i++)
    {
        element::ptr el = (*i);

        if(el->is_visible() && el->get_display() != display_inline_text)
        {
            switch(flag)
            {
            case draw_positioned:
                if(el->is_positioned() && el->get_zindex() == zindex)
                {
                    if(el->get_element_position() == element_position_fixed)
                    {
                        ret = el->get_element_by_point(client_x, client_y, client_x, client_y);
                        if(!ret && (*i)->is_point_inside(client_x, client_y))
                        {
                            ret = (*i);
                        }
                    } else
                    {
                        ret = el->get_element_by_point(pos.x, pos.y, client_x, client_y);
                        if(!ret && (*i)->is_point_inside(pos.x, pos.y))
                        {
                            ret = (*i);
                        }
                    }
                    el = 0;
                }
                break;
            case draw_block:
                if(!el->is_inline_box() && el->get_float() == float_none && !el->is_positioned())
                {
                    if(el->is_point_inside(pos.x, pos.y))
                    {
                        ret = el;
                    }
                }
                break;
            case draw_floats:
                if(el->get_float() != float_none && !el->is_positioned())
                {
                    ret = el->get_element_by_point(pos.x, pos.y, client_x, client_y);

                    if(!ret && (*i)->is_point_inside(pos.x, pos.y))
                    {
                        ret = (*i);
                    }
                    el = 0;
                }
                break;
            case draw_inlines:
                if(el->is_inline_box() && el->get_float() == float_none && !el->is_positioned())
                {
                    if(el->get_display() == display_inline_block)
                    {
                        ret = el->get_element_by_point(pos.x, pos.y, client_x, client_y);
                        el = 0;
                    }
                    if(!ret && (*i)->is_point_inside(pos.x, pos.y))
                    {
                        ret = (*i);
                    }
                }
                break;
            default:
                break;
            }

            if(el && !el->is_positioned())
            {
                if(flag == draw_positioned)
                {
                    element::ptr child = el->get_child_by_point(pos.x, pos.y, client_x, client_y, flag, zindex);
                    if(child)
                    {
                        ret = child;
                    }
                } else
                {
                    if(    el->get_float() == float_none &&
                        el->get_display() != display_inline_block)
                    {
                        element::ptr child = el->get_child_by_point(pos.x, pos.y, client_x, client_y, flag, zindex);
                        if(child)
                        {
                            ret = child;
                        }
                    }
                }
            }
        }
    }

    return ret;
}

litehtml::element::ptr litehtml::html_tag::get_element_by_point(int x, int y, int client_x, int client_y)
{
    if(!is_visible()) return 0;

    element::ptr ret;

    std::map<int, bool> zindexes;

    for(elements_vector::iterator i = m_positioned.begin(); i != m_positioned.end(); i++)
    {
        zindexes[(*i)->get_zindex()];
    }

    for(std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end() && !ret; idx++)
    {
        if(idx->first > 0)
        {
            ret = get_child_by_point(x, y, client_x, client_y, draw_positioned, idx->first);
        }
    }
    if(ret) return ret;

    for(std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end() && !ret; idx++)
    {
        if(idx->first == 0)
        {
            ret = get_child_by_point(x, y, client_x, client_y, draw_positioned, idx->first);
        }
    }
    if(ret) return ret;

    ret = get_child_by_point(x, y, client_x, client_y, draw_inlines, 0);
    if(ret) return ret;

    ret = get_child_by_point(x, y, client_x, client_y, draw_floats, 0);
    if(ret) return ret;

    ret = get_child_by_point(x, y, client_x, client_y, draw_block, 0);
    if(ret) return ret;


    for(std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end() && !ret; idx++)
    {
        if(idx->first < 0)
        {
            ret = get_child_by_point(x, y, client_x, client_y, draw_positioned, idx->first);
        }
    }
    if(ret) return ret;

    if(m_el_position == element_position_fixed)
    {
        if(is_point_inside(client_x, client_y))
        {
            ret = shared_from_this();
        }
    } else
    {
        if(is_point_inside(x, y))
        {
            ret = shared_from_this();
        }
    }

    return ret;
}

litehtml::element::ptr litehtml::html_tag::get_element_before()
{
    if(!m_children.empty())
    {
        if( !t_strcmp(m_children.front()->get_tagName(), _t("::before")) )
        {
            return m_children.front();
        }
    }
    element::ptr el = std::make_shared<el_before>(get_document());
    el->parent(shared_from_this());
    m_children.insert(m_children.begin(), el);
    return el;
}

litehtml::element::ptr litehtml::html_tag::get_element_after()
{
    if(!m_children.empty())
    {
        if( !t_strcmp(m_children.back()->get_tagName(), _t("::after")) )
        {
            return m_children.back();
        }
    }
    element::ptr el = std::make_shared<el_after>(get_document());
    appendChild(el);
    return el;
}
