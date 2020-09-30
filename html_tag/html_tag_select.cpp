//
//  html_tag_select.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "document.h"

litehtml::elements_vector litehtml::html_tag::select_all( const tstring& selector )
{
    css_selector sel(media_query_list::ptr(0));
    sel.parse(selector);

    return select_all(sel);
}

litehtml::elements_vector litehtml::html_tag::select_all( const css_selector& selector )
{
    litehtml::elements_vector res;
    select_all(selector, res);
    return res;
}

void litehtml::html_tag::select_all(const css_selector& selector, elements_vector& res)
{
    if(select(selector))
    {
        res.push_back(shared_from_this());
    }

    for(auto& el : m_children)
    {
        el->select_all(selector, res);
    }
}


litehtml::element::ptr litehtml::html_tag::select_one( const tstring& selector )
{
    css_selector sel(media_query_list::ptr(0));
    sel.parse(selector);

    return select_one(sel);
}

litehtml::element::ptr litehtml::html_tag::select_one( const css_selector& selector )
{
    if(select(selector))
    {
        return shared_from_this();
    }

    for(auto& el : m_children)
    {
        element::ptr res = el->select_one(selector);
        if(res)
        {
            return res;
        }
    }
    return 0;
}

int litehtml::html_tag::select(const css_selector& selector, bool apply_pseudo)
{
    int right_res = select(selector.m_right, apply_pseudo);
    if(right_res == select_no_match)
    {
        return select_no_match;
    }
    element::ptr el_parent = parent();
    if(selector.m_left)
    {
        if (!el_parent)
        {
            return select_no_match;
        }
        switch(selector.m_combinator)
        {
        case combinator_descendant:
            {
                bool is_pseudo = false;
                element::ptr res = find_ancestor(*selector.m_left, apply_pseudo, &is_pseudo);
                if(!res)
                {
                    return select_no_match;
                } else
                {
                    if(is_pseudo)
                    {
                        right_res |= select_match_pseudo_class;
                    }
                }
            }
            break;
        case combinator_child:
            {
                int res = el_parent->select(*selector.m_left, apply_pseudo);
                if(res == select_no_match)
                {
                    return select_no_match;
                } else
                {
                    if(right_res != select_match_pseudo_class)
                    {
                        right_res |= res;
                    }
                }
            }
            break;
        case combinator_adjacent_sibling:
            {
                bool is_pseudo = false;
                element::ptr res = el_parent->find_adjacent_sibling(shared_from_this(), *selector.m_left, apply_pseudo, &is_pseudo);
                if(!res)
                {
                    return select_no_match;
                } else
                {
                    if(is_pseudo)
                    {
                        right_res |= select_match_pseudo_class;
                    }
                }
            }
            break;
        case combinator_general_sibling:
            {
                bool is_pseudo = false;
                element::ptr res =  el_parent->find_sibling(shared_from_this(), *selector.m_left, apply_pseudo, &is_pseudo);
                if(!res)
                {
                    return select_no_match;
                } else
                {
                    if(is_pseudo)
                    {
                        right_res |= select_match_pseudo_class;
                    }
                }
            }
            break;
        default:
            right_res = select_no_match;
        }
    }
    return right_res;
}

int litehtml::html_tag::select(const css_element_selector& selector, bool apply_pseudo)
{
    if(!selector.m_tag.empty() && selector.m_tag != _t("*"))
    {
        if(selector.m_tag != m_tag)
        {
            return select_no_match;
        }
    }

    int res = select_match;
    element::ptr el_parent = parent();

    for(css_attribute_selector::vector::const_iterator i = selector.m_attrs.begin(); i != selector.m_attrs.end(); i++)
    {
        const tchar_t* attr_value = get_attr(i->attribute.c_str());
        switch(i->condition)
        {
        case select_exists:
            if(!attr_value)
            {
                return select_no_match;
            }
            break;
        case select_equal:
            if(!attr_value)
            {
                return select_no_match;
            } else
            {
                if(i->attribute == _t("class"))
                {
                    const string_vector & tokens1 = m_class_values;
                    const string_vector & tokens2 = i->class_val;
                    bool found = true;
                    for(string_vector::const_iterator str1 = tokens2.begin(); str1 != tokens2.end() && found; str1++)
                    {
                        bool f = false;
                        for(string_vector::const_iterator str2 = tokens1.begin(); str2 != tokens1.end() && !f; str2++)
                        {
                            if( !t_strcasecmp(str1->c_str(), str2->c_str()) )
                            {
                                f = true;
                            }
                        }
                        if(!f)
                        {
                            found = false;
                        }
                    }
                    if(!found)
                    {
                        return select_no_match;
                    }
                } else
                {
                    if( t_strcasecmp(i->val.c_str(), attr_value) )
                    {
                        return select_no_match;
                    }
                }
            }
            break;
        case select_contain_str:
            if(!attr_value)
            {
                return select_no_match;
            } else if(!t_strstr(attr_value, i->val.c_str()))
            {
                return select_no_match;
            }
            break;
        case select_start_str:
            if(!attr_value)
            {
                return select_no_match;
            } else if(t_strncmp(attr_value, i->val.c_str(), i->val.length()))
            {
                return select_no_match;
            }
            break;
        case select_end_str:
            if(!attr_value)
            {
                return select_no_match;
            } else if(t_strncmp(attr_value, i->val.c_str(), i->val.length()))
            {
                const tchar_t* s = attr_value + t_strlen(attr_value) - i->val.length() - 1;
                if(s < attr_value)
                {
                    return select_no_match;
                }
                if(i->val != s)
                {
                    return select_no_match;
                }
            }
            break;
        case select_pseudo_element:
            if(i->val == _t("after"))
            {
                res |= select_match_with_after;
            } else if(i->val == _t("before"))
            {
                res |= select_match_with_before;
            } else
            {
                return select_no_match;
            }
            break;
        case select_pseudo_class:
            if(apply_pseudo)
            {
                if (!el_parent) return select_no_match;

                tstring selector_param;
                tstring    selector_name;

                tstring::size_type begin    = i->val.find_first_of(_t('('));
                tstring::size_type end        = (begin == tstring::npos) ? tstring::npos : find_close_bracket(i->val, begin);
                if(begin != tstring::npos && end != tstring::npos)
                {
                    selector_param = i->val.substr(begin + 1, end - begin - 1);
                }
                if(begin != tstring::npos)
                {
                    selector_name = i->val.substr(0, begin);
                    litehtml::trim(selector_name);
                } else
                {
                    selector_name = i->val;
                }

                int selector = value_index(selector_name.c_str(), pseudo_class_strings);

                switch(selector)
                {
                case pseudo_class_only_child:
                    if (!el_parent->is_only_child(shared_from_this(), false))
                    {
                        return select_no_match;
                    }
                    break;
                case pseudo_class_only_of_type:
                    if (!el_parent->is_only_child(shared_from_this(), true))
                    {
                        return select_no_match;
                    }
                    break;
                case pseudo_class_first_child:
                    if (!el_parent->is_nth_child(shared_from_this(), 0, 1, false))
                    {
                        return select_no_match;
                    }
                    break;
                case pseudo_class_first_of_type:
                    if (!el_parent->is_nth_child(shared_from_this(), 0, 1, true))
                    {
                        return select_no_match;
                    }
                    break;
                case pseudo_class_last_child:
                    if (!el_parent->is_nth_last_child(shared_from_this(), 0, 1, false))
                    {
                        return select_no_match;
                    }
                    break;
                case pseudo_class_last_of_type:
                    if (!el_parent->is_nth_last_child(shared_from_this(), 0, 1, true))
                    {
                        return select_no_match;
                    }
                    break;
                case pseudo_class_nth_child:
                case pseudo_class_nth_of_type:
                case pseudo_class_nth_last_child:
                case pseudo_class_nth_last_of_type:
                    {
                        if(selector_param.empty()) return select_no_match;

                        int num = 0;
                        int off = 0;

                        parse_nth_child_params(selector_param, num, off);
                        if(!num && !off) return select_no_match;
                        switch(selector)
                        {
                        case pseudo_class_nth_child:
                            if (!el_parent->is_nth_child(shared_from_this(), num, off, false))
                            {
                                return select_no_match;
                            }
                            break;
                        case pseudo_class_nth_of_type:
                            if (!el_parent->is_nth_child(shared_from_this(), num, off, true))
                            {
                                return select_no_match;
                            }
                            break;
                        case pseudo_class_nth_last_child:
                            if (!el_parent->is_nth_last_child(shared_from_this(), num, off, false))
                            {
                                return select_no_match;
                            }
                            break;
                        case pseudo_class_nth_last_of_type:
                            if (!el_parent->is_nth_last_child(shared_from_this(), num, off, true))
                            {
                                return select_no_match;
                            }
                            break;
                        }

                    }
                    break;
                case pseudo_class_not:
                    {
                        css_element_selector sel;
                        sel.parse(selector_param);
                        if(select(sel, apply_pseudo))
                        {
                            return select_no_match;
                        }
                    }
                    break;
                case pseudo_class_lang:
                    {
                        trim( selector_param );

                        if( !get_document()->match_lang( selector_param ) )
                        {
                            return select_no_match;
                        }
                    }
                    break;
                default:
                    if(std::find(m_pseudo_classes.begin(), m_pseudo_classes.end(), i->val) == m_pseudo_classes.end())
                    {
                        return select_no_match;
                    }
                    break;
                }
            } else
            {
                res |= select_match_pseudo_class;
            }
            break;
        }
    }
    return res;
}
