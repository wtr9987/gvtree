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
 * \brief CompareTree is a QTreeView widget
 *        to display the files of a git version or
 *        changed files between two or more git versions.
 */
class TagTree : public QTreeView
{
    Q_OBJECT
public:
    TagTree(QWidget* _parent = NULL);

    void setMainWindow(class MainWindow* _mwin);
    void setGraphWidget(class GraphWidget* _graph);

    // remove all current data
    void resetTagTree();

    void addData(const Version* _v);
    void compress(QStandardItem* _p = NULL);
    void updateSearchResult(QList<Version*>& _matches);

protected:
    void collectSubitems(const QModelIndex& _p, QList<Version*>& _collect);
    QStandardItemModel* treemodel;
    virtual void mousePressEvent (QMouseEvent* event);

public slots:
    // show context menu
    void onCustomContextMenu(const QPoint& point);

protected slots:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);


protected:
    class MainWindow* mwin;
    class GraphWidget* graph;
    QStandardItem* root;
};

#endif
