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

#ifndef __TAGPREFLIST_H__
#define __TAGPREFLIST_H__

#include <QVBoxLayout>
#include <QWidget>
#include <QString>
#include "tagpreference.h"

class TagPrefList : public QWidget
{
    Q_OBJECT
public:
    TagPrefList(QWidget* _parent = NULL);

    void addTagPreference(const QString& _name,
                          const QString& _regexp,
                          const QString& _color = QString(),
                          const QString& _font = QString(),
                          bool _regexpChangable = false,
                          int _fold = -1);

    const TagPreference* getTagPreference(const QString& _item) const;
    void getVisibleTagPreferences(QStringList& _visible) const;
    void getTagPreferences(QStringList& _all) const;
    void getChangeableTagPreferences(QStringList& _changeable) const;

protected:
    QColor bgcolor;
    QMap<QString, TagPreference*> tp;

public slots:
    void setBackgroundColor(const QColor& _bgcolor);
    void regexpChangedProxy(const QString&);
    void visibilityChangedProxy(const QString&);
    void foldChangedProxy(const QString&);
    void deleteTagPreference(const QString& _name);
    void addTagPreferenceShort(const QString& _name);
    void moveUp(TagPreference* _item);
    void moveDown(TagPreference* _item);

signals:
    void regexpChanged(const QString&);
    void visibilityChanged(const QString&);
    void foldChanged(const QString&);
    void elementChanged();
    void sigSetBackgroundColor(const QColor&);
};

#endif