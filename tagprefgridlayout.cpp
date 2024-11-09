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
// TODO save/load rules to QSettings

#include "tagprefgridlayout.h"

TagPrefGridLayout::TagPrefGridLayout(QWidget* _parent) : QWidget(_parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    setLayout(layout);
}

const TagPreference* TagPrefGridLayout::getTagPreference(const QString& _item) const
{
    if (tp.contains(_item))
        return tp[_item];

    return NULL;
}

void TagPrefGridLayout::addTagPreference(const QString& _name,
                                         const QString& _regexp,
                                         const QString& _color,
                                         const QString& _font,
                                         bool _regexpChangeable)
{
    if (tp.contains(_name))
        return;

    TagPreference* tpw = new TagPreference(_name, _regexp, _color, _font, bgcolor, true, _regexpChangeable, this);

    connect(this, SIGNAL(sigSetBackgroundColor(const QColor&)), tpw, SLOT(setBackgroundColor(const QColor&)));

    connect(tpw, SIGNAL(deleteTagPreference(const QString&)), this, SLOT(deleteTagPreference(const QString&)));
    connect(tpw, SIGNAL(addTagPreference(const QString&)), this, SLOT(addTagPreferenceShort(const QString&)));

    layout()->addWidget(tpw);

    connect(tpw, SIGNAL(regexpChanged()), this, SLOT(regexpChangedProxy()));
    connect(tpw, SIGNAL(visibilityChanged()), this, SLOT(visibilityChangedProxy()));
    tp[_name] = tpw;
}

void TagPrefGridLayout::addTagPreferenceShort(const QString& _name)
{
    if (tp.contains(_name))
        return;

    QColor color = (bgcolor.lightness() < 128) ? QColor(255, 255, 255) : QColor(0, 0, 0);
    QFont font = QFont();

    addTagPreference(_name, QString(), color.name(), font.toString(), true);

    emit elementChanged();
}

void TagPrefGridLayout::deleteTagPreference(const QString& _name)
{
    if (!tp.contains(_name))
        return;

    QMap<QString, TagPreference*>::iterator it = tp.find(_name);

    layout()->removeWidget(it.value());
    it.value()->deleteLater();
    tp.erase(it);

    emit elementChanged();
}

void TagPrefGridLayout::regexpChangedProxy()
{
    emit regexpChanged();
}

void TagPrefGridLayout::visibilityChangedProxy()
{
    emit visibilityChanged();
}

void TagPrefGridLayout::setBackgroundColor(const QColor& _bgcolor)
{
    bgcolor = _bgcolor;
    emit sigSetBackgroundColor(_bgcolor);
}

void TagPrefGridLayout::getVisibleTagPreferences(QStringList& _visible) const
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

void TagPrefGridLayout::getTagPreferences(QStringList& _visible) const
{
    _visible.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
        _visible << it.value()->text();
    }
}

void TagPrefGridLayout::getChangeableTagPreferences(QStringList& _changeable) const
{
    _changeable.clear();
    for (QMap<QString, TagPreference*>::const_iterator it = tp.begin(); it != tp.end(); it++)
    {
      if (it.value()->getChangeable())
        _changeable << it.value()->text();
    }
}
