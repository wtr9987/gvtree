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

#ifndef __TAGLIST_H__
#define __TAGLIST_H__

#include <QListWidget>
#include <QTabWidget>
#include <QSet>
#include <QStringList>
#include <QWidget>
#include <QString>

class TagList : public QListWidget
{
    Q_OBJECT

public:
    TagList(QWidget* _parent = NULL);

    void addData(const QStringList& _data);

    virtual void clear();

private:
    QSet<QString> raw;
};

#endif
