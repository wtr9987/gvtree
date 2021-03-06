/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.2-0                */
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

#include "tagwidget.h"
#include "mainwindow.h"

TagWidget::TagWidget(MainWindow* _parent) : QTabWidget(_parent), mwin(_parent)
{
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
}

void TagWidget::clear()
{
    labelToIndex.clear();
    QTabWidget::clear();
}

void TagWidget::addData(const QMap<QString, QStringList>& _data)
{
    for (QMap<QString, QStringList>::const_iterator it = _data.begin();
         it != _data.end();
         it++)
    {
        if (it.value().size() == 0)
            continue;
        if (it.key().at(0) == QChar('_'))
            continue;
        if (labelToIndex.find(it.key()) == labelToIndex.end())
        {
            TagList* tmp = new TagList(this);
            tmp->setObjectName(it.key());
            connect(tmp, SIGNAL(itemPressed(QListWidgetItem*)),
                    this, SLOT(lookupId(QListWidgetItem*)));
            tmp->addData(it.value());
            labelToIndex[it.key()] = addTab(tmp, it.key());
        }
        else
        {
            TagList* tmp = dynamic_cast<TagList*>(widget(labelToIndex[it.key()]));
            tmp->addData(it.value());
        }
    }
}

void TagWidget::lookupId(QListWidgetItem* _item)
{
    mwin->lookupId(_item->text(), true);
}

void TagWidget::setDefault()
{
    QSettings settings;

    int index = settings.contains("tagwidget/current") ?
        indexOf(findChild<QWidget*>(settings.value("tagwidget/current").toString())) : 0;

    if (index < 0)
    {
        index = 0;
        settings.setValue("tagwidget/current", QVariant(QString()));
    }

    setCurrentIndex(index);
}

void TagWidget::currentChanged(int _val)
{
    QSettings settings;

    settings.setValue("tagwidget/current", tabText(_val));
}
