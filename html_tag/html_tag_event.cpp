//
//  html_tag_event.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"

bool litehtml::html_tag::on_mouse_over()
{
    bool ret = false;

    element::ptr el = shared_from_this();
    while(el)
    {
        if(el->set_pseudo_class(_t("hover"), true))
        {
            ret = true;
        }
        el = el->parent();
    }

    return ret;
}

bool litehtml::html_tag::on_mouse_leave()
{
    bool ret = false;

    element::ptr el = shared_from_this();
    while(el)
    {
        if(el->set_pseudo_class(_t("hover"), false))
        {
            ret = true;
        }
        if(el->set_pseudo_class(_t("active"), false))
        {
            ret = true;
        }
        el = el->parent();
    }

    return ret;
}

bool litehtml::html_tag::on_lbutton_down()
{
    bool ret = false;

    element::ptr el = shared_from_this();
    while (el)
    {
        if (el->set_pseudo_class(_t("active"), true))
        {
            ret = true;
        }
        el = el->parent();
    }

    return ret;
}

bool litehtml::html_tag::on_lbutton_up()
{
    bool ret = false;

    element::ptr el = shared_from_this();
    while (el)
    {
        if (el->set_pseudo_class(_t("active"), false))
        {
            ret = true;
        }
        el = el->parent();
    }

    on_click();

    return ret;
}

void litehtml::html_tag::on_click()
{
    if (have_parent())
    {
        element::ptr el_parent = parent();
        if (el_parent)
        {
            el_parent->on_click();
        }
    }
}
