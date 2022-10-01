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

#ifndef __MIMETABLE_H__
#define __MIMETABLE_H__

#include <QTableWidget>
#include <QSettings>
#include <QMap>
#include <QString>
#include <QHeaderView>

class MimeTable : public QTableWidget
{
    Q_OBJECT

public:
    MimeTable(QWidget* _parent = NULL);

    void fromSettings();

    void insert(const QString& _mimetype, const QString& _diff, const QString& _edit);

    bool get(const QString& _mimetype, QString& _diff, QString& _edit);

public slots:
    void itemChanged(QTableWidgetItem* _item);
};

#endif
