//
//  html_tag_css.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"

litehtml::css_length litehtml::html_tag::get_css_left() const
{
    return m_css_offsets.left;
}

litehtml::css_length litehtml::html_tag::get_css_right() const
{
    return m_css_offsets.right;
}

litehtml::css_length litehtml::html_tag::get_css_top() const
{
    return m_css_offsets.top;
}

litehtml::css_length litehtml::html_tag::get_css_bottom() const
{
    return m_css_offsets.bottom;
}


litehtml::css_offsets litehtml::html_tag::get_css_offsets() const
{
    return m_css_offsets;
}

litehtml::css_length litehtml::html_tag::get_css_width() const
{
    return m_css_width;
}

litehtml::css_length litehtml::html_tag::get_css_height() const
{
    return m_css_height;
}

void litehtml::html_tag::set_css_width( css_length& w )
{
    m_css_width = w;
}

