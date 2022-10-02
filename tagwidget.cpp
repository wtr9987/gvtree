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
    foreach(const QString &info, mwin->getNodeInfo())
    {
        TagList* tmp = new TagList(this);

        tmp->setObjectName(info);
        connect(tmp, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(lookupId(QListWidgetItem*)));
        labelToIndex[info] = addTab(tmp, info);
    }
}

void TagWidget::addData(const QMap<QString, QStringList>& _data)
{
    for (QMap<QString, QStringList>::const_iterator it = _data.begin();
         it != _data.end();
         it++)
    {
        if ((it.value().size() == 0)
            || (it.key().at(0) == QChar('_'))
            || (it.key() == QString("Comment"))
           )
            continue;

        // TODO improve "CommentRaw"/"Comment" processing.
        // Without this replacement, only "Comment" fragments
        // (separated lines) appear in the list view. 
        // With "CommentRaw" the complete checkin comment is
        // contained in the list.
        QString key = (it.key() == QString("CommentRaw")) ? QString("Comment") : it.key();

        if (labelToIndex.find(key) != labelToIndex.end())
        {
            TagList* tmp = dynamic_cast<TagList*>(widget(labelToIndex[key]));
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
