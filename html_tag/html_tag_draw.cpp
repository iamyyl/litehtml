//
//  html_tag_draw.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

void litehtml::html_tag::draw( uint_ptr hdc, int x, int y, const position* clip )
{
    position pos = m_pos;
    pos.x    += x;
    pos.y    += y;

    draw_background(hdc, x, y, clip);

    if(m_display == display_list_item && m_list_style_type != list_style_type_none)
    {
        if(m_overflow > overflow_visible)
        {
            position border_box = pos;
            border_box += m_padding;
            border_box += m_borders;

            border_radiuses bdr_radius = m_css_borders.radius.calc_percents(border_box.width, border_box.height);

            bdr_radius -= m_borders;
            bdr_radius -= m_padding;

            get_document()->container()->set_clip(pos, bdr_radius, true, true);
        }

        draw_list_marker(hdc, pos);

        if(m_overflow > overflow_visible)
        {
            get_document()->container()->del_clip();
        }
    }
}

void litehtml::html_tag::draw_background( uint_ptr hdc, int x, int y, const position* clip )
{
    position pos = m_pos;
    pos.x    += x;
    pos.y    += y;

    position el_pos = pos;
    el_pos += m_padding;
    el_pos += m_borders;

    if(m_display != display_inline && m_display != display_table_row)
    {
        if(el_pos.does_intersect(clip))
        {
            const background* bg = get_background();
            if(bg)
            {
                background_paint bg_paint;
                init_background_paint(pos, bg_paint, bg);

                get_document()->container()->draw_background(hdc, bg_paint);
            }
            position border_box = pos;
            border_box += m_padding;
            border_box += m_borders;

            borders bdr = m_css_borders;
            bdr.radius = m_css_borders.radius.calc_percents(border_box.width, border_box.height);

            get_document()->container()->draw_borders(hdc, bdr, border_box, have_parent() ? false : true);
        }
    } else
    {
        const background* bg = get_background();

        position::vector boxes;
        get_inline_boxes(boxes);

        background_paint bg_paint;
        position content_box;

        for(position::vector::iterator box = boxes.begin(); box != boxes.end(); box++)
        {
            box->x    += x;
            box->y    += y;

            if(box->does_intersect(clip))
            {
                content_box = *box;
                content_box -= m_borders;
                content_box -= m_padding;

                if(bg)
                {
                    init_background_paint(content_box, bg_paint, bg);
                }

                css_borders bdr;

                // set left borders radius for the first box
                if(box == boxes.begin())
                {
                    bdr.radius.bottom_left_x    = m_css_borders.radius.bottom_left_x;
                    bdr.radius.bottom_left_y    = m_css_borders.radius.bottom_left_y;
                    bdr.radius.top_left_x        = m_css_borders.radius.top_left_x;
                    bdr.radius.top_left_y        = m_css_borders.radius.top_left_y;
                }

                // set right borders radius for the last box
                if(box == boxes.end() - 1)
                {
                    bdr.radius.bottom_right_x    = m_css_borders.radius.bottom_right_x;
                    bdr.radius.bottom_right_y    = m_css_borders.radius.bottom_right_y;
                    bdr.radius.top_right_x        = m_css_borders.radius.top_right_x;
                    bdr.radius.top_right_y        = m_css_borders.radius.top_right_y;
                }


                bdr.top        = m_css_borders.top;
                bdr.bottom    = m_css_borders.bottom;
                if(box == boxes.begin())
                {
                    bdr.left    = m_css_borders.left;
                }
                if(box == boxes.end() - 1)
                {
                    bdr.right    = m_css_borders.right;
                }


                if(bg)
                {
                    bg_paint.border_radius = bdr.radius.calc_percents(bg_paint.border_box.width, bg_paint.border_box.width);
                    get_document()->container()->draw_background(hdc, bg_paint);
                }
                borders b = bdr;
                b.radius = bdr.radius.calc_percents(box->width, box->height);
                get_document()->container()->draw_borders(hdc, b, *box, false);
            }
        }
    }
}

void litehtml::html_tag::draw_list_marker( uint_ptr hdc, const position &pos )
{
    list_marker lm;

    const tchar_t* list_image = get_style_property(_t("list-style-image"), true, 0);
    size img_size;
    if(list_image)
    {
        css::parse_css_url(list_image, lm.image);
        lm.baseurl = get_style_property(_t("list-style-image-baseurl"), true, 0);
        get_document()->container()->get_image_size(lm.image.c_str(), lm.baseurl, img_size);
    } else
    {
        lm.baseurl = 0;
    }

    int ln_height    = line_height();
    int sz_font        = get_font_size();
    lm.pos.x        = pos.x;
    lm.pos.width = sz_font - sz_font * 2 / 3;
    lm.color = get_color(_t("color"), true, web_color(0, 0, 0));
    lm.marker_type = m_list_style_type;
    lm.font = get_font();

    if (m_list_style_type >= list_style_type_armenian)
    {
        lm.pos.y = pos.y;
        lm.pos.height = pos.height;
        lm.index = get_attr(_t("list_index"), _t(""))[0];
    }
    else
    {
        lm.pos.height = sz_font - sz_font * 2 / 3;
        lm.pos.y = pos.y + ln_height / 2 - lm.pos.height / 2;
        lm.index = -1;
    }

    if(img_size.width && img_size.height)
    {
        if(lm.pos.y + img_size.height > pos.y + pos.height)
        {
            lm.pos.y = pos.y + pos.height - img_size.height;
        }
        if(img_size.width > lm.pos.width)
        {
            lm.pos.x -= img_size.width - lm.pos.width;
        }

        lm.pos.width    = img_size.width;
        lm.pos.height    = img_size.height;
    }

    if (m_list_style_position == list_style_position_outside)
    {
        if (m_list_style_type >= list_style_type_armenian)
        {
            auto tw_space = get_document()->container()->text_width(_t(" "), lm.font);
            lm.pos.x = pos.x - tw_space * 2;
            lm.pos.width = tw_space;
        }
        else
        {
            lm.pos.x -= sz_font;
        }
    }

    if (m_list_style_type >= list_style_type_armenian)
    {
        auto marker_text = get_list_marker_text(lm.index);
        lm.pos.height = ln_height;
        if (marker_text.empty())
        {
            get_document()->container()->draw_list_marker(hdc, lm);
        }
        else
        {
            marker_text += _t(".");
            auto tw = get_document()->container()->text_width(marker_text.c_str(), lm.font);
            auto text_pos = lm.pos;
            text_pos.move_to(text_pos.right() - tw, text_pos.y);
            get_document()->container()->draw_text(hdc, marker_text.c_str(), lm.font, lm.color, text_pos);
        }
    }
    else
    {
        get_document()->container()->draw_list_marker(hdc, lm);
    }
}

void litehtml::html_tag::draw_children( uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex )
{
    if (m_display == display_table || m_display == display_inline_table)
    {
        draw_children_table(hdc, x, y, clip, flag, zindex);
    }
    else
    {
        draw_children_box(hdc, x, y, clip, flag, zindex);
    }
}

void litehtml::html_tag::draw_stacking_context( uint_ptr hdc, int x, int y, const position* clip, bool with_positioned )
{
    if(!is_visible()) return;

    std::map<int, bool> zindexes;
    if(with_positioned)
    {
        for(elements_vector::iterator i = m_positioned.begin(); i != m_positioned.end(); i++)
        {
            zindexes[(*i)->get_zindex()];
        }

        for(std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end(); idx++)
        {
            if(idx->first < 0)
            {
                draw_children(hdc, x, y, clip, draw_positioned, idx->first);
            }
        }
    }
    draw_children(hdc, x, y, clip, draw_block, 0);
    draw_children(hdc, x, y, clip, draw_floats, 0);
    draw_children(hdc, x, y, clip, draw_inlines, 0);
    if(with_positioned)
    {
        for(std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end(); idx++)
        {
            if(idx->first == 0)
            {
                draw_children(hdc, x, y, clip, draw_positioned, idx->first);
            }
        }

        for(std::map<int, bool>::iterator idx = zindexes.begin(); idx != zindexes.end(); idx++)
        {
            if(idx->first > 0)
            {
                draw_children(hdc, x, y, clip, draw_positioned, idx->first);
            }
        }
    }
}

void litehtml::html_tag::draw_children_box(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex)
{
    position pos = m_pos;
    pos.x += x;
    pos.y += y;

    document::ptr doc = get_document();

    if (m_overflow > overflow_visible)
    {
        position border_box = pos;
        border_box += m_padding;
        border_box += m_borders;

        border_radiuses bdr_radius = m_css_borders.radius.calc_percents(border_box.width, border_box.height);

        bdr_radius -= m_borders;
        bdr_radius -= m_padding;

        doc->container()->set_clip(pos, bdr_radius, true, true);
    }

    position browser_wnd;
    doc->container()->get_client_rect(browser_wnd);

    element::ptr el;
    for (auto& item : m_children)
    {
        el = item;
        if (el->is_visible())
        {
            switch (flag)
            {
            case draw_positioned:
                if (el->is_positioned() && el->get_zindex() == zindex)
                {
                    if (el->get_element_position() == element_position_fixed)
                    {
                        el->draw(hdc, browser_wnd.x, browser_wnd.y, clip);
                        el->draw_stacking_context(hdc, browser_wnd.x, browser_wnd.y, clip, true);
                    }
                    else
                    {
                        el->draw(hdc, pos.x, pos.y, clip);
                        el->draw_stacking_context(hdc, pos.x, pos.y, clip, true);
                    }
                    el = 0;
                }
                break;
            case draw_block:
                if (!el->is_inline_box() && el->get_float() == float_none && !el->is_positioned())
                {
                    el->draw(hdc, pos.x, pos.y, clip);
                }
                break;
            case draw_floats:
                if (el->get_float() != float_none && !el->is_positioned())
                {
                    el->draw(hdc, pos.x, pos.y, clip);
                    el->draw_stacking_context(hdc, pos.x, pos.y, clip, false);
                    el = 0;
                }
                break;
            case draw_inlines:
                if (el->is_inline_box() && el->get_float() == float_none && !el->is_positioned())
                {
                    el->draw(hdc, pos.x, pos.y, clip);
                    if (el->get_display() == display_inline_block)
                    {
                        el->draw_stacking_context(hdc, pos.x, pos.y, clip, false);
                        el = 0;
                    }
                }
                break;
            default:
                break;
            }

            if (el)
            {
                if (flag == draw_positioned)
                {
                    if (!el->is_positioned())
                    {
                        el->draw_children(hdc, pos.x, pos.y, clip, flag, zindex);
                    }
                }
                else
                {
                    if (el->get_float() == float_none &&
                        el->get_display() != display_inline_block &&
                        !el->is_positioned())
                    {
                        el->draw_children(hdc, pos.x, pos.y, clip, flag, zindex);
                    }
                }
            }
        }
    }

    if (m_overflow > overflow_visible)
    {
        doc->container()->del_clip();
    }
}

void litehtml::html_tag::draw_children_table(uint_ptr hdc, int x, int y, const position* clip, draw_flag flag, int zindex)
{
    if (!m_grid) return;

    position pos = m_pos;
    pos.x += x;
    pos.y += y;
    for (int row = 0; row < m_grid->rows_count(); row++)
    {
        if (flag == draw_block)
        {
            m_grid->row(row).el_row->draw_background(hdc, pos.x, pos.y, clip);
        }
        for (int col = 0; col < m_grid->cols_count(); col++)
        {
            table_cell* cell = m_grid->cell(col, row);
            if (cell->el)
            {
                if (flag == draw_block)
                {
                    cell->el->draw(hdc, pos.x, pos.y, clip);
                }
                cell->el->draw_children(hdc, pos.x, pos.y, clip, flag, zindex);
            }
        }
    }
}
