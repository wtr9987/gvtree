/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2023 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.9-0                */
/*                                               */
/*             git version tree browser          */
/*                                               */
/*   19. November 2023                           */
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
    BranchTable(QWidget* _parent = NULL);

    /**
     * \brief Get branch information of current local repository
     */
    void refresh(const QString& _localRepositoryPath);
    QString getSelectedBranch() const;
    void setMainWindow(class MainWindow* _mwin);

public slots:
    void sortChanged(int _col);
    void lookupCurrent();
    void lookupBranch(int _row, int _column);
    void onCustomContextMenu(const QPoint& point);

protected:
    // block right button to allow context menu
    virtual void mousePressEvent (QMouseEvent* event);
    void branchSelectionChanged();

    class MainWindow* mwin;
    QTableWidgetItem* currentBranch;
    QTableWidgetItem* selectedBranch;
    bool blockReload;
};

#endif
