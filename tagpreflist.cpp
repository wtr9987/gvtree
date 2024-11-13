/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.9-0                */
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

// TODO RegExp rules are greedy, the order is relevant
// TODO allow shifting of the rules

#include <iostream>
#include "tagpreflist.h"
#include <QSpacerItem>

TagPrefList::TagPrefList(QWidget* _parent) : QWidget(_parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addSpacerItem(new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Maximum));

    setLayout(layout);
}

const TagPreference* TagPrefList::getTagPreference(const QString& _item) const
{
    if (tp.contains(_item))
        return tp[_item];

    return NULL;
}

void TagPrefList::addTagPreference(const QString& _name,
                                         const QString& _regexp,
                                         const QString& _color,
                                         const QString& _font,
                                         bool _regexpChangeable,
                                         int _fold)
{
    if (tp.contains(_name))
        return;

    TagPreference* tpw = new TagPreference(_name, _regexp, _color, _font, bgcolor, true, _regexpChangeable, _fold , this);

    connect(this, SIGNAL(sigSetBackgroundColor(const QColor&)), tpw, SLOT(setBackgroundColor(const QColor&)));

    connect(tpw, SIGNAL(deleteTagPreference(const QString&)), this, SLOT(deleteTagPreference(const QString&)));
    connect(tpw, SIGNAL(addTagPreference(const QString&)), this, SLOT(addTagPreferenceShort(const QString&)));

    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(layout());
    if (l)
    {
    l->insertWidget(layout()->count()-1,tpw);
    }

    connect(tpw, SIGNAL(regexpChanged(const QString&)), this, SLOT(regexpChangedProxy(const QString&)));
    connect(tpw, SIGNAL(visibilityChanged(const QString&)), this, SLOT(visibilityChangedProxy(const QString&)));
    connect(tpw, SIGNAL(foldChanged(const QString&)), this, SLOT(foldChangedProxy(const QString&)));
    connect(tpw, SIGNAL(moveUp(TagPreference*)), this, SLOT(moveUp(TagPreference*)));
    connect(tpw, SIGNAL(moveDown(TagPreference*)), this, SLOT(moveDown(TagPreference*)));
    tp[_name] = tpw;
}

void TagPrefList::addTagPreferenceShort(const QString& _name)
{
    if (tp.contains(_name))
        return;

    QColor color = (bgcolor.lightness() < 128) ? QColor(255, 255, 255) : QColor(0, 0, 0);
    QFont font = QFont();

    addTagPreference(_name, QString(), color.name(), font.toString(), true, true);

    emit elementChanged();
}

void TagPrefList::deleteTagPreference(const QString& _name)
{
    if (!tp.contains(_name))
        return;

    QMap<QString, TagPreference*>::iterator it = tp.find(_name);

    layout()->removeWidget(it.value());
    it.value()->deleteLater();
    tp.erase(it);

    emit elementChanged();
}

void TagPrefList::regexpChangedProxy(const QString& _text)
{
    emit regexpChanged(_text);
}

void TagPrefList::visibilityChangedProxy(const QString& _text)
{
    emit visibilityChanged(_text);
}

void TagPrefList::foldChangedProxy(const QString& _text)
{
    emit foldChanged(_text);
}

void TagPrefList::setBackgroundColor(const QColor& _bgcolor)
{
    bgcolor = _bgcolor;
    emit sigSetBackgroundColor(_bgcolor);
}

void TagPrefList::getVisibleTagPreferences(QStringList& _visible) const
{
    _visible.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
        if (it.value()->getVisibility())
        {
            _visible << it.value()->text();
        }
    }
}

void TagPrefList::getTagPreferences(QStringList& _visible) const
{
    _visible.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
        _visible << it.value()->text();
    }
}

void TagPrefList::getChangeableTagPreferences(QStringList& _changeable) const
{
    _changeable.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
      if (it.value()->getChangeable())
        _changeable << it.value()->text();
    }
}

void TagPrefList::moveUp(TagPreference* _item)
{
  QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());
  if (!layout)
    return;

  int index=-1;
  for (int i=0;i<layout->count()-1;i++)
  {
    if (_item == layout->itemAt(i)->widget())
    {
      index = i;
      break;
    }
  }

  if (index==-1 || index==0)
    return;

  QLayoutItem* item1 = layout->takeAt(index);
  layout->insertItem(index-1,item1);
  layout->update();
}

void TagPrefList::moveDown(TagPreference* _item)
{
  QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());
  if (!layout)
    return;

  int index=-1;
  for (int i=0;i<layout->count()-1;i++)
  {
    if (_item == layout->itemAt(i)->widget())
    {
      index = i;
      break;
    }
  }

  if (index==-1 || (index>=layout->count()-2))
    return;

  QLayoutItem* item1 = layout->takeAt(index);
  layout->insertItem(index+1,item1);
  layout->update();
}
