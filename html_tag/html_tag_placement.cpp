//
//  html_tag_placement.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"

int litehtml::html_tag::place_element(const element::ptr &el, int max_width)
{
    if(el->get_display() == display_none) return 0;

    if(el->get_display() == display_inline)
    {
        return el->render_inline(shared_from_this(), max_width);
    }

    element_position el_position = el->get_element_position();

    if(el_position == element_position_absolute || el_position == element_position_fixed)
    {
        int line_top = 0;
        if(!m_boxes.empty())
        {
            if(m_boxes.back()->get_type() == box_line)
            {
                line_top = m_boxes.back()->top();
                if(!m_boxes.back()->is_empty())
                {
                    line_top += line_height();
                }
            } else
            {
                line_top = m_boxes.back()->bottom();
            }
        }

        el->render(0, line_top, max_width);
        el->m_pos.x    += el->content_margins_left();
        el->m_pos.y    += el->content_margins_top();

        return 0;
    }

    int ret_width = 0;

    switch(el->get_float())
    {
    case float_left:
        {
            int line_top = 0;
            if(!m_boxes.empty())
            {
                if(m_boxes.back()->get_type() == box_line)
                {
                    line_top = m_boxes.back()->top();
                } else
                {
                    line_top = m_boxes.back()->bottom();
                }
            }
            line_top        = get_cleared_top(el, line_top);
            int line_left    = 0;
            int line_right    = max_width;
            get_line_left_right(line_top, max_width, line_left, line_right);

            el->render(line_left, line_top, line_right);
            if(el->right() > line_right)
            {
                int new_top = find_next_line_top(el->top(), el->width(), max_width);
                el->m_pos.x = get_line_left(new_top) + el->content_margins_left();
                el->m_pos.y = new_top + el->content_margins_top();
            }
            add_float(el, 0, 0);
            ret_width = fix_line_width(max_width, float_left);
            if(!ret_width)
            {
                ret_width = el->right();
            }
        }
        break;
    case float_right:
        {
            int line_top = 0;
            if(!m_boxes.empty())
            {
                if(m_boxes.back()->get_type() == box_line)
                {
                    line_top = m_boxes.back()->top();
                } else
                {
                    line_top = m_boxes.back()->bottom();
                }
            }
            line_top        = get_cleared_top(el, line_top);
            int line_left    = 0;
            int line_right    = max_width;
            get_line_left_right(line_top, max_width, line_left, line_right);

            el->render(0, line_top, line_right);

            if(line_left + el->width() > line_right)
            {
                int new_top = find_next_line_top(el->top(), el->width(), max_width);
                el->m_pos.x = get_line_right(new_top, max_width) - el->width() + el->content_margins_left();
                el->m_pos.y = new_top + el->content_margins_top();
            } else
            {
                el->m_pos.x = line_right - el->width() + el->content_margins_left();
            }
            add_float(el, 0, 0);
            ret_width = fix_line_width(max_width, float_right);

            if(!ret_width)
            {
                line_left    = 0;
                line_right    = max_width;
                get_line_left_right(line_top, max_width, line_left, line_right);

                ret_width = ret_width + (max_width - line_right);
            }
        }
        break;
    default:
        {
            line_context line_ctx;
            line_ctx.top = 0;
            if (!m_boxes.empty())
            {
                line_ctx.top = m_boxes.back()->top();
            }
            line_ctx.left = 0;
            line_ctx.right = max_width;
            line_ctx.fix_top();
            get_line_left_right(line_ctx.top, max_width, line_ctx.left, line_ctx.right);

            switch(el->get_display())
            {
            case display_inline_block:
            case display_inline_table:
                ret_width = el->render(line_ctx.left, line_ctx.top, line_ctx.right);
                break;
            case display_block:
                if(el->is_replaced() || el->is_floats_holder())
                {
                    element::ptr el_parent = el->parent();
                    el->m_pos.width = el->get_css_width().calc_percent(line_ctx.right - line_ctx.left);
                    el->m_pos.height = el->get_css_height().calc_percent(el_parent ? el_parent->m_pos.height : 0);
                }
                el->calc_outlines(line_ctx.right - line_ctx.left);
                break;
            case display_inline_text:
                {
                    litehtml::size sz;
                    el->get_content_size(sz, line_ctx.right);
                    el->m_pos = sz;
                }
                break;
            default:
                ret_width = 0;
                break;
            }

            bool add_box = true;
            if(!m_boxes.empty())
            {
                if(m_boxes.back()->can_hold(el, m_white_space))
                {
                    add_box = false;
                }
            }
            if(add_box)
            {
                new_box(el, max_width, line_ctx);
            } else if(!m_boxes.empty())
            {
                line_ctx.top = m_boxes.back()->top();
            }

            if (line_ctx.top != line_ctx.calculatedTop)
            {
                line_ctx.left = 0;
                line_ctx.right = max_width;
                line_ctx.fix_top();
                get_line_left_right(line_ctx.top, max_width, line_ctx.left, line_ctx.right);
            }

            if(!el->is_inline_box())
            {
                if(m_boxes.size() == 1)
                {
                    if(collapse_top_margin())
                    {
                        int shift = el->margin_top();
                        if(shift >= 0)
                        {
                            line_ctx.top -= shift;
                            m_boxes.back()->y_shift(-shift);
                        }
                    }
                } else
                {
                    int shift = 0;
                    int prev_margin = m_boxes[m_boxes.size() - 2]->bottom_margin();

                    if(prev_margin > el->margin_top())
                    {
                        shift = el->margin_top();
                    } else
                    {
                        shift = prev_margin;
                    }
                    if(shift >= 0)
                    {
                        line_ctx.top -= shift;
                        m_boxes.back()->y_shift(-shift);
                    }
                }
            }

            switch(el->get_display())
            {
            case display_table:
            case display_list_item:
                ret_width = el->render(line_ctx.left, line_ctx.top, line_ctx.width());
                break;
            case display_block:
            case display_table_cell:
            case display_table_caption:
            case display_table_row:
                if(el->is_replaced() || el->is_floats_holder())
                {
                    ret_width = el->render(line_ctx.left, line_ctx.top, line_ctx.width()) + line_ctx.left + (max_width - line_ctx.right);
                } else
                {
                    ret_width = el->render(0, line_ctx.top, max_width);
                }
                break;
            default:
                ret_width = 0;
                break;
            }

            m_boxes.back()->add_element(el);

            if(el->is_inline_box() && !el->skip())
            {
                ret_width = el->right() + (max_width - line_ctx.right);
            }
        }
        break;
    }

    return ret_width;
}
