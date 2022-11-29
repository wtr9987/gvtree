/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.5-0                */
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

#include <iostream>
#include "mimetable.h"

MimeTable::MimeTable(QWidget* _parent) : QTableWidget(_parent)
{
    setColumnCount(3);
    QStringList header;
    header << "mime-type" << "diff tool" << "edit tool";
    setHorizontalHeaderLabels(header);
    verticalHeader()->hide();

    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setHighlightSections(false);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSectionsClickable(false);
#else
    horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setClickable(false);
#endif
}

void MimeTable::fromSettings()
{
    QSettings settings;

    QMap<QString, QString> difftoolMimeTypes;
    QMap<QString, QString> edittoolMimeTypes;

    settings.beginGroup("difftoolMimeTypes");
    QStringList keys = settings.allKeys();
    foreach(const QString &key, keys)
    {
        difftoolMimeTypes[key] = settings.value(key).toString();
    }
    settings.endGroup();

    settings.beginGroup("edittoolMimeTypes");
    keys = settings.allKeys();
    foreach(const QString &key, keys)
    {
        edittoolMimeTypes[key] = settings.value(key).toString();
    }
    settings.endGroup();

    for (QMap<QString, QString>::iterator it = difftoolMimeTypes.begin();
         it != difftoolMimeTypes.end();
         it++)
    {
        insert(it.key(), it.value(), edittoolMimeTypes[it.key()]);
    }

    connect(this,
            SIGNAL(itemChanged(QTableWidgetItem*)),
            this,
            SLOT(itemChanged(QTableWidgetItem*)));
}

void MimeTable::insert(const QString& _mimetype, const QString& _diff, const QString& _edit)
{
    disconnect(this,
               SIGNAL(itemChanged(QTableWidgetItem*)),
               this,
               SLOT(itemChanged(QTableWidgetItem*)));
    insertRow(rowCount());
    QTableWidgetItem* item = new QTableWidgetItem(_mimetype);
    item->setFlags(Qt::NoItemFlags);
    setItem(rowCount() - 1, 0, item);
    setItem(rowCount() - 1, 1, new QTableWidgetItem(_diff));
    setItem(rowCount() - 1, 2, new QTableWidgetItem(_edit));
    QString path = "difftoolMimeTypes/" + _mimetype;

    QSettings settings;

    settings.setValue(path, _diff);
    path = "edittoolMimeTypes/" + _mimetype;
    settings.setValue(path, _edit);

    connect(this,
            SIGNAL(itemChanged(QTableWidgetItem*)),
            this,
            SLOT(itemChanged(QTableWidgetItem*)));
}

bool MimeTable::get(const QString& _mimetype, QString& _diff, QString& _edit)
{
    for (int i = 0; i < rowCount(); i++)
    {
        if (item(i, 0)->text() == _mimetype)
        {
            _diff = item(i, 1)->text();
            _edit = item(i, 2)->text();
            return true;
        }
    }
    return false;
}

void MimeTable::itemChanged(QTableWidgetItem* _item)
{
    int r = _item->row();

    QString mimetype = item(r, 0)->text();
    QString difftool = item(r, 1)->text();
    QString edittool = item(r, 2)->text();

    QString path = "difftoolMimeTypes/" + mimetype;

    QSettings settings;

    settings.setValue(path, difftool);
    path = "edittoolMimeTypes/" + mimetype;
    settings.setValue(path, edittool);
}
