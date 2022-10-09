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

TagTree::TagTree(QWidget* _parent) : QTreeView(_parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    treemodel = new QStandardItemModel(NULL);
    setModel(treemodel);

    resetTagTree();

#if QT_VERSION >= 0x51400
    header()->setSectionResizeMode(QHeaderView::Interactive);
#else
    header()->setResizeMode(QHeaderView::Interactive);
#endif

    show();
}

void TagTree::updateSearchResult(QList<Version*>& _matches)
{
    treemodel->removeRow(0, root->index());
    QStandardItem* search = new QStandardItem("Search");
    QList<QStandardItem*> sl;
    sl.push_back(search);
    treemodel->insertRow(0, sl);

    QStandardItem* p = root->child(0);

    foreach(Version* v, _matches)
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
    if (_p == NULL)
        _p = root;

    int rows = _p->rowCount();
    if (rows == 1)
    {
        if (_p->child(0, 0)->hasChildren() == false)
        {
            QStandardItem* c1 = _p->takeChild(0, 1);
            Version* v = c1 ? c1->data(Qt::UserRole + 1).value<VersionPointer>() : NULL;
            if (c1 && v)
            {
                _p->parent()->setChild(_p->row(), 1, c1);
                treemodel->removeRow(0, _p->index());
                return;
            }
        }
    }
    for (int i = 0; i < rows; i++)
    {
        if (_p->child(i))
            compress(_p->child(i));
    }
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

        QStandardItem* p = root;

        // level 1 : taginfo key
        bool skip = false;
        for (int i = 0; i < p->rowCount(); i++)
        {
            QStandardItem* t = p->child(i);
            if (t->text() == key)
            {
                p = t;
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            // std::cerr << "TODO new key information? " << key.toUtf8().data() << std::endl;
            continue;
        }

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
                bool skip = false;

                for (int i = 0; i < p->rowCount(); i++)
                {
                    QStandardItem* t = p->child(i);
                    if (t->text() == tmp)
                    {
                        p = t;
                        skip = true;
                        break;
                    }
                }
                if (skip == false)
                {
                    QStandardItem* t = new QStandardItem(tmp);
                    t->setEditable(false);
                    p->appendRow(t);
                    p = t;
                }
            }
        }
        else
        {
            // level 2 different values
            foreach(const QString &val, it.value())
            {
                bool skip = false;

                for (int i = 0; i < p->rowCount(); i++)
                {
                    QStandardItem* t = p->child(i);
                    if (t->text() == val)
                    {
                        p = t;
                        skip = true;
                        break;
                    }
                }
                if (skip == false)
                {
                    QStandardItem* t = new QStandardItem(val);
                    t->setEditable(false);
                    p->appendRow(t);
                    p = t;
                }
            }
        }

        // add leaf info...
        QList<QStandardItem*> columns;
        QStandardItem* t = new QStandardItem("");
        t->setEditable(false);
        columns << t;
        t = new QStandardItem(timestamp);
        t->setData(QVariant::fromValue(VersionPointer(_v)), Qt::UserRole + 1);
        t->setEditable(false);
        columns << t;
        p->appendRow(columns);
    }
}

void TagTree::resetTagTree()
{
    if (!treemodel)
        return;

    treemodel->clear();

    treemodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Tag Information")));
    treemodel->setHorizontalHeaderItem(1, new QStandardItem(QString("Commit Date")));
    // treemodel->setHorizontalHeaderItem(2, new QStandardItem(QString("Hash")));

    root = treemodel->invisibleRootItem();
    root->setEditable(false);
    // nodeInfo << "HEAD" << "Commit Date" << "User Name" << "Hash" << "Branch" << "Release Label" << "Baseline Label" << "FIX/PQT Label" << "HO Label" << "Other Tags" << "Comment";
    nodeInfo << "Search"
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

void TagTree::setMainWindow(class MainWindow* _mwin)
{
    mwin = _mwin;
}

void TagTree::setGraphWidget(class GraphWidget* _graph)
{
    graph = _graph;
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
