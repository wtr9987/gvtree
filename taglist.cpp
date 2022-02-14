/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.2-0                */
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

#include "taglist.h"

TagList::TagList(QWidget* _parent) : QListWidget(_parent)
{
  setSelectionMode(QAbstractItemView::SingleSelection);
}

void TagList::addData(const QStringList& _data)
{
    QSet<QString> add = _data.toSet();
    raw.unite(add);
    QStringList current = raw.toList();
    qSort(current.begin(), current.end(), qGreater<QString>()); 
    QListWidget::clear();
    addItems(current);
}

void TagList::clear()
{
    raw.clear();
    QListWidget::clear();
}
