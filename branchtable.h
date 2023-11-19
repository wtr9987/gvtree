/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2022 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.7-0                */
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

#ifndef __BRANCHTABLE_H__
#define __BRANCHTABLE_H__

#include <QStandardItem>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QTableWidget>
#include <QWidget>
#include <QString>

class BranchTable : public QTableWidget
{
    Q_OBJECT

public:
    BranchTable(class QWidget* _parent = NULL);

    /**
     * \brief Get branch information of current local repository
     */
    void refresh(const QString& _localRepositoryPath);
    QString getSelectedBranch() const;
    void setMainWindow(class MainWindow* _mwin);

public slots:
    void sortChanged(int _col);

    void selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected);
    void onCustomContextMenu(const QPoint& point);
    void selectCurrentBranch();

protected:
    // block right button to allow context menu
    virtual void mousePressEvent (QMouseEvent* event);
    class MainWindow* mwin;
    QTableWidgetItem* currentBranch;
    QTableWidgetItem* selectedBranch;

    int sortRole;
    Qt::SortOrder sortOrder;

signals:
    void itemSelectionChanged();
};

#endif
