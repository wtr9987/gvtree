/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2022 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.2-0                */
/*                                               */
/*             git version tree browser          */
/*                                               */
/*   13. February 2022                           */
/*                                               */
/*         This program is licensed under        */
/*           GNU GENERAL PUBLIC LICENSE          */
/*            Version 3, 29 June 2007            */
/*                                               */
/* --------------------------------------------- */

#include "execute_cmd.h"
#include "branchlist.h"
#include "mainwindow.h"

BranchList::BranchList(QWidget* _parent) : QListView(_parent),
    mwin(NULL),
    sortRole(Qt::UserRole),
    sortOrder(Qt::DescendingOrder)
{
    blModel = new QStandardItemModel(_parent);
    setModel(blModel);

    setSelectionMode(QAbstractItemView::SingleSelection);

    connect (selectionModel(),
             SIGNAL(selectionChanged(const QItemSelection &_selected,const QItemSelection &_deselected)),
             this, SLOT(selectionChanged(const QItemSelection &_selected,const QItemSelection &_deselected)));
}

void BranchList::setMainWindow(MainWindow* _mwin)
{
    mwin = _mwin;
}

void BranchList::refresh(const QString& _localRepositoryPath)
{
    bool sigstat = blockSignals(true);

    QString selectedBranch = currentIndex().isValid() ? blModel->data(currentIndex(), Qt::DisplayRole).toString() : QString();

    // erase all
    blModel->clear();

    // default sort is latest committed branch on top
    QString cmd = "git -C " + _localRepositoryPath + " branch -a --sort=-committerdate";

    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    // insert data
    QStandardItem* root = blModel->invisibleRootItem();
    QString branchName;
    int i = 0, cr = 0, sr = -1;

    foreach(const QString &it, cache)
    {
        if (it.indexOf("->") > 0)
            continue;

        QStandardItem* item = new QStandardItem();
        if (it.at(0) == '*')
        {
            branchName = it.mid(1).trimmed();
            currentBranch = item;
            cr = i;
        }
        else
        {
            branchName = it.trimmed();
            sr = (selectedBranch == branchName) ? i : sr;
        }
        item->setData(i++, Qt::UserRole);
        item->setEditable(false);
        item->setData(branchName, Qt::DisplayRole);
        root->appendRow(item);
    }

    if (sr == -1)
        setCurrentIndex(blModel->index(cr, 0));

    blModel->setSortRole(sortRole);
    blModel->sort(0, sortOrder);

    blockSignals(sigstat);
}

QString BranchList::getSelectedBranch() const
{
    if (currentIndex().isValid() && blModel->itemFromIndex(currentIndex()) != currentBranch)
        return blModel->data(currentIndex(), Qt::DisplayRole).toString();

    return QString();
}

void BranchList::resetSelection()
{
    setCurrentIndex(blModel->indexFromItem(currentBranch));
}

void BranchList::setSort(int _val)
{
    switch (_val)
    {
        case 0:
            sortRole = Qt::UserRole;
            sortOrder = Qt::DescendingOrder;
            break;
        case 1:
            sortRole = Qt::UserRole;
            sortOrder = Qt::AscendingOrder;
            break;
        case 2:
            sortRole = Qt::DisplayRole;
            sortOrder = Qt::AscendingOrder;
            break;
        case 3:
            sortRole = Qt::DisplayRole;
            sortOrder = Qt::DescendingOrder;
            break;
    }
    blModel->setSortRole(sortRole);
    blModel->sort(0, sortOrder);
}

void BranchList::selectionChanged(const QItemSelection&, const QItemSelection&)
{
    emit itemSelectionChanged();
}
