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

#ifndef __TAGPREFGRIDLAYOUT_H__
#define __TAGPREFGRIDLAYOUT_H__

#include <QVBoxLayout>
#include <QWidget>
#include <QString>
#include "tagpreference.h"

class TagPrefGridLayout : public QWidget
{
    Q_OBJECT
public:
    TagPrefGridLayout(QWidget* _parent = NULL);

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
    void regexpChangedProxy();
    void visibilityChangedProxy();
    void deleteTagPreference(const QString& _name);
    void addTagPreferenceShort(const QString& _name);

signals:
    void regexpChanged();
    void visibilityChanged();
    void elementChanged();
    void sigSetBackgroundColor(const QColor&);
};

#endif
