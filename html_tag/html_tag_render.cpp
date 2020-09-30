//
//  html_tag_render.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

int litehtml::html_tag::render( int x, int y, int max_width, bool second_pass )
{
    if (m_display == display_table || m_display == display_inline_table)
    {
        return render_table(x, y, max_width, second_pass);
    }

    return render_box(x, y, max_width, second_pass);
}

int litehtml::html_tag::render_inline(const element::ptr &container, int max_width)
{
    int ret_width = 0;
    int rw = 0;

    white_space ws = get_white_space();
    bool skip_spaces = false;
    if (ws == white_space_normal ||
        ws == white_space_nowrap ||
        ws == white_space_pre_line)
    {
        skip_spaces = true;
    }
    bool was_space = false;

    for (auto& el : m_children)
    {
        // skip spaces to make rendering a bit faster
        if (skip_spaces)
        {
            if (el->is_white_space())
            {
                if (was_space)
                {
                    el->skip(true);
                    continue;
                }
                else
                {
                    was_space = true;
                }
            }
            else
            {
                was_space = false;
            }
        }

        rw = container->place_element( el, max_width );
        if(rw > ret_width)
        {
            ret_width = rw;
        }
    }
    return ret_width;
}

void litehtml::html_tag::render_positioned(render_type rt)
{
    position wnd_position;
    get_document()->container()->get_client_rect(wnd_position);

    element_position el_position;
    bool process;
    for (auto& el : m_positioned)
    {
        el_position = el->get_element_position();

        process = false;
        if(el->get_display() != display_none)
        {
            if(el_position == element_position_absolute)
            {
                if(rt != render_fixed_only)
                {
                    process = true;
                }
            } else if(el_position == element_position_fixed)
            {
                if(rt != render_no_fixed)
                {
                    process = true;
                }
            }
        }

        if(process)
        {
            int parent_height    = 0;
            int parent_width    = 0;
            int client_x        = 0;
            int client_y        = 0;
            if(el_position == element_position_fixed)
            {
                parent_height    = wnd_position.height;
                parent_width    = wnd_position.width;
                client_x        = wnd_position.left();
                client_y        = wnd_position.top();
            } else
            {
                element::ptr el_parent = el->parent();
                if(el_parent)
                {
                    parent_height    = el_parent->height();
                    parent_width    = el_parent->width();
                }
            }

            css_length    css_left    = el->get_css_left();
            css_length    css_right    = el->get_css_right();
            css_length    css_top        = el->get_css_top();
            css_length    css_bottom    = el->get_css_bottom();

            bool need_render = false;

            css_length el_w = el->get_css_width();
            css_length el_h = el->get_css_height();

            int new_width = -1;
            int new_height = -1;
            if(el_w.units() == css_units_percentage && parent_width)
            {
                new_width = el_w.calc_percent(parent_width);
                if(el->m_pos.width != new_width)
                {
                    need_render = true;
                    el->m_pos.width = new_width;
                }
            }

            if(el_h.units() == css_units_percentage && parent_height)
            {
                new_height = el_h.calc_percent(parent_height);
                if(el->m_pos.height != new_height)
                {
                    need_render = true;
                    el->m_pos.height = new_height;
                }
            }

            bool cvt_x = false;
            bool cvt_y = false;

            if(el_position == element_position_fixed)
            {
                if(!css_left.is_predefined() || !css_right.is_predefined())
                {
                    if(!css_left.is_predefined() && css_right.is_predefined())
                    {
                        el->m_pos.x = css_left.calc_percent(parent_width) + el->content_margins_left();
                    } else if(css_left.is_predefined() && !css_right.is_predefined())
                    {
                        el->m_pos.x = parent_width - css_right.calc_percent(parent_width) - el->m_pos.width - el->content_margins_right();
                    } else
                    {
                        el->m_pos.x        = css_left.calc_percent(parent_width) + el->content_margins_left();
                        el->m_pos.width    = parent_width - css_left.calc_percent(parent_width) - css_right.calc_percent(parent_width) - (el->content_margins_left() + el->content_margins_right());
                        need_render = true;
                    }
                }

                if(!css_top.is_predefined() || !css_bottom.is_predefined())
                {
                    if(!css_top.is_predefined() && css_bottom.is_predefined())
                    {
                        el->m_pos.y = css_top.calc_percent(parent_height) + el->content_margins_top();
                    } else if(css_top.is_predefined() && !css_bottom.is_predefined())
                    {
                        el->m_pos.y = parent_height - css_bottom.calc_percent(parent_height) - el->m_pos.height - el->content_margins_bottom();
                    } else
                    {
                        el->m_pos.y            = css_top.calc_percent(parent_height) + el->content_margins_top();
                        el->m_pos.height    = parent_height - css_top.calc_percent(parent_height) - css_bottom.calc_percent(parent_height) - (el->content_margins_top() + el->content_margins_bottom());
                        need_render = true;
                    }
                }
            } else
            {
                if(!css_left.is_predefined() || !css_right.is_predefined())
                {
                    if(!css_left.is_predefined() && css_right.is_predefined())
                    {
                        el->m_pos.x = css_left.calc_percent(parent_width) + el->content_margins_left() - m_padding.left;
                    } else if(css_left.is_predefined() && !css_right.is_predefined())
                    {
                        el->m_pos.x = m_pos.width + m_padding.right - css_right.calc_percent(parent_width) - el->m_pos.width - el->content_margins_right();
                    } else
                    {
                        el->m_pos.x        = css_left.calc_percent(parent_width) + el->content_margins_left() - m_padding.left;
                        el->m_pos.width    = m_pos.width + m_padding.left + m_padding.right - css_left.calc_percent(parent_width) - css_right.calc_percent(parent_width) - (el->content_margins_left() + el->content_margins_right());
                        if (new_width != -1)
                        {
                            el->m_pos.x += (el->m_pos.width - new_width) / 2;
                            el->m_pos.width = new_width;
                        }
                        need_render = true;
                    }
                    cvt_x = true;
                }

                if(!css_top.is_predefined() || !css_bottom.is_predefined())
                {
                    if(!css_top.is_predefined() && css_bottom.is_predefined())
                    {
                        el->m_pos.y = css_top.calc_percent(parent_height) + el->content_margins_top() - m_padding.top;
                    } else if(css_top.is_predefined() && !css_bottom.is_predefined())
                    {
                        el->m_pos.y = m_pos.height + m_padding.bottom - css_bottom.calc_percent(parent_height) - el->m_pos.height - el->content_margins_bottom();
                    } else
                    {
                        el->m_pos.y            = css_top.calc_percent(parent_height) + el->content_margins_top() - m_padding.top;
                        el->m_pos.height    = m_pos.height + m_padding.top + m_padding.bottom - css_top.calc_percent(parent_height) - css_bottom.calc_percent(parent_height) - (el->content_margins_top() + el->content_margins_bottom());
                        if (new_height != -1)
                        {
                            el->m_pos.y += (el->m_pos.height - new_height) / 2;
                            el->m_pos.height = new_height;
                        }
                        need_render = true;
                    }
                    cvt_y = true;
                }
            }

            if(cvt_x || cvt_y)
            {
                int offset_x = 0;
                int offset_y = 0;
                element::ptr cur_el = el->parent();
                element::ptr this_el = shared_from_this();
                while(cur_el && cur_el != this_el)
                {
                    offset_x += cur_el->m_pos.x;
                    offset_y += cur_el->m_pos.y;
                    cur_el = cur_el->parent();
                }
                if(cvt_x)    el->m_pos.x -= offset_x;
                if(cvt_y)    el->m_pos.y -= offset_y;
            }

            if(need_render)
            {
                position pos = el->m_pos;
                el->render(el->left(), el->top(), el->width(), true);
                el->m_pos = pos;
            }

            if(el_position == element_position_fixed)
            {
                position fixed_pos;
                el->get_redraw_box(fixed_pos);
                get_document()->add_fixed_box(fixed_pos);
            }
        }

        el->render_positioned();
    }

    if(!m_positioned.empty())
    {
        std::stable_sort(m_positioned.begin(), m_positioned.end(), [](const litehtml::element::ptr& _Left, const litehtml::element::ptr& _Right)
        {
            return (_Left->get_zindex() < _Right->get_zindex());
        });
    }
}

int litehtml::html_tag::render_box(int x, int y, int max_width, bool second_pass /*= false*/)
{
    int parent_width = max_width;

    calc_outlines(parent_width);

    m_pos.clear();
    m_pos.move_to(x, y);

    m_pos.x += content_margins_left();
    m_pos.y += content_margins_top();

    int ret_width = 0;

    def_value<int>    block_width(0);

    if (m_display != display_table_cell && !m_css_width.is_predefined())
    {
        int w = calc_width(parent_width);

        if (m_box_sizing == box_sizing_border_box)
        {
            w -= m_padding.width() + m_borders.width();
        }
        ret_width = max_width = block_width = w;
    }
    else
    {
        if (max_width)
        {
            max_width -= content_margins_left() + content_margins_right();
        }
    }

    // check for max-width (on the first pass only)
    if (!m_css_max_width.is_predefined() && !second_pass)
    {
        int mw = get_document()->cvt_units(m_css_max_width, m_font_size, parent_width);
        if (m_box_sizing == box_sizing_border_box)
        {
            mw -= m_padding.left + m_borders.left + m_padding.right + m_borders.right;
        }
        if (max_width > mw)
        {
            max_width = mw;
        }
    }

    m_floats_left.clear();
    m_floats_right.clear();
    m_boxes.clear();
    m_cahe_line_left.invalidate();
    m_cahe_line_right.invalidate();

    element_position el_position;

    int block_height = 0;

    m_pos.height = 0;

    if (get_predefined_height(block_height))
    {
        m_pos.height = block_height;
    }

    white_space ws = get_white_space();
    bool skip_spaces = false;
    if (ws == white_space_normal ||
        ws == white_space_nowrap ||
        ws == white_space_pre_line)
    {
        skip_spaces = true;
    }

    bool was_space = false;

    for (auto el : m_children)
    {
        // we don't need process absolute and fixed positioned element on the second pass
        if (second_pass)
        {
            el_position = el->get_element_position();
            if ((el_position == element_position_absolute || el_position == element_position_fixed)) continue;
        }

        // skip spaces to make rendering a bit faster
        if (skip_spaces)
        {
            if (el->is_white_space())
            {
                if (was_space)
                {
                    el->skip(true);
                    continue;
                }
                else
                {
                    was_space = true;
                }
            }
            else
            {
                was_space = false;
            }
        }

        // place element into rendering flow
        int rw = place_element(el, max_width);
        if (rw > ret_width)
        {
            ret_width = rw;
        }
    }

    finish_last_box(true);

    if (block_width.is_default() && is_inline_box())
    {
        m_pos.width = ret_width;
    }
    else
    {
        m_pos.width = max_width;
    }
    calc_auto_margins(parent_width);

    if (!m_boxes.empty())
    {
        if (collapse_top_margin())
        {
            int old_top = m_margins.top;
            m_margins.top = std::max(m_boxes.front()->top_margin(), m_margins.top);
            if (m_margins.top != old_top)
            {
                update_floats(m_margins.top - old_top, shared_from_this());
            }
        }
        if (collapse_bottom_margin())
        {
            m_margins.bottom = std::max(m_boxes.back()->bottom_margin(), m_margins.bottom);
            m_pos.height = m_boxes.back()->bottom() - m_boxes.back()->bottom_margin();
        }
        else
        {
            m_pos.height = m_boxes.back()->bottom();
        }
    }

    // add the floats height to the block height
    if (is_floats_holder())
    {
        int floats_height = get_floats_height();
        if (floats_height > m_pos.height)
        {
            m_pos.height = floats_height;
        }
    }

    // calculate the final position

    m_pos.move_to(x, y);
    m_pos.x += content_margins_left();
    m_pos.y += content_margins_top();

    if (get_predefined_height(block_height))
    {
        m_pos.height = block_height;
    }

    int min_height = 0;
    if (!m_css_min_height.is_predefined() && m_css_min_height.units() == css_units_percentage)
    {
        element::ptr el_parent = parent();
        if (el_parent)
        {
            if (el_parent->get_predefined_height(block_height))
            {
                min_height = m_css_min_height.calc_percent(block_height);
            }
        }
    }
    else
    {
        min_height = (int)m_css_min_height.val();
    }
    if (min_height != 0 && m_box_sizing == box_sizing_border_box)
    {
        min_height -= m_padding.top + m_borders.top + m_padding.bottom + m_borders.bottom;
        if (min_height < 0) min_height = 0;
    }

    if (m_display == display_list_item)
    {
        const tchar_t* list_image = get_style_property(_t("list-style-image"), true, 0);
        if (list_image)
        {
            tstring url;
            css::parse_css_url(list_image, url);

            size sz;
            const tchar_t* list_image_baseurl = get_style_property(_t("list-style-image-baseurl"), true, 0);
            get_document()->container()->get_image_size(url.c_str(), list_image_baseurl, sz);
            if (min_height < sz.height)
            {
                min_height = sz.height;
            }
        }

    }

    if (min_height > m_pos.height)
    {
        m_pos.height = min_height;
    }

    int min_width = m_css_min_width.calc_percent(parent_width);

    if (min_width != 0 && m_box_sizing == box_sizing_border_box)
    {
        min_width -= m_padding.left + m_borders.left + m_padding.right + m_borders.right;
        if (min_width < 0) min_width = 0;
    }

    if (min_width != 0)
    {
        if (min_width > m_pos.width)
        {
            m_pos.width = min_width;
        }
        if (min_width > ret_width)
        {
            ret_width = min_width;
        }
    }

    ret_width += content_margins_left() + content_margins_right();

    // re-render with new width
    if (ret_width < max_width && !second_pass && have_parent())
    {
        if (m_display == display_inline_block ||
            (m_css_width.is_predefined() &&
            (m_float != float_none ||
            m_display == display_table ||
            m_el_position == element_position_absolute ||
            m_el_position == element_position_fixed
            ))
            )
        {
            render(x, y, ret_width, true);
            m_pos.width = ret_width - (content_margins_left() + content_margins_right());
        }
    }

    if (is_floats_holder() && !second_pass)
    {
        for (const auto& fb : m_floats_left)
        {
            fb.el->apply_relative_shift(fb.el->parent()->calc_width(m_pos.width));
        }
    }


    return ret_width;
}

int litehtml::html_tag::render_table(int x, int y, int max_width, bool second_pass /*= false*/)
{
    if (!m_grid) return 0;

    int parent_width = max_width;

    calc_outlines(parent_width);

    m_pos.clear();
    m_pos.move_to(x, y);

    m_pos.x += content_margins_left();
    m_pos.y += content_margins_top();

    def_value<int>    block_width(0);

    if (!m_css_width.is_predefined())
    {
        max_width = block_width = calc_width(parent_width) - m_padding.width() - m_borders.width();
    }
    else
    {
        if (max_width)
        {
            max_width -= content_margins_left() + content_margins_right();
        }
    }

    // Calculate table spacing
    int table_width_spacing = 0;
    if (m_border_collapse == border_collapse_separate)
    {
        table_width_spacing = m_border_spacing_x * (m_grid->cols_count() + 1);
    }
    else
    {
        table_width_spacing = 0;

        if (m_grid->cols_count())
        {
            table_width_spacing -= std::min(border_left(), m_grid->column(0).border_left);
            table_width_spacing -= std::min(border_right(), m_grid->column(m_grid->cols_count() - 1).border_right);
        }

        for (int col = 1; col < m_grid->cols_count(); col++)
        {
            table_width_spacing -= std::min(m_grid->column(col).border_left, m_grid->column(col - 1).border_right);
        }
    }


    // Calculate the minimum content width (MCW) of each cell: the formatted content may span any number of lines but may not overflow the cell box.
    // If the specified 'width' (W) of the cell is greater than MCW, W is the minimum cell width. A value of 'auto' means that MCW is the minimum
    // cell width.
    //
    // Also, calculate the "maximum" cell width of each cell: formatting the content without breaking lines other than where explicit line breaks occur.

    if (m_grid->cols_count() == 1 && !block_width.is_default())
    {
        for (int row = 0; row < m_grid->rows_count(); row++)
        {
            table_cell* cell = m_grid->cell(0, row);
            if (cell && cell->el)
            {
                cell->min_width = cell->max_width = cell->el->render(0, 0, max_width - table_width_spacing);
                cell->el->m_pos.width = cell->min_width - cell->el->content_margins_left() - cell->el->content_margins_right();
            }
        }
    }
    else
    {
        for (int row = 0; row < m_grid->rows_count(); row++)
        {
            for (int col = 0; col < m_grid->cols_count(); col++)
            {
                table_cell* cell = m_grid->cell(col, row);
                if (cell && cell->el)
                {
                    if (!m_grid->column(col).css_width.is_predefined() && m_grid->column(col).css_width.units() != css_units_percentage)
                    {
                        int css_w = m_grid->column(col).css_width.calc_percent(block_width);
                        int el_w = cell->el->render(0, 0, css_w);
                        cell->min_width = cell->max_width = std::max(css_w, el_w);
                        cell->el->m_pos.width = cell->min_width - cell->el->content_margins_left() - cell->el->content_margins_right();
                    }
                    else
                    {
                        // calculate minimum content width
                        cell->min_width = cell->el->render(0, 0, 1);
                        // calculate maximum content width
                        cell->max_width = cell->el->render(0, 0, max_width - table_width_spacing);
                    }
                }
            }
        }
    }

    // For each column, determine a maximum and minimum column width from the cells that span only that column.
    // The minimum is that required by the cell with the largest minimum cell width (or the column 'width', whichever is larger).
    // The maximum is that required by the cell with the largest maximum cell width (or the column 'width', whichever is larger).

    for (int col = 0; col < m_grid->cols_count(); col++)
    {
        m_grid->column(col).max_width = 0;
        m_grid->column(col).min_width = 0;
        for (int row = 0; row < m_grid->rows_count(); row++)
        {
            if (m_grid->cell(col, row)->colspan <= 1)
            {
                m_grid->column(col).max_width = std::max(m_grid->column(col).max_width, m_grid->cell(col, row)->max_width);
                m_grid->column(col).min_width = std::max(m_grid->column(col).min_width, m_grid->cell(col, row)->min_width);
            }
        }
    }

    // For each cell that spans more than one column, increase the minimum widths of the columns it spans so that together,
    // they are at least as wide as the cell. Do the same for the maximum widths.
    // If possible, widen all spanned columns by approximately the same amount.

    for (int col = 0; col < m_grid->cols_count(); col++)
    {
        for (int row = 0; row < m_grid->rows_count(); row++)
        {
            if (m_grid->cell(col, row)->colspan > 1)
            {
                int max_total_width = m_grid->column(col).max_width;
                int min_total_width = m_grid->column(col).min_width;
                for (int col2 = col + 1; col2 < col + m_grid->cell(col, row)->colspan; col2++)
                {
                    max_total_width += m_grid->column(col2).max_width;
                    min_total_width += m_grid->column(col2).min_width;
                }
                if (min_total_width < m_grid->cell(col, row)->min_width)
                {
                    m_grid->distribute_min_width(m_grid->cell(col, row)->min_width - min_total_width, col, col + m_grid->cell(col, row)->colspan - 1);
                }
                if (max_total_width < m_grid->cell(col, row)->max_width)
                {
                    m_grid->distribute_max_width(m_grid->cell(col, row)->max_width - max_total_width, col, col + m_grid->cell(col, row)->colspan - 1);
                }
            }
        }
    }

    // If the 'table' or 'inline-table' element's 'width' property has a computed value (W) other than 'auto', the used width is the
    // greater of W, CAPMIN, and the minimum width required by all the columns plus cell spacing or borders (MIN).
    // If the used width is greater than MIN, the extra width should be distributed over the columns.
    //
    // If the 'table' or 'inline-table' element has 'width: auto', the used width is the greater of the table's containing block width,
    // CAPMIN, and MIN. However, if either CAPMIN or the maximum width required by the columns plus cell spacing or borders (MAX) is
    // less than that of the containing block, use max(MAX, CAPMIN).


    int table_width = 0;
    int min_table_width = 0;
    int max_table_width = 0;

    if (!block_width.is_default())
    {
        table_width = m_grid->calc_table_width(block_width - table_width_spacing, false, min_table_width, max_table_width);
    }
    else
    {
        table_width = m_grid->calc_table_width(max_width - table_width_spacing, true, min_table_width, max_table_width);
    }

    min_table_width += table_width_spacing;
    max_table_width += table_width_spacing;
    table_width += table_width_spacing;
    m_grid->calc_horizontal_positions(m_borders, m_border_collapse, m_border_spacing_x);

    bool row_span_found = false;

    // render cells with computed width
    for (int row = 0; row < m_grid->rows_count(); row++)
    {
        m_grid->row(row).height = 0;
        for (int col = 0; col < m_grid->cols_count(); col++)
        {
            table_cell* cell = m_grid->cell(col, row);
            if (cell->el)
            {
                int span_col = col + cell->colspan - 1;
                if (span_col >= m_grid->cols_count())
                {
                    span_col = m_grid->cols_count() - 1;
                }
                int cell_width = m_grid->column(span_col).right - m_grid->column(col).left;

                if (cell->el->m_pos.width != cell_width - cell->el->content_margins_left() - cell->el->content_margins_right())
                {
                    cell->el->render(m_grid->column(col).left, 0, cell_width);
                    cell->el->m_pos.width = cell_width - cell->el->content_margins_left() - cell->el->content_margins_right();
                }
                else
                {
                    cell->el->m_pos.x = m_grid->column(col).left + cell->el->content_margins_left();
                }

                if (cell->rowspan <= 1)
                {
                    m_grid->row(row).height = std::max(m_grid->row(row).height, cell->el->height());
                }
                else
                {
                    row_span_found = true;
                }

            }
        }
    }

    if (row_span_found)
    {
        for (int col = 0; col < m_grid->cols_count(); col++)
        {
            for (int row = 0; row < m_grid->rows_count(); row++)
            {
                table_cell* cell = m_grid->cell(col, row);
                if (cell->el)
                {
                    int span_row = row + cell->rowspan - 1;
                    if (span_row >= m_grid->rows_count())
                    {
                        span_row = m_grid->rows_count() - 1;
                    }
                    if (span_row != row)
                    {
                        int h = 0;
                        for (int i = row; i <= span_row; i++)
                        {
                            h += m_grid->row(i).height;
                        }
                        if (h < cell->el->height())
                        {
                            m_grid->row(span_row).height += cell->el->height() - h;
                        }
                    }
                }
            }
        }
    }

    // Calculate vertical table spacing
    int table_height_spacing = 0;
    if (m_border_collapse == border_collapse_separate)
    {
        table_height_spacing = m_border_spacing_y * (m_grid->rows_count() + 1);
    }
    else
    {
        table_height_spacing = 0;

        if (m_grid->rows_count())
        {
            table_height_spacing -= std::min(border_top(), m_grid->row(0).border_top);
            table_height_spacing -= std::min(border_bottom(), m_grid->row(m_grid->rows_count() - 1).border_bottom);
        }

        for (int row = 1; row < m_grid->rows_count(); row++)
        {
            table_height_spacing -= std::min(m_grid->row(row).border_top, m_grid->row(row - 1).border_bottom);
        }
    }


    // calculate block height
    int block_height = 0;
    if (get_predefined_height(block_height))
    {
        block_height -= m_padding.height() + m_borders.height();
    }

    // calculate minimum height from m_css_min_height
    int min_height = 0;
    if (!m_css_min_height.is_predefined() && m_css_min_height.units() == css_units_percentage)
    {
        element::ptr el_parent = parent();
        if (el_parent)
        {
            int parent_height = 0;
            if (el_parent->get_predefined_height(parent_height))
            {
                min_height = m_css_min_height.calc_percent(parent_height);
            }
        }
    }
    else
    {
        min_height = (int)m_css_min_height.val();
    }

    int extra_row_height = 0;
    int minimum_table_height = std::max(block_height, min_height);

    m_grid->calc_rows_height(minimum_table_height - table_height_spacing, m_border_spacing_y);
    m_grid->calc_vertical_positions(m_borders, m_border_collapse, m_border_spacing_y);

    int table_height = 0;

    // place cells vertically
    for (int col = 0; col < m_grid->cols_count(); col++)
    {
        for (int row = 0; row < m_grid->rows_count(); row++)
        {
            table_cell* cell = m_grid->cell(col, row);
            if (cell->el)
            {
                int span_row = row + cell->rowspan - 1;
                if (span_row >= m_grid->rows_count())
                {
                    span_row = m_grid->rows_count() - 1;
                }
                cell->el->m_pos.y = m_grid->row(row).top + cell->el->content_margins_top();
                cell->el->m_pos.height = m_grid->row(span_row).bottom - m_grid->row(row).top - cell->el->content_margins_top() - cell->el->content_margins_bottom();
                table_height = std::max(table_height, m_grid->row(span_row).bottom);
                cell->el->apply_vertical_align();
            }
        }
    }

    if (m_border_collapse == border_collapse_collapse)
    {
        if (m_grid->rows_count())
        {
            table_height -= std::min(border_bottom(), m_grid->row(m_grid->rows_count() - 1).border_bottom);
        }
    }
    else
    {
        table_height += m_border_spacing_y;
    }

    m_pos.width = table_width;

    calc_auto_margins(parent_width);

    m_pos.move_to(x, y);
    m_pos.x += content_margins_left();
    m_pos.y += content_margins_top();
    m_pos.width = table_width;
    m_pos.height = table_height;

    return max_table_width;
}
