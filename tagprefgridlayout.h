/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.8-0                */
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

#include <QWidget>
#include <QGridLayout>
#include <QString>
#include "tagpreference.h"

class TagPrefGridLayout : public QGridLayout
{
    Q_OBJECT
public:
    TagPrefGridLayout(QWidget* _parent = NULL);

    void addTagPreference(const QString& _name, const QString& _regexp, bool _regexpChangable);

    const TagPreference* getTagPreference(const QString& _item) const;

protected:
    int line;
    QVBoxLayout* cl;
    QMap<QString, TagPreference*> tp;

public slots:
    void regexpChangedProxy();

signals:
    void regexpChanged();
};

#endif
