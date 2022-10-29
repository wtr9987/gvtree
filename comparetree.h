/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.4-0                */
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

#ifndef __COMPARE_TREE_H__
#define __COMPARE_TREE_H__

#include <QTreeView>
#include <QStandardItemModel>
#include <QWidget>
#include <QAction>
#include <QString>

/**
 * \brief CompareTree is a QTreeView widget
 *        to display the files of a git version or
 *        changed files between two or more git versions.
 */
class CompareTree : public QTreeView
{
    Q_OBJECT
public:
    CompareTree(QWidget* _parent = NULL);

    void setMainWindow(class MainWindow* _mwin);
    void setGraphWidget(class GraphWidget* _graph);

    // show file tree of the given version
    void viewThisVersion(const QString& _hash);

    // show the local changes
    void viewLocalChanges(bool _staged);

    // show differences of the fromVersions and the toVersion in an editor
    void compareFileVersions(const QString& _path, const QString& _status, const QString& _path_old);

    // open a selected file in an editor
    void editCurrentVersion(const QString& _path);

    // compare hash to local (modified files)
    void compareHashToLocal(const QString& _hash);

    // compare versions
    void compareHashes(const QStringList& _hash1, const QString& _hash2);

    // remove all current data
    void resetCompareTree();

    // if file constraint is set only the path to the file is exapanded,
    // else everything up to level 10
    void expandTree();


protected:
    // called by compareFileVersions to create temporary files
    QString createTempVersionFile(const QString& _hash, const QString& _path) const;
    // get mime type
    QString getMimeType(const QString& _path) const;
    // get file extension
    QString getFileExtension(const QString& _path) const;

public slots:
    // update commit info displayed in the text edit
    void currentIndexChanged(int _index);

    // show context menu
    void onCustomContextMenu(const QPoint& point);
    // context menu actions
    // version diff
    void compareFileVersionsAction(QAction* _act);
    // version edit/show
    void editCurrentVersionAction(QAction* _act);
    // setting/removing file constraint to version tree
    void gitLogFileAction(QAction* _act);

protected:
    class MainWindow* mwin;
    class GraphWidget* graph;
};

#endif
