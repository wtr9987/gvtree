/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.3-0                */
/*                                               */
/*             git version tree browser          */
/*                                               */
/*   28. December 2021                           */
/*                                               */
/*         This program is licensed under        */
/*           GNU GENERAL PUBLIC LICENSE          */
/*            Version 3, 29 June 2007            */
/*                                               */
/* --------------------------------------------- */

#include <algorithm>
#include "taglist.h"

TagList::TagList(QWidget* _parent) : QListWidget(_parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void TagList::addData(const QStringList& _data)
{
#if QT_VERSION >= 0x51400
    QSet<QString> add(_data.begin(), _data.end());
#else
    QSet<QString> add = _data.toSet();
#endif
    raw.unite(add);
    QStringList current = raw.values();
    std::sort(current.begin(), current.end(), std::greater<QString>());
    QListWidget::clear();
    addItems(current);
}

void TagList::clear()
{
    raw.clear();
    QListWidget::clear();
}
