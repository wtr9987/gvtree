/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2022 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.4-0                */
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

#ifndef __BRANCHLIST_H__
#define __BRANCHLIST_H__

#include <QStandardItem>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QListView>
#include <QWidget>
#include <QString>

class BranchList : public QListView
{
    Q_OBJECT

public:
    BranchList(class QWidget* _parent = NULL);

    /**
     * \brief Get branch information of current local repository
     */
    void refresh(const QString& _localRepositoryPath);
    QString getSelectedBranch() const;
    void setMainWindow(class MainWindow* _mwin);

public slots:
    void setSort(int _val);
    void resetSelection();

    void selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected);

protected:
    class MainWindow* mwin;
    QStandardItem* currentBranch;

    int sortRole;
    Qt::SortOrder sortOrder;
    QStandardItemModel* blModel;

signals:
    void itemSelectionChanged();
};

#endif
