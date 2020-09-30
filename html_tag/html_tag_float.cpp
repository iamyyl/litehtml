//
//  html_tag_float.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"


int litehtml::html_tag::get_floats_height(element_float el_float) const
{
    if(is_floats_holder())
    {
        int h = 0;

        bool process = false;

        for(const auto& fb : m_floats_left)
        {
            process = false;
            switch(el_float)
            {
            case float_none:
                process = true;
                break;
            case float_left:
                if (fb.clear_floats == clear_left || fb.clear_floats == clear_both)
                {
                    process = true;
                }
                break;
            case float_right:
                if (fb.clear_floats == clear_right || fb.clear_floats == clear_both)
                {
                    process = true;
                }
                break;
            }
            if(process)
            {
                if(el_float == float_none)
                {
                    h = std::max(h, fb.pos.bottom());
                } else
                {
                    h = std::max(h, fb.pos.top());
                }
            }
        }


        for(const auto fb : m_floats_right)
        {
            process = false;
            switch(el_float)
            {
            case float_none:
                process = true;
                break;
            case float_left:
                if (fb.clear_floats == clear_left || fb.clear_floats == clear_both)
                {
                    process = true;
                }
                break;
            case float_right:
                if (fb.clear_floats == clear_right || fb.clear_floats == clear_both)
                {
                    process = true;
                }
                break;
            }
            if(process)
            {
                if(el_float == float_none)
                {
                    h = std::max(h, fb.pos.bottom());
                } else
                {
                    h = std::max(h, fb.pos.top());
                }
            }
        }

        return h;
    }
    element::ptr el_parent = parent();
    if (el_parent)
    {
        int h = el_parent->get_floats_height(el_float);
        return h - m_pos.y;
    }
    return 0;
}

int litehtml::html_tag::get_left_floats_height() const
{
    if(is_floats_holder())
    {
        int h = 0;
        if(!m_floats_left.empty())
        {
            for (const auto& fb : m_floats_left)
            {
                h = std::max(h, fb.pos.bottom());
            }
        }
        return h;
    }
    element::ptr el_parent = parent();
    if (el_parent)
    {
        int h = el_parent->get_left_floats_height();
        return h - m_pos.y;
    }
    return 0;
}

int litehtml::html_tag::get_right_floats_height() const
{
    if(is_floats_holder())
    {
        int h = 0;
        if(!m_floats_right.empty())
        {
            for(const auto& fb : m_floats_right)
            {
                h = std::max(h, fb.pos.bottom());
            }
        }
        return h;
    }
    element::ptr el_parent = parent();
    if (el_parent)
    {
        int h = el_parent->get_right_floats_height();
        return h - m_pos.y;
    }
    return 0;
}

litehtml::element_float litehtml::html_tag::get_float() const
{
    return m_float;
}

bool litehtml::html_tag::is_floats_holder() const
{
    if(    m_display == display_inline_block ||
        m_display == display_table_cell ||
        !have_parent() ||
        is_body() ||
        m_float != float_none ||
        m_el_position == element_position_absolute ||
        m_el_position == element_position_fixed ||
        m_overflow > overflow_visible)
    {
        return true;
    }
    return false;
}

void litehtml::html_tag::add_float(const element::ptr &el, int x, int y)
{
    if(is_floats_holder())
    {
        floated_box fb;
        fb.pos.x        = el->left() + x;
        fb.pos.y        = el->top()  + y;
        fb.pos.width    = el->width();
        fb.pos.height    = el->height();
        fb.float_side    = el->get_float();
        fb.clear_floats    = el->get_clear();
        fb.el            = el;

        if(fb.float_side == float_left)
        {
            if(m_floats_left.empty())
            {
                m_floats_left.push_back(fb);
            } else
            {
                bool inserted = false;
                for(floated_box::vector::iterator i = m_floats_left.begin(); i != m_floats_left.end(); i++)
                {
                    if(fb.pos.right() > i->pos.right())
                    {
                        m_floats_left.insert(i, std::move(fb));
                        inserted = true;
                        break;
                    }
                }
                if(!inserted)
                {
                    m_floats_left.push_back(std::move(fb));
                }
            }
            m_cahe_line_left.invalidate();
        } else if(fb.float_side == float_right)
        {
            if(m_floats_right.empty())
            {
                m_floats_right.push_back(std::move(fb));
            } else
            {
                bool inserted = false;
                for(floated_box::vector::iterator i = m_floats_right.begin(); i != m_floats_right.end(); i++)
                {
                    if(fb.pos.left() < i->pos.left())
                    {
                        m_floats_right.insert(i, std::move(fb));
                        inserted = true;
                        break;
                    }
                }
                if(!inserted)
                {
                    m_floats_right.push_back(fb);
                }
            }
            m_cahe_line_right.invalidate();
        }
    } else
    {
        element::ptr el_parent = parent();
        if (el_parent)
        {
            el_parent->add_float(el, x + m_pos.x, y + m_pos.y);
        }
    }
}

void litehtml::html_tag::update_floats(int dy, const element::ptr &parent)
{
    if(is_floats_holder())
    {
        bool reset_cache = false;
        for(floated_box::vector::reverse_iterator fb = m_floats_left.rbegin(); fb != m_floats_left.rend(); fb++)
        {
            if(fb->el->is_ancestor(parent))
            {
                reset_cache    = true;
                fb->pos.y    += dy;
            }
        }
        if(reset_cache)
        {
            m_cahe_line_left.invalidate();
        }
        reset_cache = false;
        for(floated_box::vector::reverse_iterator fb = m_floats_right.rbegin(); fb != m_floats_right.rend(); fb++)
        {
            if(fb->el->is_ancestor(parent))
            {
                reset_cache    = true;
                fb->pos.y    += dy;
            }
        }
        if(reset_cache)
        {
            m_cahe_line_right.invalidate();
        }
    } else
    {
        element::ptr el_parent = this->parent();
        if (el_parent)
        {
            el_parent->update_floats(dy, parent);
        }
    }
}

int litehtml::html_tag::get_cleared_top(const element::ptr &el, int line_top) const
{
    switch(el->get_clear())
    {
    case clear_left:
        {
            int fh = get_left_floats_height();
            if(fh && fh > line_top)
            {
                line_top = fh;
            }
        }
        break;
    case clear_right:
        {
            int fh = get_right_floats_height();
            if(fh && fh > line_top)
            {
                line_top = fh;
            }
        }
        break;
    case clear_both:
        {
            int fh = get_floats_height();
            if(fh && fh > line_top)
            {
                line_top = fh;
            }
        }
        break;
    default:
        if(el->get_float() != float_none)
        {
            int fh = get_floats_height(el->get_float());
            if(fh && fh > line_top)
            {
                line_top = fh;
            }
        }
        break;
    }
    return line_top;
}
