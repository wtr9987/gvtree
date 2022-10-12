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

#ifndef __TAG_TREE_H__
#define __TAG_TREE_H__

#include <QTreeView>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QWidget>
#include <QAction>
#include <QString>

#include "version.h"

/**
 * \brief TagTree is a replacement for tagwidget/taglist.
 *        The tabs are now the top level of the tree representation.
 *        Search Result, HEAD, Release Label, Baseline Label, ...
 *        If a search pattern is entered, all matching versions are listed
 *        under the Search Result node.
 *        Under HEAD, Release Label, Baseline Label, ...
 *        the matching tag information is listed and one level below the
 *        matching versions.
 */
class TagTree : public QTreeView
{
    Q_OBJECT
public:
    TagTree(class GraphWidget* _graph, class MainWindow* _mwin);

    // reset/init tree
    void resetTagTree();

    // add information
    void addData(const Version* _v);

    // compress last tree level
    void compress(QStandardItem* _p = NULL);

    // import mathing versions from search dialog
    void updateSearchResult(const QString& _pattern, QList<Version*>& _matches);

protected:
    // construct tree
    QStandardItem* findOrInsert(QStandardItem* _p, const QString& _val);
    void insertLeaf(QStandardItem* _p, const QString& _timestamp, const Version* _v);

    // create a list of all selected version nodes
    void collectSubitems(const QModelIndex& _p, QList<Version*>& _collect);

    // block right button to allow context menu
    virtual void mousePressEvent (QMouseEvent* event);

public slots:
    // context menu to expand or collapse the whole tree
    void onCustomContextMenu(const QPoint& point);

protected slots:
    // focus selection in main view
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
    class GraphWidget* graph;
    class MainWindow* mwin;
    QStandardItemModel* treemodel;
    QStandardItem* root;
};

#endif
