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

#include "tagpreflist.h"

TagPregList::TagPregList(QWidget* _parent) : QWidget(_parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    setLayout(layout);
}

const TagPreference* TagPregList::getTagPreference(const QString& _item) const
{
    if (tp.contains(_item))
        return tp[_item];

    return NULL;
}

void TagPregList::addTagPreference(const QString& _name,
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

    layout()->addWidget(tpw);

    connect(tpw, SIGNAL(regexpChanged(const QString&)), this, SLOT(regexpChangedProxy(const QString&)));
    connect(tpw, SIGNAL(visibilityChanged(const QString&)), this, SLOT(visibilityChangedProxy(const QString&)));
    connect(tpw, SIGNAL(foldChanged(const QString&)), this, SLOT(foldChangedProxy(const QString&)));
    tp[_name] = tpw;
}

void TagPregList::addTagPreferenceShort(const QString& _name)
{
    if (tp.contains(_name))
        return;

    QColor color = (bgcolor.lightness() < 128) ? QColor(255, 255, 255) : QColor(0, 0, 0);
    QFont font = QFont();

    addTagPreference(_name, QString(), color.name(), font.toString(), true, true);

    emit elementChanged();
}

void TagPregList::deleteTagPreference(const QString& _name)
{
    if (!tp.contains(_name))
        return;

    QMap<QString, TagPreference*>::iterator it = tp.find(_name);

    layout()->removeWidget(it.value());
    it.value()->deleteLater();
    tp.erase(it);

    emit elementChanged();
}

void TagPregList::regexpChangedProxy(const QString& _text)
{
    emit regexpChanged(_text);
}

void TagPregList::visibilityChangedProxy(const QString& _text)
{
    emit visibilityChanged(_text);
}

void TagPregList::foldChangedProxy(const QString& _text)
{
    emit foldChanged(_text);
}

void TagPregList::setBackgroundColor(const QColor& _bgcolor)
{
    bgcolor = _bgcolor;
    emit sigSetBackgroundColor(_bgcolor);
}

void TagPregList::getVisibleTagPreferences(QStringList& _visible) const
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

void TagPregList::getTagPreferences(QStringList& _visible) const
{
    _visible.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
        _visible << it.value()->text();
    }
}

void TagPregList::getChangeableTagPreferences(QStringList& _changeable) const
{
    _changeable.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
      if (it.value()->getChangeable())
        _changeable << it.value()->text();
    }
}
