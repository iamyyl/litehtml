//
//  html_tag_parse.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

void litehtml::html_tag::parse_attributes()
{
    for(auto& el : m_children)
    {
        el->parse_attributes();
    }
}

void litehtml::html_tag::parse_background()
{
    // parse background-color
    m_bg.m_color        = get_color(_t("background-color"), false, web_color(0, 0, 0, 0));

    // parse background-position
    const tchar_t* str = get_style_property(_t("background-position"), false, _t("0% 0%"));
    if(str)
    {
        string_vector res;
        split_string(str, res, _t(" \t"));
        if(res.size() > 0)
        {
            if(res.size() == 1)
            {
                if( value_in_list(res[0].c_str(), _t("left;right;center")) )
                {
                    m_bg.m_position.x.fromString(res[0], _t("left;right;center"));
                    m_bg.m_position.y.set_value(50, css_units_percentage);
                } else if( value_in_list(res[0].c_str(), _t("top;bottom;center")) )
                {
                    m_bg.m_position.y.fromString(res[0], _t("top;bottom;center"));
                    m_bg.m_position.x.set_value(50, css_units_percentage);
                } else
                {
                    m_bg.m_position.x.fromString(res[0], _t("left;right;center"));
                    m_bg.m_position.y.set_value(50, css_units_percentage);
                }
            } else
            {
                if(value_in_list(res[0].c_str(), _t("left;right")))
                {
                    m_bg.m_position.x.fromString(res[0], _t("left;right;center"));
                    m_bg.m_position.y.fromString(res[1], _t("top;bottom;center"));
                } else if(value_in_list(res[0].c_str(), _t("top;bottom")))
                {
                    m_bg.m_position.x.fromString(res[1], _t("left;right;center"));
                    m_bg.m_position.y.fromString(res[0], _t("top;bottom;center"));
                } else if(value_in_list(res[1].c_str(), _t("left;right")))
                {
                    m_bg.m_position.x.fromString(res[1], _t("left;right;center"));
                    m_bg.m_position.y.fromString(res[0], _t("top;bottom;center"));
                }else if(value_in_list(res[1].c_str(), _t("top;bottom")))
                {
                    m_bg.m_position.x.fromString(res[0], _t("left;right;center"));
                    m_bg.m_position.y.fromString(res[1], _t("top;bottom;center"));
                } else
                {
                    m_bg.m_position.x.fromString(res[0], _t("left;right;center"));
                    m_bg.m_position.y.fromString(res[1], _t("top;bottom;center"));
                }
            }

            if(m_bg.m_position.x.is_predefined())
            {
                switch(m_bg.m_position.x.predef())
                {
                case 0:
                    m_bg.m_position.x.set_value(0, css_units_percentage);
                    break;
                case 1:
                    m_bg.m_position.x.set_value(100, css_units_percentage);
                    break;
                case 2:
                    m_bg.m_position.x.set_value(50, css_units_percentage);
                    break;
                }
            }
            if(m_bg.m_position.y.is_predefined())
            {
                switch(m_bg.m_position.y.predef())
                {
                case 0:
                    m_bg.m_position.y.set_value(0, css_units_percentage);
                    break;
                case 1:
                    m_bg.m_position.y.set_value(100, css_units_percentage);
                    break;
                case 2:
                    m_bg.m_position.y.set_value(50, css_units_percentage);
                    break;
                }
            }
        } else
        {
            m_bg.m_position.x.set_value(0, css_units_percentage);
            m_bg.m_position.y.set_value(0, css_units_percentage);
        }
    } else
    {
        m_bg.m_position.y.set_value(0, css_units_percentage);
        m_bg.m_position.x.set_value(0, css_units_percentage);
    }

    str = get_style_property(_t("background-size"), false, _t("auto"));
    if(str)
    {
        string_vector res;
        split_string(str, res, _t(" \t"));
        if(!res.empty())
        {
            m_bg.m_position.width.fromString(res[0], background_size_strings);
            if(res.size() > 1)
            {
                m_bg.m_position.height.fromString(res[1], background_size_strings);
            } else
            {
                m_bg.m_position.height.predef(background_size_auto);
            }
        } else
        {
            m_bg.m_position.width.predef(background_size_auto);
            m_bg.m_position.height.predef(background_size_auto);
        }
    }

    document::ptr doc = get_document();

    doc->cvt_units(m_bg.m_position.x,        m_font_size);
    doc->cvt_units(m_bg.m_position.y,        m_font_size);
    doc->cvt_units(m_bg.m_position.width,    m_font_size);
    doc->cvt_units(m_bg.m_position.height,    m_font_size);

    // parse background_attachment
    m_bg.m_attachment = (background_attachment) value_index(
        get_style_property(_t("background-attachment"), false, _t("scroll")),
        background_attachment_strings,
        background_attachment_scroll);

    // parse background_attachment
    m_bg.m_repeat = (background_repeat) value_index(
        get_style_property(_t("background-repeat"), false, _t("repeat")),
        background_repeat_strings,
        background_repeat_repeat);

    // parse background_clip
    m_bg.m_clip = (background_box) value_index(
        get_style_property(_t("background-clip"), false, _t("border-box")),
        background_box_strings,
        background_box_border);

    // parse background_origin
    m_bg.m_origin = (background_box) value_index(
        get_style_property(_t("background-origin"), false, _t("padding-box")),
        background_box_strings,
        background_box_content);

    // parse background-image
    css::parse_css_url(get_style_property(_t("background-image"), false, _t("")), m_bg.m_image);
    m_bg.m_baseurl = get_style_property(_t("background-image-baseurl"), false, _t(""));

    if(!m_bg.m_image.empty())
    {
        doc->container()->load_image(m_bg.m_image.c_str(), m_bg.m_baseurl.empty() ? 0 : m_bg.m_baseurl.c_str(), true);
    }
}

void litehtml::html_tag::parse_nth_child_params( tstring param, int &num, int &off )
{
    if(param == _t("odd"))
    {
        num = 2;
        off = 1;
    } else if(param == _t("even"))
    {
        num = 2;
        off = 0;
    } else
    {
        string_vector tokens;
        split_string(param, tokens, _t(" n"), _t("n"));

        tstring s_num;
        tstring s_off;

        tstring s_int;
        for(string_vector::iterator tok = tokens.begin(); tok != tokens.end(); tok++)
        {
            if((*tok) == _t("n"))
            {
                s_num = s_int;
                s_int.clear();
            } else
            {
                s_int += (*tok);
            }
        }
        s_off = s_int;

        num = t_atoi(s_num.c_str());
        off = t_atoi(s_off.c_str());
    }
}

