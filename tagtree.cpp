/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.3-0                */
/*                                               */
/*             git version tree browser          */
/*                                               */
/*   9. October 2022                             */
/*                                               */
/*         This program is licensed under        */
/*           GNU GENERAL PUBLIC LICENSE          */
/*            Version 3, 29 June 2007            */
/*                                               */
/* --------------------------------------------- */

#include <iostream>

#include <QMenu>
#include <QFileInfo>
#include "tagtree.h"
#include "mainwindow.h"
#include "graphwidget.h"
#include <unistd.h>

using namespace std;

TagTree::TagTree(GraphWidget* _graph, MainWindow* _mwin) : QTreeView(_mwin), graph(_graph), mwin(_mwin), treemodel(NULL), root(NULL)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    treemodel = new QStandardItemModel(NULL);
    setModel(treemodel);

    resetTagTree();

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    header()->setResizeMode(QHeaderView::Stretch);
#endif

    show();
}

void TagTree::updateSearchResult(QList<Version*>& _matches)
{
    // remove "Search Result" subttree
    treemodel->removeRow(0, root->index());

    // insert empty "Search Result" node.
    QStandardItem* search = new QStandardItem("Search Result");
    QList<QStandardItem*> sl;
    sl.push_back(search);
    treemodel->insertRow(0, sl);

    QStandardItem* p = root->child(0);

    // Insert matches from search dialog
    foreach(Version * v, _matches)
    {
        QList<QStandardItem*> columns;
        QString timestamp = v->getKeyInformation().find("Commit Date").value().join(" ");

        QStandardItem* t = new QStandardItem(timestamp);
        t->setEditable(false);
        columns << t;
        t = new QStandardItem(timestamp);
        t->setData(QVariant::fromValue(VersionPointer(v)), Qt::UserRole + 1);
        t->setEditable(false);
        columns << t;
        p->appendRow(columns);
    }

    // display search results
    expand(p->index());
}

void TagTree::compress(QStandardItem* _p)
{
    _p = (_p == NULL) ? root : _p;

    if (_p->rowCount() == 1 && !_p->child(0, 0)->hasChildren())
    {
        QStandardItem* c = _p->takeChild(0, 1);
        if (c && c->data(Qt::UserRole + 1).value<VersionPointer>() && _p->parent())
        {
            _p->parent()->setChild(_p->row(), 1, c);
            treemodel->removeRow(0, _p->index());
            return;
        }
    }

    for (int i = 0; i < _p->rowCount(); i++)
        compress(_p->child(i));
}

void TagTree::addData(const Version* _v)
{
    QString timestamp;

    QMap<QString, QStringList>::const_iterator dt = _v->getKeyInformation().find("Commit Date");
    if (dt != _v->getKeyInformation().end())
        timestamp = dt.value().join(" ");

    for (QMap<QString, QStringList>::const_iterator it = _v->getKeyInformation().begin();
         it != _v->getKeyInformation().end();
         it++)
    {
        QString key = it.key();
        if (key == "_input" || key == "Comment")
            continue;
        else if (key == "CommentRaw")
            key = "Comment";

        // level 1 : taginfo key
        QStandardItem* p = findOrInsert(root, key);

        if (key == "Commit Date")
        {
            QString year = timestamp.mid(0, 4);
            QString month = timestamp.mid(5, 2);
            QString day = timestamp.mid(8, 2);
            QString daytime = timestamp.mid(11, 8);

            QStringList ts;
            ts << year << month << day << daytime;

            foreach(const QString &tmp, ts)
            {
                p = findOrInsert(p, tmp);
            }

            insertLeaf(p, timestamp, _v);
        }
        else
        {
            foreach(const QString &val, it.value())
            {
                insertLeaf(findOrInsert(p, val), timestamp, _v);
            }
        }
    }
}

QStandardItem* TagTree::findOrInsert(QStandardItem* _p, const QString& _val)
{
    for (int i = 0; i < _p->rowCount(); i++)
    {
        if (_p->child(i)->text() == _val)
            return _p->child(i);
    }

    QStandardItem* t = new QStandardItem(_val);
    t->setEditable(false);
    _p->appendRow(t);
    return t;
}

void TagTree::insertLeaf(QStandardItem* _p, const QString& _timestamp, const Version* _v)
{
    QList<QStandardItem*> columns;
    columns << new QStandardItem("") << new QStandardItem(_timestamp);
    columns.back()->setData(QVariant::fromValue(VersionPointer(_v)), Qt::UserRole + 1);
    columns.back()->setEditable(false);
    _p->appendRow(columns);
}

void TagTree::resetTagTree()
{
    if (!treemodel)
        return;

    treemodel->clear();

    treemodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Tag Information")));
    treemodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Commit Date")));

    root = treemodel->invisibleRootItem();
    root->setEditable(false);

    QStringList nodeInfo;
    nodeInfo << "Search Result"
             << "HEAD"
             << "Release Label"
             << "Baseline Label"
             << "Commit Date"
             << "Branch"
             << "FIX/PQT Label"
             << "HO Label"
             << "Other Tags"
             << "User Name" // ... many versions
             << "Comment"  // ... don't know if useful, maby not
             << "Hash"; // ... not useful, use search instead

    foreach(const QString &key, nodeInfo)
    {
        QStandardItem* t = new QStandardItem(key);

        t->setEditable(false);
        root->appendRow(t);
    }
}

void TagTree::onCustomContextMenu(const QPoint& point)
{
    QMenu* menu = new QMenu(this);
    QAction* act = new QAction("Collapse all", this);

    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(collapseAll()));

    act = new QAction("Expand all", this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(expandAll()));

    menu->exec(viewport()->mapToGlobal(point));
}

void TagTree::mousePressEvent (QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        event->accept();
    }
    else
        QTreeView::mousePressEvent(event);
}

void TagTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    const QModelIndexList& sel = selected.indexes();

    if (sel.size() == 0)
    {
        graph->resetMatches();
    }
    else
    {
        foreach (const QModelIndex &idx, sel)
        {
            if (idx.column() == 0)
            {
                QList<Version*> collect;
                collectSubitems(idx, collect);
                graph->focusElements(collect);
                break;
            }
        }
    }
}

void TagTree::collectSubitems(const QModelIndex& _p, QList<Version*>& _collect)
{
    // check for version
    QModelIndex sib = _p.sibling(_p.row(), 1);

    if (sib.isValid())
    {
        Version* v = sib.data(Qt::UserRole + 1).value<VersionPointer>();
        if (v)
        {
            _collect.push_back(v);
            return;
        }
    }

    //
    int rows = treemodel->rowCount(_p);

    for (int i = 0; i < rows; i++)
    {
        collectSubitems(treemodel->index(i, 0, _p), _collect);
    }
}
