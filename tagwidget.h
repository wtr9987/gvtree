/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.3-0                */
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

#ifndef __TAGWIDGET_H__
#define __TAGWIDGET_H__

#include "taglist.h"

#include <QTabWidget>
#include <QWidget>
#include <QHash>
#include <QString>
#include <QStringList>

#include <iostream>

class TagWidget : public QTabWidget
{
    Q_OBJECT

public:
    TagWidget(class MainWindow* _parent = NULL);

    virtual void clear();

    void addData(const QMap<QString, QStringList>& _data);
    void setDefault();

public slots:
    void lookupId(QListWidgetItem* _item);
    void currentChanged(int _val);

private:
    QMap<QString, int> labelToIndex;
    class MainWindow* mwin;
    QString restoreTab;
};

#endif
