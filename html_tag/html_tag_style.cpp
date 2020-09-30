//
//  html_tag_style.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

void litehtml::html_tag::apply_stylesheet( const litehtml::css& stylesheet )
{
    remove_before_after();

    for(const auto& sel : stylesheet.selectors())
    {
        int apply = select(*sel, false);

        if(apply != select_no_match)
        {
            used_selector::ptr us = std::unique_ptr<used_selector>(new used_selector(sel, false));

            if(sel->is_media_valid())
            {
                if(apply & select_match_pseudo_class)
                {
                    if(!select(*sel, true))
                    {
                        return;
                    }
                    else if(apply & select_match_with_after)
                    {
                        element::ptr el = get_element_after();
                        if(el) el->add_style(*sel->m_style);
                    }
                    else if(apply & select_match_with_before)
                    {
                        element::ptr el = get_element_before();
                        if(el) el->add_style(*sel->m_style);
                    }
                    else
                    {
                        add_style(*sel->m_style);
                        us->m_used = true;
                    }
                } else if(apply & select_match_with_after)
                {
                    element::ptr el = get_element_after();
                    if(el) el->add_style(*sel->m_style);
                } else if(apply & select_match_with_before)
                {
                    element::ptr el = get_element_before();
                    if(el) el->add_style(*sel->m_style);
                } else
                {
                    add_style(*sel->m_style);
                    us->m_used = true;
                }
            }
            m_used_styles.push_back(std::move(us));
        }
    }

    for(auto& el : m_children)
    {
        if(el->get_display() != display_inline_text)
        {
            el->apply_stylesheet(stylesheet);
        }
    }
}

const litehtml::tchar_t* litehtml::html_tag::get_style_property( const tchar_t* name, bool inherited, const tchar_t* def /*= 0*/ )
{
    const tchar_t* ret = m_style.get_property(name);
    element::ptr el_parent = parent();
    if (el_parent)
    {
        if ( ( ret && !t_strcasecmp(ret, _t("inherit")) ) || (!ret && inherited) )
        {
            ret = el_parent->get_style_property(name, inherited, def);
        }
    }

    if(!ret)
    {
        ret = def;
    }

    return ret;
}

void litehtml::html_tag::parse_styles(bool is_reparse)
{
    const tchar_t* style = get_attr(_t("style"));

    if(style)
    {
        m_style.add(style, NULL);
    }

    init_font();
    document::ptr doc = get_document();

    m_el_position    = (element_position)    value_index(get_style_property(_t("position"),        false,    _t("static")),        element_position_strings,    element_position_fixed);
    m_text_align    = (text_align)            value_index(get_style_property(_t("text-align"),        true,    _t("left")),        text_align_strings,            text_align_left);
    m_overflow        = (overflow)            value_index(get_style_property(_t("overflow"),        false,    _t("visible")),        overflow_strings,            overflow_visible);
    m_white_space    = (white_space)            value_index(get_style_property(_t("white-space"),    true,    _t("normal")),        white_space_strings,        white_space_normal);
    m_display        = (style_display)        value_index(get_style_property(_t("display"),        false,    _t("inline")),        style_display_strings,        display_inline);
    m_visibility    = (visibility)            value_index(get_style_property(_t("visibility"),    true,    _t("visible")),        visibility_strings,            visibility_visible);
    m_box_sizing    = (box_sizing)            value_index(get_style_property(_t("box-sizing"),        false,    _t("content-box")),    box_sizing_strings,            box_sizing_content_box);

    if(m_el_position != element_position_static)
    {
        const tchar_t* val = get_style_property(_t("z-index"), false, 0);
        if(val)
        {
            m_z_index = t_atoi(val);
        }
    }

    const tchar_t* va    = get_style_property(_t("vertical-align"), true,    _t("baseline"));
    m_vertical_align = (vertical_align) value_index(va, vertical_align_strings, va_baseline);

    const tchar_t* fl    = get_style_property(_t("float"), false,    _t("none"));
    m_float = (element_float) value_index(fl, element_float_strings, float_none);

    m_clear = (element_clear) value_index(get_style_property(_t("clear"), false, _t("none")), element_clear_strings, clear_none);

    if (m_float != float_none)
    {
        // reset display in to block for floating elements
        if (m_display != display_none)
        {
            m_display = display_block;
        }
    }
    else if (m_display == display_table ||
        m_display == display_inline_table ||
        m_display == display_table_caption ||
        m_display == display_table_cell ||
        m_display == display_table_column ||
        m_display == display_table_column_group ||
        m_display == display_table_footer_group ||
        m_display == display_table_header_group ||
        m_display == display_table_row ||
        m_display == display_table_row_group)
    {
        doc->add_tabular(shared_from_this());
    }
    // fix inline boxes with absolute/fixed positions
    else if (m_display != display_none && is_inline_box())
    {
        if (m_el_position == element_position_absolute || m_el_position == element_position_fixed)
        {
            m_display = display_block;
        }
    }

    m_css_text_indent.fromString(    get_style_property(_t("text-indent"),    true,    _t("0")),    _t("0"));

    m_css_width.fromString(            get_style_property(_t("width"),            false,    _t("auto")), _t("auto"));
    m_css_height.fromString(        get_style_property(_t("height"),        false,    _t("auto")), _t("auto"));

    doc->cvt_units(m_css_width, m_font_size);
    doc->cvt_units(m_css_height, m_font_size);

    m_css_min_width.fromString(        get_style_property(_t("min-width"),        false,    _t("0")));
    m_css_min_height.fromString(    get_style_property(_t("min-height"),        false,    _t("0")));

    m_css_max_width.fromString(        get_style_property(_t("max-width"),        false,    _t("none")),    _t("none"));
    m_css_max_height.fromString(    get_style_property(_t("max-height"),        false,    _t("none")),    _t("none"));

    doc->cvt_units(m_css_min_width, m_font_size);
    doc->cvt_units(m_css_min_height, m_font_size);

    m_css_offsets.left.fromString(        get_style_property(_t("left"),                false,    _t("auto")), _t("auto"));
    m_css_offsets.right.fromString(        get_style_property(_t("right"),                false,    _t("auto")), _t("auto"));
    m_css_offsets.top.fromString(        get_style_property(_t("top"),                false,    _t("auto")), _t("auto"));
    m_css_offsets.bottom.fromString(    get_style_property(_t("bottom"),            false,    _t("auto")), _t("auto"));

    doc->cvt_units(m_css_offsets.left, m_font_size);
    doc->cvt_units(m_css_offsets.right, m_font_size);
    doc->cvt_units(m_css_offsets.top,        m_font_size);
    doc->cvt_units(m_css_offsets.bottom,    m_font_size);

    m_css_margins.left.fromString(        get_style_property(_t("margin-left"),        false,    _t("0")), _t("auto"));
    m_css_margins.right.fromString(        get_style_property(_t("margin-right"),        false,    _t("0")), _t("auto"));
    m_css_margins.top.fromString(        get_style_property(_t("margin-top"),            false,    _t("0")), _t("auto"));
    m_css_margins.bottom.fromString(    get_style_property(_t("margin-bottom"),        false,    _t("0")), _t("auto"));

    m_css_padding.left.fromString(        get_style_property(_t("padding-left"),        false,    _t("0")), _t(""));
    m_css_padding.right.fromString(        get_style_property(_t("padding-right"),        false,    _t("0")), _t(""));
    m_css_padding.top.fromString(        get_style_property(_t("padding-top"),        false,    _t("0")), _t(""));
    m_css_padding.bottom.fromString(    get_style_property(_t("padding-bottom"),        false,    _t("0")), _t(""));

    m_css_borders.left.width.fromString(    get_style_property(_t("border-left-width"),        false,    _t("medium")), border_width_strings);
    m_css_borders.right.width.fromString(    get_style_property(_t("border-right-width"),        false,    _t("medium")), border_width_strings);
    m_css_borders.top.width.fromString(        get_style_property(_t("border-top-width"),        false,    _t("medium")), border_width_strings);
    m_css_borders.bottom.width.fromString(    get_style_property(_t("border-bottom-width"),    false,    _t("medium")), border_width_strings);

    m_css_borders.left.color = web_color::from_string(get_style_property(_t("border-left-color"),    false,    _t("")), doc->container());
    m_css_borders.left.style = (border_style) value_index(get_style_property(_t("border-left-style"), false, _t("none")), border_style_strings, border_style_none);

    m_css_borders.right.color = web_color::from_string(get_style_property(_t("border-right-color"), false, _t("")), doc->container());
    m_css_borders.right.style = (border_style) value_index(get_style_property(_t("border-right-style"), false, _t("none")), border_style_strings, border_style_none);

    m_css_borders.top.color = web_color::from_string(get_style_property(_t("border-top-color"), false, _t("")), doc->container());
    m_css_borders.top.style = (border_style) value_index(get_style_property(_t("border-top-style"), false, _t("none")), border_style_strings, border_style_none);

    m_css_borders.bottom.color = web_color::from_string(get_style_property(_t("border-bottom-color"), false, _t("")), doc->container());
    m_css_borders.bottom.style = (border_style) value_index(get_style_property(_t("border-bottom-style"), false, _t("none")), border_style_strings, border_style_none);

    m_css_borders.radius.top_left_x.fromString(get_style_property(_t("border-top-left-radius-x"), false, _t("0")));
    m_css_borders.radius.top_left_y.fromString(get_style_property(_t("border-top-left-radius-y"), false, _t("0")));

    m_css_borders.radius.top_right_x.fromString(get_style_property(_t("border-top-right-radius-x"), false, _t("0")));
    m_css_borders.radius.top_right_y.fromString(get_style_property(_t("border-top-right-radius-y"), false, _t("0")));

    m_css_borders.radius.bottom_right_x.fromString(get_style_property(_t("border-bottom-right-radius-x"), false, _t("0")));
    m_css_borders.radius.bottom_right_y.fromString(get_style_property(_t("border-bottom-right-radius-y"), false, _t("0")));

    m_css_borders.radius.bottom_left_x.fromString(get_style_property(_t("border-bottom-left-radius-x"), false, _t("0")));
    m_css_borders.radius.bottom_left_y.fromString(get_style_property(_t("border-bottom-left-radius-y"), false, _t("0")));

    doc->cvt_units(m_css_borders.radius.bottom_left_x,            m_font_size);
    doc->cvt_units(m_css_borders.radius.bottom_left_y,            m_font_size);
    doc->cvt_units(m_css_borders.radius.bottom_right_x,            m_font_size);
    doc->cvt_units(m_css_borders.radius.bottom_right_y,            m_font_size);
    doc->cvt_units(m_css_borders.radius.top_left_x,                m_font_size);
    doc->cvt_units(m_css_borders.radius.top_left_y,                m_font_size);
    doc->cvt_units(m_css_borders.radius.top_right_x,                m_font_size);
    doc->cvt_units(m_css_borders.radius.top_right_y,                m_font_size);

    doc->cvt_units(m_css_text_indent,                                m_font_size);

    m_margins.left        = doc->cvt_units(m_css_margins.left,        m_font_size);
    m_margins.right        = doc->cvt_units(m_css_margins.right,        m_font_size);
    m_margins.top        = doc->cvt_units(m_css_margins.top,        m_font_size);
    m_margins.bottom    = doc->cvt_units(m_css_margins.bottom,    m_font_size);

    m_padding.left        = doc->cvt_units(m_css_padding.left,        m_font_size);
    m_padding.right        = doc->cvt_units(m_css_padding.right,        m_font_size);
    m_padding.top        = doc->cvt_units(m_css_padding.top,        m_font_size);
    m_padding.bottom    = doc->cvt_units(m_css_padding.bottom,    m_font_size);

    m_borders.left        = doc->cvt_units(m_css_borders.left.width,    m_font_size);
    m_borders.right        = doc->cvt_units(m_css_borders.right.width,    m_font_size);
    m_borders.top        = doc->cvt_units(m_css_borders.top.width,        m_font_size);
    m_borders.bottom    = doc->cvt_units(m_css_borders.bottom.width,    m_font_size);

    css_length line_height;
    line_height.fromString(get_style_property(_t("line-height"),    true,    _t("normal")), _t("normal"));
    if(line_height.is_predefined())
    {
        m_line_height = m_font_metrics.height;
        m_lh_predefined = true;
    } else if(line_height.units() == css_units_none)
    {
        m_line_height = (int) (line_height.val() * m_font_size);
        m_lh_predefined = false;
    } else
    {
        m_line_height =  doc->cvt_units(line_height,    m_font_size, m_font_size);
        m_lh_predefined = false;
    }


    if(m_display == display_list_item)
    {
        const tchar_t* list_type = get_style_property(_t("list-style-type"), true, _t("disc"));
        m_list_style_type = (list_style_type) value_index(list_type, list_style_type_strings, list_style_type_disc);

        const tchar_t* list_pos = get_style_property(_t("list-style-position"), true, _t("outside"));
        m_list_style_position = (list_style_position) value_index(list_pos, list_style_position_strings, list_style_position_outside);

        const tchar_t* list_image = get_style_property(_t("list-style-image"), true, 0);
        if(list_image && list_image[0])
        {
            tstring url;
            css::parse_css_url(list_image, url);

            const tchar_t* list_image_baseurl = get_style_property(_t("list-style-image-baseurl"), true, 0);
            doc->container()->load_image(url.c_str(), list_image_baseurl, true);
        }

    }

    parse_background();

    if(!is_reparse)
    {
        for(auto& el : m_children)
        {
            el->parse_styles();
        }
    }
}

bool litehtml::html_tag::find_styles_changes( position::vector& redraw_boxes, int x, int y )
{
    if(m_display == display_inline_text)
    {
        return false;
    }

    bool ret = false;
    bool apply = false;
    for (used_selector::vector::iterator iter = m_used_styles.begin(); iter != m_used_styles.end() && !apply; iter++)
    {
        if((*iter)->m_selector->is_media_valid())
        {
            int res = select(*((*iter)->m_selector), true);
            if( (res == select_no_match && (*iter)->m_used) || (res == select_match && !(*iter)->m_used) )
            {
                apply = true;
            }
        }
    }

    if(apply)
    {
        if(m_display == display_inline ||  m_display == display_table_row)
        {
            position::vector boxes;
            get_inline_boxes(boxes);
            for(position::vector::iterator pos = boxes.begin(); pos != boxes.end(); pos++)
            {
                pos->x    += x;
                pos->y    += y;
                redraw_boxes.push_back(*pos);
            }
        } else
        {
            position pos = m_pos;
            if(m_el_position != element_position_fixed)
            {
                pos.x += x;
                pos.y += y;
            }
            pos += m_padding;
            pos += m_borders;
            redraw_boxes.push_back(pos);
        }

        ret = true;
        refresh_styles();
        parse_styles();
    }
    for (auto& el : m_children)
    {
        if(!el->skip())
        {
            if(m_el_position != element_position_fixed)
            {
                if(el->find_styles_changes(redraw_boxes, x + m_pos.x, y + m_pos.y))
                {
                    ret = true;
                }
            } else
            {
                if(el->find_styles_changes(redraw_boxes, m_pos.x, m_pos.y))
                {
                    ret = true;
                }
            }
        }
    }
    return ret;
}

void litehtml::html_tag::add_style( const litehtml::style& st )
{
    m_style.combine(st);
}

void litehtml::html_tag::refresh_styles()
{
    remove_before_after();

    for (auto& el : m_children)
    {
        if(el->get_display() != display_inline_text)
        {
            el->refresh_styles();
        }
    }

    m_style.clear();

    for (auto& usel : m_used_styles)
    {
        usel->m_used = false;

        if(usel->m_selector->is_media_valid())
        {
            int apply = select(*usel->m_selector, false);

            if(apply != select_no_match)
            {
                if(apply & select_match_pseudo_class)
                {
                    if(select(*usel->m_selector, true))
                    {
                        if(apply & select_match_with_after)
                        {
                            element::ptr el = get_element_after();
                            if(el)
                            {
                                el->add_style(*usel->m_selector->m_style);
                            }
                        } else if(apply & select_match_with_before)
                        {
                            element::ptr el = get_element_before();
                            if(el)
                            {
                                el->add_style(*usel->m_selector->m_style);
                            }
                        }
                        else
                        {
                            add_style(*usel->m_selector->m_style);
                            usel->m_used = true;
                        }
                    }
                } else if(apply & select_match_with_after)
                {
                    element::ptr el = get_element_after();
                    if(el)
                    {
                        el->add_style(*usel->m_selector->m_style);
                    }
                } else if(apply & select_match_with_before)
                {
                    element::ptr el = get_element_before();
                    if(el)
                    {
                        el->add_style(*usel->m_selector->m_style);
                    }
                } else
                {
                    add_style(*usel->m_selector->m_style);
                    usel->m_used = true;
                }
            }
        }
    }
}
