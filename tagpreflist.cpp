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
#include <QSettings>

TagPrefList::TagPrefList(QWidget* _parent) : QWidget(_parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Maximum));

    setLayout(layout);
}

const TagPreference* TagPrefList::getTagPreference(const QString& _item) const
{
    return (tp.contains(_item)) ? tp[_item] : NULL;
}

bool TagPrefList::addTagPreference(const QString& _name,
                                   const QString& _regexp,
                                   const QString& _color,
                                   const QString& _font,
                                   bool _visibility,
                                   bool _regexpChangeable,
                                   int _fold)
{
    QVBoxLayout* l = dynamic_cast<QVBoxLayout*>(layout());

    if (l && !tp.contains(_name))
    {
        TagPreference* tpw = new TagPreference(_name, _regexp, _color, _font, bgcolor, _visibility, _regexpChangeable, _fold, this);
        if (tpw)
        {
            connect(tpw, SIGNAL(deleteTagPreference(const QString&)), this, SLOT(deleteTagPreference(const QString&)));
            connect(tpw, SIGNAL(addTagPreference(const QString&)), this, SLOT(addTagPreferenceShort(const QString&)));
            connect(tpw, SIGNAL(regexpChanged(const QString&)), this, SLOT(regexpChangedProxy(const QString&)));
            connect(tpw, SIGNAL(visibilityChanged(const QString&)), this, SLOT(visibilityChangedProxy(const QString&)));
            connect(tpw, SIGNAL(moveUp(TagPreference*)), this, SLOT(moveUp(TagPreference*)));
            connect(tpw, SIGNAL(moveDown(TagPreference*)), this, SLOT(moveDown(TagPreference*)));

            tp[_name] = tpw;

            l->insertWidget(layout()->count() - 1, tpw);

            updateSettings();
            return true;
        }
    }
    return false;
}

void TagPrefList::updateSettings()
{
    QSettings settings;
    QStringList items;

    getVisibleTagPreferences(items);
    settings.setValue("tagprefs", QVariant::fromValue(items));
}

void TagPrefList::addTagPreferenceShort(const QString& _name)
{
    QColor color = (bgcolor.lightness() < 128) ? QColor(255, 255, 255) : QColor(0, 0, 0);

    if (addTagPreference(_name, QString(), color.name(), QFont().toString(), true, true))
    {
        emit elementChanged();
    }
}

void TagPrefList::deleteTagPreference(const QString& _name)
{
    if (tp.contains(_name))
    {
        QMap<QString, TagPreference*>::iterator it = tp.find(_name);

        layout()->removeWidget(it.value());
        it.value()->deleteLater();
        tp.erase(it);

        updateSettings();

        emit elementChanged();
    }
}

void TagPrefList::regexpChangedProxy(const QString& )
{
        emit elementChanged();
}

void TagPrefList::visibilityChangedProxy(const QString& _text)
{
    emit visibilityChanged(_text);
}

void TagPrefList::setBackgroundColor(const QColor& _bgcolor)
{
    bgcolor = _bgcolor;

    for (QMap<QString, TagPreference*>::iterator it = tp.begin(); it != tp.end(); it++)
    {
        it.value()->setBackgroundColor(_bgcolor);
    }
}

void TagPrefList::getOrderedTagPreferences(QList<TagPreference*>& _list) const
{
    _list.clear();
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());

    if (layout)
    {
        for (int i = 0; i < layout->count() - 1; i++)
        {
            TagPreference* t = dynamic_cast<TagPreference*>(layout->itemAt(i)->widget());
            if (t)
            {
                _list << t;
            }
        }
    }
}

void TagPrefList::getVisibleTagPreferences(QStringList& _visible) const
{
    _visible.clear();
    QList<TagPreference*> ordered;

    getOrderedTagPreferences(ordered);
    foreach(TagPreference * t, ordered)
    {
        if (t->getVisibility())
        {
            _visible << t->text();
        }
    }
}

void TagPrefList::getTagPreferences(QStringList& _all) const
{
    _all.clear();
    QList<TagPreference*> ordered;

    getOrderedTagPreferences(ordered);
    foreach(TagPreference * t, ordered)
    {
        _all << t->text();
    }
}

void TagPrefList::getChangeableTagPreferences(QStringList& _changeable) const
{
    _changeable.clear();
    QList<TagPreference*> ordered;

    getOrderedTagPreferences(ordered);
    foreach(TagPreference * t, ordered)
    {
        if (t->getChangeable())
        {
            _changeable << t->text();
        }
    }
}

int TagPrefList::getIndex(QVBoxLayout* _layout, TagPreference* _item) const
{
    for (int i = 0; _layout && i < _layout->count(); i++)
    {
        if (_item == _layout->itemAt(i)->widget())
        {
            return i;
        }
    }
    return -1;
}

void TagPrefList::moveUp(TagPreference* _item)
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());

    int index = getIndex(layout, _item);

    if (!(index == -1 || index == 0))
    {
        QLayoutItem* item1 = layout->takeAt(index);

        layout->insertItem(index - 1, item1);
        layout->update();

        updateSettings();

        emit visibilityChanged(QString());
    }
}

void TagPrefList::moveDown(TagPreference* _item)
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(this->layout());

    int index = getIndex(layout, _item);

    if (!(index == -1 || (index >= layout->count() - 2)))
    {
        QLayoutItem* item1 = layout->takeAt(index);

        layout->insertItem(index + 1, item1);
        layout->update();

        updateSettings();

        emit visibilityChanged(QString());
    }
}
