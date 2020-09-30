//
//  html_tag_font.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

static const int font_size_table[8][7] =
{
    { 9,    9,     9,     9,    11,    14,    18},
    { 9,    9,     9,    10,    12,    15,    20},
    { 9,    9,     9,    11,    13,    17,    22},
    { 9,    9,    10,    12,    14,    18,    24},
    { 9,    9,    10,    13,    16,    20,    26},
    { 9,    9,    11,    14,    17,    21,    28},
    { 9,   10,    12,    15,    17,    23,    30},
    { 9,   10,    13,    16,    18,    24,    32}
};

void litehtml::html_tag::init_font()
{
    // initialize font size
    const tchar_t* str = get_style_property(_t("font-size"), false, 0);

    int parent_sz = 0;
    int doc_font_size = get_document()->container()->get_default_font_size();
    element::ptr el_parent = parent();
    if (el_parent)
    {
        parent_sz = el_parent->get_font_size();
    } else
    {
        parent_sz = doc_font_size;
    }


    if(!str)
    {
        m_font_size = parent_sz;
    } else
    {
        m_font_size = parent_sz;

        css_length sz;
        sz.fromString(str, font_size_strings);
        if(sz.is_predefined())
        {
            int idx_in_table = doc_font_size - 9;
            if(idx_in_table >= 0 && idx_in_table <= 7)
            {
                if(sz.predef() >= fontSize_xx_small && sz.predef() <= fontSize_xx_large)
                {
                    m_font_size = font_size_table[idx_in_table][sz.predef()];
                } else
                {
                    m_font_size = doc_font_size;
                }
            } else
            {
                switch(sz.predef())
                {
                case fontSize_xx_small:
                    m_font_size = doc_font_size * 3 / 5;
                    break;
                case fontSize_x_small:
                    m_font_size = doc_font_size * 3 / 4;
                    break;
                case fontSize_small:
                    m_font_size = doc_font_size * 8 / 9;
                    break;
                case fontSize_large:
                    m_font_size = doc_font_size * 6 / 5;
                    break;
                case fontSize_x_large:
                    m_font_size = doc_font_size * 3 / 2;
                    break;
                case fontSize_xx_large:
                    m_font_size = doc_font_size * 2;
                    break;
                default:
                    m_font_size = doc_font_size;
                    break;
                }
            }
        } else
        {
            if(sz.units() == css_units_percentage)
            {
                m_font_size = sz.calc_percent(parent_sz);
            } else if(sz.units() == css_units_none)
            {
                m_font_size = parent_sz;
            } else
            {
                m_font_size = get_document()->cvt_units(sz, parent_sz);
            }
        }
    }

    // initialize font
    const tchar_t* name            = get_style_property(_t("font-family"),        true,    _t("inherit"));
    const tchar_t* weight        = get_style_property(_t("font-weight"),        true,    _t("normal"));
    const tchar_t* style        = get_style_property(_t("font-style"),        true,    _t("normal"));
    const tchar_t* decoration    = get_style_property(_t("text-decoration"),    true,    _t("none"));

    m_font = get_document()->get_font(name, m_font_size, weight, style, decoration, &m_font_metrics);
}


int litehtml::html_tag::get_font_size() const
{
    return m_font_size;
}

litehtml::uint_ptr litehtml::html_tag::get_font(font_metrics* fm)
{
    if(fm)
    {
        *fm = m_font_metrics;
    }
    return m_font;
}
