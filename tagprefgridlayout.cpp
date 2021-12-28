/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.1-0                */
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

#include "tagprefgridlayout.h"

TagPrefGridLayout::TagPrefGridLayout(QWidget* _parent) : QGridLayout(_parent), line(0)
{
}

const TagPreference* TagPrefGridLayout::getTagPreference(const QString& _item) const
{
  if (tp.contains(_item))
    return tp[_item];
  return NULL;
}

void TagPrefGridLayout::addTagPreference(const QString& _name, const QString& _regexDefault)
{
  TagPreference* tpw = new TagPreference(line, _name, _regexDefault, this);
  tp[_name] = tpw;
  line++;
}
