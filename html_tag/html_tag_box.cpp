//
//  html_tag_box.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

void litehtml::html_tag::get_inline_boxes( position::vector& boxes )
{
    // collect all box of child elements in this tag
    // pos in box is relative of parent (this tag)
    
    litehtml::box* old_box = 0;
    position pos; // current pos
    for(auto& el : m_children)
    {
        if(el->skip())
        {
            continue;
        }
            if(el->m_box)
            {
                if(el->m_box != old_box)
                {
                    if(old_box)
                    {
                        if(boxes.empty())
                        {
                            // first box and set left
                            pos.x        -= m_padding.left + m_borders.left;
                            pos.width    += m_padding.left + m_borders.left;
                        }
                        boxes.push_back(pos);
                    }
                    old_box        = el->m_box;
                    pos.x        = el->left() + el->margin_left();
                    pos.y        = el->top() - m_padding.top - m_borders.top;
                    pos.width    = 0;
                    pos.height    = 0;
                }
                pos.width    = el->right() - pos.x - el->margin_right() - el->margin_left();
                pos.height    = std::max(pos.height, el->height() + m_padding.top + m_padding.bottom + m_borders.top + m_borders.bottom);
            } else if(el->get_display() == display_inline)
            {
                position::vector sub_boxes;
                el->get_inline_boxes(sub_boxes);
                if(!sub_boxes.empty())
                {
                    sub_boxes.rbegin()->width += el->margin_right();
                    if(boxes.empty())
                    {
                        if(m_padding.left + m_borders.left > 0)
                        {
                            position padding_box = (*sub_boxes.begin());
                            padding_box.x        -= m_padding.left + m_borders.left + el->margin_left();
                            padding_box.width    = m_padding.left + m_borders.left + el->margin_left();
                            boxes.push_back(padding_box);
                        }
                    }

                    sub_boxes.rbegin()->width += el->margin_right();

                    boxes.insert(boxes.end(), sub_boxes.begin(), sub_boxes.end());
                }
            }
    } // for end
    
    if(pos.width || pos.height)
    {
        if(boxes.empty())
        {
            pos.x        -= m_padding.left + m_borders.left;
            pos.width    += m_padding.left + m_borders.left;
        }
        boxes.push_back(pos);
    }
    if(!boxes.empty())
    {
        if(m_padding.right + m_borders.right > 0)
        {
            boxes.back().width += m_padding.right + m_borders.right;
        }
    }
}


void litehtml::html_tag::init_background_paint(position pos, background_paint &bg_paint, const background* bg)
{
    if(!bg) return;

    bg_paint = *bg;
    position content_box    = pos;
    position padding_box    = pos;
    padding_box += m_padding;
    position border_box        = padding_box;
    border_box += m_borders;

    switch(bg->m_clip)
    {
    case litehtml::background_box_padding:
        bg_paint.clip_box = padding_box;
        break;
    case litehtml::background_box_content:
        bg_paint.clip_box = content_box;
        break;
    default:
        bg_paint.clip_box = border_box;
        break;
    }

    switch(bg->m_origin)
    {
    case litehtml::background_box_border:
        bg_paint.origin_box = border_box;
        break;
    case litehtml::background_box_content:
        bg_paint.origin_box = content_box;
        break;
    default:
        bg_paint.origin_box = padding_box;
        break;
    }

    if(!bg_paint.image.empty())
    {
        get_document()->container()->get_image_size(bg_paint.image.c_str(), bg_paint.baseurl.c_str(), bg_paint.image_size);
        if(bg_paint.image_size.width && bg_paint.image_size.height)
        {
            litehtml::size img_new_sz = bg_paint.image_size;
            double img_ar_width        = (double) bg_paint.image_size.width / (double) bg_paint.image_size.height;
            double img_ar_height    = (double) bg_paint.image_size.height / (double) bg_paint.image_size.width;


            if(bg->m_position.width.is_predefined())
            {
                switch(bg->m_position.width.predef())
                {
                case litehtml::background_size_contain:
                    if( (int) ((double) bg_paint.origin_box.width * img_ar_height) <= bg_paint.origin_box.height )
                    {
                        img_new_sz.width = bg_paint.origin_box.width;
                        img_new_sz.height    = (int) ((double) bg_paint.origin_box.width * img_ar_height);
                    } else
                    {
                        img_new_sz.height = bg_paint.origin_box.height;
                        img_new_sz.width    = (int) ((double) bg_paint.origin_box.height * img_ar_width);
                    }
                    break;
                case litehtml::background_size_cover:
                    if( (int) ((double) bg_paint.origin_box.width * img_ar_height) >= bg_paint.origin_box.height )
                    {
                        img_new_sz.width = bg_paint.origin_box.width;
                        img_new_sz.height    = (int) ((double) bg_paint.origin_box.width * img_ar_height);
                    } else
                    {
                        img_new_sz.height = bg_paint.origin_box.height;
                        img_new_sz.width    = (int) ((double) bg_paint.origin_box.height * img_ar_width);
                    }
                    break;
                    break;
                case litehtml::background_size_auto:
                    if(!bg->m_position.height.is_predefined())
                    {
                        img_new_sz.height    = bg->m_position.height.calc_percent(bg_paint.origin_box.height);
                        img_new_sz.width    = (int) ((double) img_new_sz.height * img_ar_width);
                    }
                    break;
                }
            } else
            {
                img_new_sz.width = bg->m_position.width.calc_percent(bg_paint.origin_box.width);
                if(bg->m_position.height.is_predefined())
                {
                    img_new_sz.height = (int) ((double) img_new_sz.width * img_ar_height);
                } else
                {
                    img_new_sz.height = bg->m_position.height.calc_percent(bg_paint.origin_box.height);
                }
            }

            bg_paint.image_size = img_new_sz;
            bg_paint.position_x = bg_paint.origin_box.x + (int) bg->m_position.x.calc_percent(bg_paint.origin_box.width - bg_paint.image_size.width);
            bg_paint.position_y = bg_paint.origin_box.y + (int) bg->m_position.y.calc_percent(bg_paint.origin_box.height - bg_paint.image_size.height);
        }

    }
    bg_paint.border_radius    = m_css_borders.radius.calc_percents(border_box.width, border_box.height);;
    bg_paint.border_box        = border_box;
    bg_paint.is_root        = have_parent() ? false : true;
}

int litehtml::html_tag::finish_last_box(bool end_of_render)
{
    int line_top = 0;

    if(!m_boxes.empty())
    {
        m_boxes.back()->finish(end_of_render);

        if(m_boxes.back()->is_empty())
        {
            line_top = m_boxes.back()->top();
            m_boxes.pop_back();
        }

        if(!m_boxes.empty())
        {
            line_top = m_boxes.back()->bottom();
        }
    }
    return line_top;
}

int litehtml::html_tag::new_box(const element::ptr &el, int max_width, line_context& line_ctx)
{
    line_ctx.top = get_cleared_top(el, finish_last_box());

    line_ctx.left = 0;
    line_ctx.right = max_width;
    line_ctx.fix_top();
    get_line_left_right(line_ctx.top, max_width, line_ctx.left, line_ctx.right);

    if(el->is_inline_box() || el->is_floats_holder())
    {
        if (el->width() > line_ctx.right - line_ctx.left)
        {
            line_ctx.top = find_next_line_top(line_ctx.top, el->width(), max_width);
            line_ctx.left = 0;
            line_ctx.right = max_width;
            line_ctx.fix_top();
            get_line_left_right(line_ctx.top, max_width, line_ctx.left, line_ctx.right);
        }
    }

    int first_line_margin = 0;
    if(m_boxes.empty() && m_list_style_type != list_style_type_none && m_list_style_position == list_style_position_inside)
    {
        int sz_font = get_font_size();
        first_line_margin = sz_font;
    }

    if(el->is_inline_box())
    {
        int text_indent = 0;
        if(m_css_text_indent.val() != 0)
        {
            bool line_box_found = false;
            for(box::vector::iterator iter = m_boxes.begin(); iter != m_boxes.end(); iter++)
            {
                if((*iter)->get_type() == box_line)
                {
                    line_box_found = true;
                    break;
                }
            }
            if(!line_box_found)
            {
                text_indent = m_css_text_indent.calc_percent(max_width);
            }
        }

        font_metrics fm;
        get_font(&fm);
        m_boxes.emplace_back(std::unique_ptr<line_box>(new line_box(line_ctx.top, line_ctx.left + first_line_margin + text_indent, line_ctx.right, line_height(), fm, m_text_align)));
    } else
    {
        m_boxes.emplace_back(std::unique_ptr<block_box>(new block_box(line_ctx.top, line_ctx.left, line_ctx.right)));
    }

    return line_ctx.top;
}

void litehtml::html_tag::apply_vertical_align()
{
    if(!m_boxes.empty())
    {
        int add = 0;
        int content_height    = m_boxes.back()->bottom();

        if(m_pos.height > content_height)
        {
            switch(m_vertical_align)
            {
            case va_middle:
                add = (m_pos.height - content_height) / 2;
                break;
            case va_bottom:
                add = m_pos.height - content_height;
                break;
            default:
                add = 0;
                break;
            }
        }

        if(add)
        {
            for(size_t i = 0; i < m_boxes.size(); i++)
            {
                m_boxes[i]->y_shift(add);
            }
        }
    }
}


void litehtml::html_tag::get_redraw_box(litehtml::position& pos, int x /*= 0*/, int y /*= 0*/)
{
    if(is_visible())
    {
        element::get_redraw_box(pos, x, y);

        if(m_overflow == overflow_visible)
        {
            for(auto& el : m_children)
            {
                if(el->get_element_position() != element_position_fixed)
                {
                    el->get_redraw_box(pos, x + m_pos.x, y + m_pos.y);
                }
            }
        }
    }
}
