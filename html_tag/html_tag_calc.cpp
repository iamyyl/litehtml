//
//  html_tag_calc.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"


void litehtml::html_tag::calc_outlines( int parent_width )
{
    m_padding.left    = m_css_padding.left.calc_percent(parent_width);
    m_padding.right    = m_css_padding.right.calc_percent(parent_width);

    m_borders.left    = m_css_borders.left.width.calc_percent(parent_width);
    m_borders.right    = m_css_borders.right.width.calc_percent(parent_width);

    m_margins.left    = m_css_margins.left.calc_percent(parent_width);
    m_margins.right    = m_css_margins.right.calc_percent(parent_width);

    m_margins.top        = m_css_margins.top.calc_percent(parent_width);
    m_margins.bottom    = m_css_margins.bottom.calc_percent(parent_width);

    m_padding.top        = m_css_padding.top.calc_percent(parent_width);
    m_padding.bottom    = m_css_padding.bottom.calc_percent(parent_width);
}

void litehtml::html_tag::calc_auto_margins(int parent_width)
{
    if (get_element_position() != element_position_absolute && (m_display == display_block || m_display == display_table))
    {
        if (m_css_margins.left.is_predefined() && m_css_margins.right.is_predefined())
        {
            int el_width = m_pos.width + m_borders.left + m_borders.right + m_padding.left + m_padding.right;
            if (el_width <= parent_width)
            {
                m_margins.left = (parent_width - el_width) / 2;
                m_margins.right = (parent_width - el_width) - m_margins.left;
            }
            else
            {
                m_margins.left = 0;
                m_margins.right = 0;
            }
        }
        else if (m_css_margins.left.is_predefined() && !m_css_margins.right.is_predefined())
        {
            int el_width = m_pos.width + m_borders.left + m_borders.right + m_padding.left + m_padding.right + m_margins.right;
            m_margins.left = parent_width - el_width;
            if (m_margins.left < 0) m_margins.left = 0;
        }
        else if (!m_css_margins.left.is_predefined() && m_css_margins.right.is_predefined())
        {
            int el_width = m_pos.width + m_borders.left + m_borders.right + m_padding.left + m_padding.right + m_margins.left;
            m_margins.right = parent_width - el_width;
            if (m_margins.right < 0) m_margins.right = 0;
        }
    }
}

void litehtml::html_tag::calc_document_size( litehtml::size& sz, int x /*= 0*/, int y /*= 0*/ )
{
    if(is_visible() && m_el_position != element_position_fixed)
    {
        element::calc_document_size(sz, x, y);

        if(m_overflow == overflow_visible)
        {
            for(auto& el : m_children)
            {
                el->calc_document_size(sz, x + m_pos.x, y + m_pos.y);
            }
        }

        // root element (<html>) must to cover entire window
        if(!have_parent())
        {
            position client_pos;
            get_document()->container()->get_client_rect(client_pos);
            m_pos.height = std::max(sz.height, client_pos.height) - content_margins_top() - content_margins_bottom();
            m_pos.width     = std::max(sz.width, client_pos.width) - content_margins_left() - content_margins_right();
        }
    }
}

