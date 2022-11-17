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

#include <QMenu>
#include <QFileInfo>
#include "comparetree.h"
#include "execute_cmd.h"
#include "mainwindow.h"
#include "graphwidget.h"
#include <unistd.h>

using namespace std;

CompareTree::CompareTree(QWidget* _parent) : QTreeView(_parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));
}

void CompareTree::compareHashes(const QStringList& _hash1, const QString& _hash2)
{
    QStandardItemModel* treemodel = new QStandardItemModel(NULL);

    treemodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Path")));
    treemodel->setHorizontalHeaderItem(1, new QStandardItem(QString("File")));

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    header()->setResizeMode(QHeaderView::Stretch);
#endif

    QStandardItem* root = treemodel->invisibleRootItem();
    root->setEditable(false);

    // get changed files
    QList<QString> cache;
    foreach(QString it, _hash1)
    {
        QString cmd = "git -C " + graph->getLocalRepositoryPath() + " diff " + it + ".." + _hash2 + " --name-status";

        execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());
    }

    // now copy the path information into a nice tree widget, duplicates are removed
    for (QStringList::iterator jt = cache.begin();
         jt != cache.end();
         ++jt)
    {
        QString info = (*jt).trimmed();

        QRegExp renameNameStatusPattern("(R)[0-9]*\\s+(.*)\\s+(.*)$");
        QString status;
        QString path_old;
        QString path;

        if (renameNameStatusPattern.indexIn(info, 0) != -1)
        {
            status = renameNameStatusPattern.cap(1);
            path_old = renameNameStatusPattern.cap(2);
            path = renameNameStatusPattern.cap(3);
        }
        else
        {
            QRegExp diffNameStatusPattern("(.)\\s+(.*)$");
            if (diffNameStatusPattern.indexIn(info, 0) != -1)
            {
                status = diffNameStatusPattern.cap(1);
                path = diffNameStatusPattern.cap(2);
            }
            else
            {
                continue;
            }
        }

        // split file path into a QStringList
        QStringList path_elements = path.split(QChar('/'));

        // start with root node...
        QStandardItem* p = root;

        for (QStringList::iterator it = path_elements.begin();
             it != path_elements.end();
             ++it)
        {
            // check all childs if the entry already exists
            bool skip = false;
            for (int i = 0; i < p->rowCount(); i++)
            {
                QStandardItem* t = p->child(i);
                if (t->text() == *it)
                {
                    p = t;
                    skip = true;
                    break;
                }
            }

            if (skip == false)
            {
                QList<QStandardItem*> columns;
                // add new node under p
                QStandardItem* t = new QStandardItem(*it);
                t->setEditable(false);

                QStringList::iterator kt = it;
                ++kt;
                if (kt == path_elements.end())
                {
                    // A added
                    // D deleted
                    // M modified
                    // R renamed
                    if (status == "A")
                        t->setIcon(QIcon(":/images/gvt_add.png"));
                    else if (status == "D")
                        t->setIcon(QIcon(":/images/gvt_del.png"));
                    else if (status == "M")
                        t->setIcon(QIcon(":/images/gvt_mod.png"));
                    else // if (status == "R")
                        t->setIcon(QIcon(":/images/gvt_ren.png"));
                    columns << t;
                    QStandardItem* actitem = new QStandardItem(path);
                    actitem->setEditable(false);
                    actitem->setData(status, Qt::UserRole + 1);
                    actitem->setData(path_old, Qt::UserRole + 2);
                    columns << actitem;
                    p->appendRow(columns);
                }
                else
                {
                    t->setIcon(QIcon().fromTheme("folder")); // folder-open
                    p->appendRow(t);
                    p = t;
                }
            }
        }
    }
    setModel(treemodel);
    expandTree();

    show();
}

void CompareTree::expandTree()
{
    if (graph->getFileConstraint().isEmpty())
    {
        expandToDepth(10);
    }
    else
    {
        QStringList path_elements = graph->getFileConstraint().split(QChar('/'));

        QStandardItemModel* treemodel = dynamic_cast<QStandardItemModel*>(model());
        if (!treemodel)
        {
            return;
        }

        QStandardItem* p = treemodel->invisibleRootItem();

        collapseAll();

        for (QStringList::iterator it = path_elements.begin();
             it != path_elements.end();
             ++it)
        {

            for (int i = 0; i < p->rowCount(); i++)
            {
                QStandardItem* t = p->child(i);
                if (t->text() == *it)
                {
                    p = t;
                    setExpanded(p->index(), true);
                    break;
                }
            }
        }
        scrollTo(p->index());
        selectionModel()->select(p->index(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void CompareTree::resetCompareTree()
{
    QStandardItemModel* treemodel = dynamic_cast<QStandardItemModel*>(model());

    if (treemodel)
        treemodel->clear();

    mwin->getCompareTreeFromTextEdit()->clear();
    mwin->getFromComboBox()->clear();
    mwin->getCompareTreeToTextEdit()->clear();
    mwin->getToDateLabel()->setText(QString());
    mwin->getCompareTreeFromPushButton()->setEnabled(false);
    mwin->getCompareTreeToPushButton()->setEnabled(false);
}

void CompareTree::editCurrentVersionAction(QAction* _act)
{
    QStringList tmp = _act->data().toStringList();

    if (tmp.isEmpty() || tmp.front() != "ACT2")
        return;

    tmp.pop_front();
    if (tmp.isEmpty())
        return;

    QString path = tmp.front();

    editCurrentVersion(path);
}

void CompareTree::compareFileVersionsAction(QAction* _act)
{
    QStringList tmp = _act->data().toStringList();

    if (tmp.isEmpty() || tmp.front() != "ACT1")
        return;

    tmp.pop_front();
    if (tmp.isEmpty())
        return;

    QString path = tmp.front();
    tmp.pop_front();
    if (tmp.isEmpty())
        return;

    QString status = tmp.front();
    tmp.pop_front();

    QString path_old = (!tmp.isEmpty()) ? tmp.front() : path;

    compareFileVersions(path, status, path_old);
}

void CompareTree::gitLogFileAction(QAction* _act)
{
    QStringList tmp = _act->data().toStringList();

    if (tmp.isEmpty())
        return;

    if (tmp.front() == "ACT3")
    {
        tmp.pop_front();
        if (tmp.isEmpty())
            return;

        // graph->gitlog(tmp.front());
        graph->setGitLogFileConstraint(tmp.front());
    }
    else if (tmp.front() == "ACT4")
    {
        graph->removeFilter();
    }
}

void CompareTree::onCustomContextMenu(const QPoint& point)
{
    QModelIndex _idx = indexAt(point);

    if (_idx.isValid())
    {
        QModelIndex src = _idx;

        if (_idx.column() != 1)
        {
            src = _idx.sibling(_idx.row(), 1);
            if (src.isValid() == false)
            {
                src = _idx;
            }
        }

        QVariant ret = src.data(Qt::DisplayRole);

        if (!ret.isValid())
            return;

        QString path = ret.toString();

        ret = src.data(Qt::UserRole + 1);
        if (!ret.isValid())
            return;

        QString status = ret.toString();

        QString path_old = path;
        if (status == "R")
        {
            ret = src.data(Qt::UserRole + 2);
            if (ret.isValid())
                path_old = ret.toString();
        }

        QMenu* menu = new QMenu(this);
        QAction* act = NULL;
        QStringList tmp;

        if (status == "X" || status == "M" || status == "R" || status == "D")
        {
            act = new QAction("Show version diff", this);
            tmp << "ACT1" << path << status << path_old;
            act->setData(QVariant(tmp));
            tmp.clear();
            menu->addAction(act);
        }

        if (status == "X" || status == "M" || status == "R" || status == "A")
        {
            act = new QAction("Edit current version", this);
            tmp << "ACT2" << path;
            act->setData(QVariant(tmp));
            tmp.clear();
            menu->addAction(act);
        }

        if (graph->getFileConstraint() != path)
        {
            act = new QAction("Filter versions by file", this);
            tmp << "ACT3" << path;
            act->setData(QVariant(tmp));
            tmp.clear();
            menu->addAction(act);
        }
        else
        {
            act = new QAction("Remove filter", this);
            tmp << "ACT4";
            act->setData(QVariant(tmp));
            tmp.clear();
            menu->addAction(act);
        }

        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(compareFileVersionsAction(QAction*)));
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(editCurrentVersionAction(QAction*)));
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(gitLogFileAction(QAction*)));
        menu->exec(viewport()->mapToGlobal(point));
    }
}

void CompareTree::setMainWindow(class MainWindow* _mwin)
{
    mwin = _mwin;
}

void CompareTree::setGraphWidget(class GraphWidget* _graph)
{
    graph = _graph;
}

void CompareTree::viewLocalChanges(bool _staged)
{
    // get data
    QString cmd = "git -C " + graph->getLocalRepositoryPath() + (_staged == true ? " diff --cached --name-only" : " ls-files -m");

    QList<QString> cache;
    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    QStandardItemModel* treemodel = new QStandardItemModel(NULL);

    treemodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Path")));
    treemodel->setHorizontalHeaderItem(1, new QStandardItem(QString("File")));

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    header()->setResizeMode(QHeaderView::Stretch);
#endif

    QStandardItem* root = treemodel->invisibleRootItem();
    root->setEditable(false);

    // now copy the path information into a nice tree widget
    for (QStringList::iterator jt = cache.begin();
         jt != cache.end();
         ++jt)
    {
        QString path = (*jt).trimmed();

        // split file path into a QStringList
        QStringList path_elements = path.split(QChar('/'));

        // start with root node...
        QStandardItem* p = root;

        for (QStringList::iterator it = path_elements.begin();
             it != path_elements.end();
             ++it)
        {
            // check all childs if the entry already exists
            bool skip = false;
            for (int i = 0; i < p->rowCount(); i++)
            {
                QStandardItem* t = p->child(i);
                if (t->text() == *it)
                {
                    p = t;
                    skip = true;
                    break;
                }
            }

            if (skip == false)
            {
                QList<QStandardItem*> columns;
                // add new node under p
                QStandardItem* t = new QStandardItem(*it);
                t->setEditable(false);

                QStringList::iterator kt = it;
                ++kt;
                if (kt == path_elements.end())
                {
                    columns << t;
                    QStandardItem* actitem = new QStandardItem(path);
                    actitem->setEditable(false);
                    actitem->setData("X", Qt::UserRole + 1);
                    actitem->setData(path, Qt::UserRole + 2);
                    columns << actitem;
                    p->appendRow(columns);
                }
                else
                {
                    t->setIcon(QIcon().fromTheme("folder")); // folder-open
                    p->appendRow(t);
                    p = t;
                }
            }
        }
    }

    setModel(treemodel);
    expandTree();
}


void CompareTree::viewThisVersion(const QString& _hash)
{
    // get data
    QString cmd = "git -C " + graph->getLocalRepositoryPath() + " ls-tree --full-tree --name-only -r " + _hash;

    QList<QString> cache;
    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    QStandardItemModel* treemodel = new QStandardItemModel(NULL);

    treemodel->setHorizontalHeaderItem(0, new QStandardItem(QString("Path")));
    treemodel->setHorizontalHeaderItem(1, new QStandardItem(QString("File")));

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    header()->setResizeMode(QHeaderView::Stretch);
#endif

    QStandardItem* root = treemodel->invisibleRootItem();
    root->setEditable(false);

    // now copy the path information into a nice tree widget
    for (QStringList::iterator jt = cache.begin();
         jt != cache.end();
         ++jt)
    {
        QString path = (*jt).trimmed();

        // split file path into a QStringList
        QStringList path_elements = path.split(QChar('/'));

        // start with root node...
        QStandardItem* p = root;

        for (QStringList::iterator it = path_elements.begin();
             it != path_elements.end();
             ++it)
        {
            // check all childs if the entry already exists
            bool skip = false;
            for (int i = 0; i < p->rowCount(); i++)
            {
                QStandardItem* t = p->child(i);
                if (t->text() == *it)
                {
                    p = t;
                    skip = true;
                    break;
                }
            }

            if (skip == false)
            {
                QList<QStandardItem*> columns;
                // add new node under p
                QStandardItem* t = new QStandardItem(*it);
                t->setEditable(false);

                QStringList::iterator kt = it;
                ++kt;
                if (kt == path_elements.end())
                {
                    columns << t;
                    QStandardItem* actitem = new QStandardItem(path);
                    actitem->setEditable(false);
                    actitem->setData("X", Qt::UserRole + 1);
                    actitem->setData(path, Qt::UserRole + 2);
                    columns << actitem;
                    p->appendRow(columns);
                }
                else
                {
                    t->setIcon(QIcon().fromTheme("folder")); // folder-open
                    p->appendRow(t);
                    p = t;
                }
            }
        }
    }

    setModel(treemodel);
    expandTree();
}

QString CompareTree::createTempVersionFile(const QString& _hash, const QString& _path) const
{
    QString extension = getFileExtension(_path);
    QString fname = QString("%1/%2_%3.%4").arg(mwin->getTempPath()).arg(_hash).arg(getpid()).arg(extension);
    QString cmd = "git -C " + graph->getLocalRepositoryPath() + " show " + _hash + ":" + _path + " > " + fname;

    QList<QString> dummy;
    execute_cmd(cmd.toUtf8().data(), dummy, mwin->getPrintCmdToStdout());

    mwin->addToCleanupFiles(fname);

    return fname;
}

void CompareTree::compareFileVersions(
    const QString& _path,
    const QString& _status,
    const QString& _path_old)
{
    // if status is none of the following, nothing to do
    if (QString("MDARX").contains(_status) == false)
        return;

    bool compareToLocalCurrent = mwin->getDiffLocalFiles();

    // diffFiles will contain all temp file paths of the files to compare
    QStringList diffFiles;

    QSet<Version*> predecessors = graph->getPredecessors();
    foreach(Version * it, predecessors)
    {
        diffFiles.push_back(createTempVersionFile(it->getHash(), _path_old));
    }

    if (_status != "D" && graph->getToHash().size())
        diffFiles.push_back(createTempVersionFile(graph->getToHash(), _path));

    if (diffFiles.size() == 0)
        compareToLocalCurrent = true;

    // if local file is identical to a file from diffFiles, ignore it
    if (compareToLocalCurrent || (graph->getToHash().size() == 0))
    {
        QString localFile = graph->getLocalRepositoryPath() + "/" + _path;

        if (QFile::exists(localFile))
        {

            QStringList tmp;
            foreach(QString fname, diffFiles)
            {
                QString cmd = "diff " + fname + " " + localFile;

                QList<QString> cache;
                execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());
                if (cache.size())
                    tmp.push_back(fname);
            }
            diffFiles = tmp;
            diffFiles.push_back(localFile);
        }
    }

    // mime type to tool
    QString difftool;
    QString dummy;
    mwin->getMimeTypeTools(getMimeType(diffFiles.front()), difftool, dummy);

    // drop empty files
    {
        QStringList tmp;
        foreach(QString fname, diffFiles)
        {
            QFileInfo fi(fname);

            if (fi.size() != 0)
                tmp.push_back(fname);
        }
        diffFiles = tmp;
    }

    QString fnameList = diffFiles.join(QString(" "));

    difftool.replace("%1", fnameList);

    system(difftool.toUtf8().data());
}

void CompareTree::editCurrentVersion(const QString& _path)
{
    QString tmp = graph->getLocalRepositoryPath() + "/" + _path;
    QString mimeType = getMimeType(tmp);
    QString difftool;
    QString edittool;

    mwin->getMimeTypeTools(mimeType, difftool, edittool);
    edittool.replace("%1", tmp);
    system(edittool.toUtf8().data());
}

QString CompareTree::getMimeType(const QString& _path) const
{
    QString cmd = "file --mime-type -b " + _path;

    QList<QString> cache;
    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    if (cache.size())
        return cache.at(0);

    return QString();
}

QString CompareTree::getFileExtension(const QString& _path) const
{
    QRegExp extension("\\.([^/.]*)$");

    if (extension.indexIn(_path, 0))
    {
        return extension.cap(1);
    }
    return QString();
}

void CompareTree::currentIndexChanged(int _index)
{
    Version* v = mwin->getFromComboBox()->itemData(_index).value<VersionPointer>();

    if (v)
    {
        graph->commitInfo(v, mwin->getCompareTreeFromTextEdit());
    }
}
