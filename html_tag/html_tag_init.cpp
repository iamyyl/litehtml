//
//  html_tag_init.cpp
//  litehtml
//
//  Created by Yu, Hugo (NSB - CN/Hangzhou) on 2020/9/30.
//

#include "html.h"
#include "html_tag.h"
#include "iterators.h"

void litehtml::html_tag::init()
{
    if (m_display == display_table || m_display == display_inline_table)
    {
        if (m_grid)
        {
            m_grid->clear();
        }
        else
        {
            m_grid = std::unique_ptr<table_grid>(new table_grid());
        }

        go_inside_table         table_selector;
        table_rows_selector        row_selector;
        table_cells_selector    cell_selector;

        elements_iterator row_iter(shared_from_this(), &table_selector, &row_selector);

        element::ptr row = row_iter.next(false);
        while (row)
        {
            m_grid->begin_row(row);

            elements_iterator cell_iter(row, &table_selector, &cell_selector);
            element::ptr cell = cell_iter.next();
            while (cell)
            {
                m_grid->add_cell(cell);

                cell = cell_iter.next(false);
            }
            row = row_iter.next(false);
        }

        m_grid->finish();
    }

    for (auto& el : m_children)
    {
        el->init();
    }
}
