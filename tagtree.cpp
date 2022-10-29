/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.4-0                */
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
#include <QAction>
#include "tagtree.h"
#include "mainwindow.h"
#include "graphwidget.h"

using namespace std;

TagTree::TagTree(GraphWidget* _graph, MainWindow* _mwin) : QTreeView(_mwin),
    graph(_graph),
    mwin(_mwin),
    treemodel(NULL),
    root(NULL),
    search(NULL)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    treemodel = new QStandardItemModel(NULL);
    setModel(treemodel);
    setSortingEnabled(true);

    resetTagTree();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    header()->setResizeMode(QHeaderView::Stretch);
#endif

    show();
}

void TagTree::updateSearchResult(QList<Version*>& _matches)
{
    // Remove nodes under "Search Result" node
    QStandardItem* p = NULL;

    for (int i = 0; i < root->rowCount(); i++)
    {
        if (root->child(i)->text() == "Search Result")
        {
            p = root->child(i);
            break;
        }
    }

    if (!p)
    {
        // error
        return;
    }

    while (p->rowCount() > 0)
    {
        treemodel->removeRow(p->rowCount() - 1, p->index());
    }

    // Insert matches from search dialog
    foreach(Version * v, _matches)
    {
        QStandardItem* c1 = new QStandardItem("");

        c1->setIcon(QIcon(":/images/gvt_dot.png"));
        c1->setEditable(false);

        QStandardItem* c2 = new QStandardItem(v->getKeyInformation().find("Commit Date").value().join(" "));

        c2->setData(QVariant::fromValue(VersionPointer(v)), Qt::UserRole + 1);
        c2->setEditable(false);

        QList<QStandardItem*> row = QList<QStandardItem*>() << c1 << c2;

        p->appendRow(row);
    }

    // Set folder symbol if needed
    p->setIcon(_matches.size() ? QIcon().fromTheme("folder") : QIcon());

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
            _p->setIcon(QIcon(":/images/gvt_dot.png"));
            _p->parent()->setChild(_p->row(), 1, c);
            treemodel->removeRow(0, _p->index());
            return;
        }
        else
        {
            _p->setIcon(QIcon().fromTheme("folder"));
        }
    }

    if (_p->rowCount() > 1)
    {
        _p->setIcon(QIcon().fromTheme("folder"));
        for (int i = 0; i < _p->rowCount(); i++)
        {
            QStandardItem* c0 = _p->child(i, 0);
            QStandardItem* c1 = _p->child(i, 1);
            if (c1 && c1->data(Qt::UserRole + 1).value<VersionPointer>()
                && (c0->text() == c1->text()))
            {
                c1->setText("");
            }
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
        QStandardItem* p = findOrInsert(root, key, false);

        if (key == "Commit Date")
        {
            QString year = timestamp.mid(0, 4);
            QString month = timestamp.mid(5, 2);
            QString day = timestamp.mid(8, 2);
            QString daytime = timestamp.mid(11, 8);

            QStringList ts = QStringList() << year << month << day << daytime;

            foreach(const QString& tmp, ts)
            {
                p = findOrInsert(p, tmp);
            }

            insertLeaf(p, timestamp, _v);
        }
        else
        {
            foreach(const QString& val, it.value())
            {
                insertLeaf(findOrInsert(p, val), timestamp, _v);
            }
        }
    }
}

QStandardItem* TagTree::findOrInsert(QStandardItem* _p, const QString& _val, bool _sort)
{
    int i = 0;

    for (; i < _p->rowCount(); i++)
    {
        if (_p->child(i)->text() == _val)
            return _p->child(i);

        if (_sort && _p->child(i)->text() > _val)
            break;
    }

    QStandardItem* t = new QStandardItem(_val);

    t->setEditable(false);
    if (_sort)
        _p->insertRow(i, t);
    else
        _p->appendRow(t);
    return t;
}

void TagTree::insertLeaf(QStandardItem* _p, const QString& _timestamp, const Version* _v)
{
    QList<QStandardItem*> columns;

    columns << new QStandardItem(_timestamp) << new QStandardItem(_timestamp);
    columns.front()->setEditable(false);
    columns.front()->setIcon(QIcon(":/images/gvt_dot.png"));
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
    treemodel->setHorizontalHeaderItem(1, new QStandardItem(QString("")));

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

    foreach(const QString& key, nodeInfo)
    {
        QStandardItem* t = new QStandardItem(key);

        t->setEditable(false);
        root->appendRow(t);
    }

    search = new QLineEdit(this);
    connect(search, SIGNAL(textEdited(const QString&)), this, SLOT(lookupId(const QString&)));
    connect(search, SIGNAL(returnPressed()), this, SLOT(focusGraph()));
    setIndexWidget(treemodel->index(0, 1), search);
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

    graph->resetMatches();

    const QModelIndexList& sel = selected.indexes();

    foreach (const QModelIndex& idx, sel)
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

void TagTree::collectSubitems(const QModelIndex& _p, QList<Version*>& _collect)
{
    // check for version
    QModelIndex sib = _p.sibling(_p.row(), 1);

    if (sib.isValid())
    {
        Version* v = sib.data(Qt::UserRole + 1).value<VersionPointer>();
        if (v)
        {
            QString arg = _p.parent().data(Qt::DisplayRole).toString();
            if (arg == "Search Result")
            {
                QRegExp pattern(search->text());
                v->findMatch(pattern, search->text(), false);
            }
            else
            {
                v->addLocalVersionInfo(arg);
            }
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

void TagTree::lookupId(const QString& _text, bool _exactMatch)
{
    QString pattern = _text;

    QList<Version*> matches;

    if (pattern.size() < 3)
    {
        graph->resetMatches();
        pattern = QString("");
    }
    else
    {
        graph->matchVersions(pattern, matches, _exactMatch);

        if (matches.size())
            graph->focusElements(matches);
    }
    updateSearchResult(matches);

    // keep focus
    search->setFocus();
}

void TagTree::focusGraph()
{
    graph->setFocus();
}

QLineEdit* TagTree::getSearch()
{
    return search;
}
