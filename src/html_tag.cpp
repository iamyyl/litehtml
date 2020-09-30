#include "html.h"
#include "html_tag.h"
#include "document.h"
#include "iterators.h"
#include "stylesheet.h"
#include "table.h"
#include <algorithm>
#include <locale>
#include "el_before_after.h"
#include "num_cvt.h"

litehtml::html_tag::html_tag(const std::shared_ptr<litehtml::document>& doc) : litehtml::element(doc)
{
	m_box_sizing			= box_sizing_content_box;
	m_z_index				= 0;
	m_overflow				= overflow_visible;
	m_box					= 0;
	m_text_align			= text_align_left;
	m_el_position			= element_position_static;
	m_display				= display_inline;
	m_vertical_align		= va_baseline;
	m_list_style_type		= list_style_type_none;
	m_list_style_position	= list_style_position_outside;
	m_float					= float_none;
	m_clear					= clear_none;
	m_font					= 0;
	m_font_size				= 0;
	m_white_space			= white_space_normal;
	m_lh_predefined			= false;
	m_line_height			= 0;
	m_visibility			= visibility_visible;
	m_border_spacing_x		= 0;
	m_border_spacing_y		= 0;
	m_border_collapse		= border_collapse_separate;
}

litehtml::html_tag::~html_tag()
{

}

const litehtml::tchar_t* litehtml::html_tag::get_tagName() const
{
	return m_tag.c_str();
}

void litehtml::html_tag::set_attr( const tchar_t* name, const tchar_t* val )
{
	if(name && val)
	{
		tstring s_val = name;
		for(size_t i = 0; i < s_val.length(); i++)
		{
			s_val[i] = std::tolower(s_val[i], std::locale::classic());
		}
		m_attrs[s_val] = val;

		if( t_strcasecmp( name, _t("class") ) == 0 )
		{
			m_class_values.resize( 0 );
			split_string( val, m_class_values, _t(" ") );
		}
	}
}

const litehtml::tchar_t* litehtml::html_tag::get_attr( const tchar_t* name, const tchar_t* def ) const
{
	string_map::const_iterator attr = m_attrs.find(name);
	if(attr != m_attrs.end())
	{
		return attr->second.c_str();
	}
	return def;
}

void litehtml::html_tag::get_content_size( size& sz, int max_width )
{
	sz.height	= 0;
	if(m_display == display_block)
	{
		sz.width	= max_width;
	} else
	{
		sz.width	= 0;
	}
}

bool litehtml::html_tag::is_white_space() const
{
	return false;
}

void litehtml::html_tag::add_positioned(const element::ptr &el)
{
	if (m_el_position != element_position_static || (!have_parent()))
	{
		m_positioned.push_back(el);
	} else
	{
		element::ptr el_parent = parent();
		if (el_parent)
		{
			el_parent->add_positioned(el);
		}
	}
}

void litehtml::html_tag::get_text( tstring& text )
{
	for (auto& el : m_children)
	{
		el->get_text(text);
	}
}

bool litehtml::html_tag::is_body()  const
{
	return false;
}

void litehtml::html_tag::set_data( const tchar_t* data )
{

}

const litehtml::tchar_t* litehtml::html_tag::get_cursor()
{
	return get_style_property(_t("cursor"), true, 0);
}

bool litehtml::html_tag::is_break() const
{
	return false;
}

void litehtml::html_tag::set_tagName( const tchar_t* tag )
{
	tstring s_val = tag;
	for(size_t i = 0; i < s_val.length(); i++)
	{
		s_val[i] = std::tolower(s_val[i], std::locale::classic());
	}
	m_tag = s_val;
}

bool litehtml::html_tag::set_pseudo_class( const tchar_t* pclass, bool add )
{
	bool ret = false;
	if(add)
	{
		if(std::find(m_pseudo_classes.begin(), m_pseudo_classes.end(), pclass) == m_pseudo_classes.end())
		{
			m_pseudo_classes.push_back(pclass);
			ret = true;
		}
	} else
	{
		string_vector::iterator pi = std::find(m_pseudo_classes.begin(), m_pseudo_classes.end(), pclass);
		if(pi != m_pseudo_classes.end())
		{
			m_pseudo_classes.erase(pi);
			ret = true;
		}
	}
	return ret;
}

bool litehtml::html_tag::set_class( const tchar_t* pclass, bool add )
{
	string_vector classes;
	bool changed = false;

	split_string( pclass, classes, _t(" ") );

	if(add)
	{
		for( auto & _class : classes  )
		{
			if(std::find(m_class_values.begin(), m_class_values.end(), _class) == m_class_values.end())
			{
				m_class_values.push_back( std::move( _class ) );
				changed = true;
			}
		}
	} else
	{
		for( const auto & _class : classes )
		{
			auto end = std::remove(m_class_values.begin(), m_class_values.end(), _class);

			if(end != m_class_values.end())
			{
				m_class_values.erase(end, m_class_values.end());
				changed = true;
			}
		}
	}

	if( changed )
	{
		tstring class_string;
		join_string(class_string, m_class_values, _t(" "));
		set_attr(_t("class"), class_string.c_str());

		return true;
	}
	else
	{
		return false;
	}
}

bool litehtml::html_tag::is_replaced() const
{
	return false;
}

litehtml::style_display litehtml::html_tag::get_display() const
{
	return m_display;
}


litehtml::white_space litehtml::html_tag::get_white_space() const
{
	return m_white_space;
}

litehtml::vertical_align litehtml::html_tag::get_vertical_align() const
{
	return m_vertical_align;
}


litehtml::element_clear litehtml::html_tag::get_clear() const
{
	return m_clear;
}



litehtml::element_position litehtml::html_tag::get_element_position(css_offsets* offsets) const
{
	if(offsets && m_el_position != element_position_static)
	{
		*offsets = m_css_offsets;
	}
	return m_el_position;
}



litehtml::visibility litehtml::html_tag::get_visibility() const
{
	return m_visibility;
}


litehtml::tstring litehtml::html_tag::get_list_marker_text(int index)
{
	switch (m_list_style_type)
	{
	case litehtml::list_style_type_decimal:
		return t_to_string(index);
	case litehtml::list_style_type_decimal_leading_zero:
		{
			auto txt = t_to_string(index);
			if (txt.length() == 1)
			{
				txt = _t("0") + txt;
			}
			return txt;
		}
	case litehtml::list_style_type_lower_latin:
	case litehtml::list_style_type_lower_alpha:
		return num_cvt::to_latin_lower(index);
	case litehtml::list_style_type_lower_greek:
		return num_cvt::to_greek_lower(index);
	case litehtml::list_style_type_upper_alpha:
	case litehtml::list_style_type_upper_latin:
		return num_cvt::to_latin_upper(index);
	case litehtml::list_style_type_lower_roman:
		return num_cvt::to_roman_lower(index);
	case litehtml::list_style_type_upper_roman:
		return num_cvt::to_roman_upper(index);
	case litehtml::list_style_type_armenian:
		break;
	case litehtml::list_style_type_georgian:
		break;
	case litehtml::list_style_type_hebrew:
		break;
	case litehtml::list_style_type_hiragana:
		break;
	case litehtml::list_style_type_hiragana_iroha:
		break;
	case litehtml::list_style_type_katakana:
		break;
	case litehtml::list_style_type_katakana_iroha:
		break;
	}
	return _t("");
}

bool litehtml::html_tag::fetch_positioned()
{
	bool ret = false;

	m_positioned.clear();

	litehtml::element_position el_pos;

	for(auto& el : m_children)
	{
		el_pos = el->get_element_position();
		if (el_pos != element_position_static)
		{
			add_positioned(el);
		}
		if (!ret && (el_pos == element_position_absolute || el_pos == element_position_fixed))
		{
			ret = true;
		}
		if(el->fetch_positioned())
		{
			ret = true;
		}
	}
	return ret;
}

int litehtml::html_tag::get_zindex() const
{
	return m_z_index;
}

litehtml::overflow litehtml::html_tag::get_overflow() const
{
	return m_overflow;
}

const litehtml::background* litehtml::html_tag::get_background(bool own_only)
{
	if(own_only)
	{
		// return own background with check for empty one
		if(m_bg.m_image.empty() && !m_bg.m_color.alpha)
		{
			return 0;
		}
		return &m_bg;
	}

	if(m_bg.m_image.empty() && !m_bg.m_color.alpha)
	{
		// if this is root element (<html>) try to get background from body
		if (!have_parent())
		{
			for (const auto& el : m_children)
			{
				if( el->is_body() )
				{
					// return own body background
					return el->get_background(true);
				}
			}
		}
		return 0;
	}

	if(is_body())
	{
		element::ptr el_parent = parent();
		if (el_parent)
		{
			if (!el_parent->get_background(true))
			{
				// parent of body will draw background for body
				return 0;
			}
		}
	}

	return &m_bg;
}
