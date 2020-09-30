//
//  html_tag_line.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"

int litehtml::html_tag::get_base_line()
{
    if(is_replaced())
    {
        return 0;
    }
    int bl = 0;
    if(!m_boxes.empty())
    {
        bl = m_boxes.back()->baseline() + content_margins_bottom();
    }
    return bl;
}

int litehtml::html_tag::get_line_left( int y )
{
    if(is_floats_holder())
    {
        if(m_cahe_line_left.is_valid && m_cahe_line_left.hash == y)
        {
            return m_cahe_line_left.val;
        }

        int w = 0;
        for(const auto& fb : m_floats_left)
        {
            if (y >= fb.pos.top() && y < fb.pos.bottom())
            {
                w = std::max(w, fb.pos.right());
                if (w < fb.pos.right())
                {
                    break;
                }
            }
        }
        m_cahe_line_left.set_value(y, w);
        return w;
    }
    element::ptr el_parent = parent();
    if (el_parent)
    {
        int w = el_parent->get_line_left(y + m_pos.y);
        if (w < 0)
        {
            w = 0;
        }
        return w - (w ? m_pos.x : 0);
    }
    return 0;
}

int litehtml::html_tag::get_line_right( int y, int def_right )
{
    if(is_floats_holder())
    {
        if(m_cahe_line_right.is_valid && m_cahe_line_right.hash == y)
        {
            if(m_cahe_line_right.is_default)
            {
                return def_right;
            } else
            {
                return std::min(m_cahe_line_right.val, def_right);
            }
        }

        int w = def_right;
        m_cahe_line_right.is_default = true;
        for(const auto& fb : m_floats_right)
        {
            if(y >= fb.pos.top() && y < fb.pos.bottom())
            {
                w = std::min(w, fb.pos.left());
                m_cahe_line_right.is_default = false;
                if(w > fb.pos.left())
                {
                    break;
                }
            }
        }
        m_cahe_line_right.set_value(y, w);
        return w;
    }
    element::ptr el_parent = parent();
    if (el_parent)
    {
        int w = el_parent->get_line_right(y + m_pos.y, def_right + m_pos.x);
        return w - m_pos.x;
    }
    return 0;
}


void litehtml::html_tag::get_line_left_right( int y, int def_right, int& ln_left, int& ln_right )
{
    if(is_floats_holder())
    {
        ln_left        = get_line_left(y);
        ln_right    = get_line_right(y, def_right);
    } else
    {
        element::ptr el_parent = parent();
        if (el_parent)
        {
            el_parent->get_line_left_right(y + m_pos.y, def_right + m_pos.x, ln_left, ln_right);
        }
        ln_right -= m_pos.x;
        ln_left -= m_pos.x;

        if(ln_left < 0)
        {
            ln_left = 0;
        }
    }
}

int litehtml::html_tag::fix_line_width( int max_width, element_float flt )
{
    int ret_width = 0;
    if(!m_boxes.empty())
    {
        elements_vector els;
        m_boxes.back()->get_elements(els);
        bool was_cleared = false;
        if(!els.empty() && els.front()->get_clear() != clear_none)
        {
            if(els.front()->get_clear() == clear_both)
            {
                was_cleared = true;
            } else
            {
                if(    (flt == float_left    && els.front()->get_clear() == clear_left) ||
                    (flt == float_right    && els.front()->get_clear() == clear_right) )
                {
                    was_cleared = true;
                }
            }
        }

        if(!was_cleared)
        {
            m_boxes.pop_back();

            for(elements_vector::iterator i = els.begin(); i != els.end(); i++)
            {
                int rw = place_element((*i), max_width);
                if(rw > ret_width)
                {
                    ret_width = rw;
                }
            }
        } else
        {
            int line_top = 0;
            if(m_boxes.back()->get_type() == box_line)
            {
                line_top = m_boxes.back()->top();
            } else
            {
                line_top = m_boxes.back()->bottom();
            }

            int line_left    = 0;
            int line_right    = max_width;
            get_line_left_right(line_top, max_width, line_left, line_right);

            if(m_boxes.back()->get_type() == box_line)
            {
                if(m_boxes.size() == 1 && m_list_style_type != list_style_type_none && m_list_style_position == list_style_position_inside)
                {
                    int sz_font = get_font_size();
                    line_left += sz_font;
                }

                if(m_css_text_indent.val() != 0)
                {
                    bool line_box_found = false;
                    for(box::vector::iterator iter = m_boxes.begin(); iter < m_boxes.end(); iter++)
                    {
                        if((*iter)->get_type() == box_line)
                        {
                            line_box_found = true;
                            break;
                        }
                    }
                    if(!line_box_found)
                    {
                        line_left += m_css_text_indent.calc_percent(max_width);
                    }
                }

            }

            elements_vector els;
            m_boxes.back()->new_width(line_left, line_right, els);
            for(auto& el : els)
            {
                int rw = place_element(el, max_width);
                if(rw > ret_width)
                {
                    ret_width = rw;
                }
            }
        }
    }

    return ret_width;
}

int litehtml::html_tag::line_height() const
{
    return m_line_height;
}
