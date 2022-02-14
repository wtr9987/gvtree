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

#ifndef __BRANCHLIST_H__
#define __BRANCHLIST_H__

#include <QListWidget>
#include <QTabWidget>
#include <QSet>
#include <QStringList>
#include <QWidget>
#include <QString>

class BranchList : public QListWidget
{
    Q_OBJECT

public:
    BranchList(class MainWindow* _parent = NULL);

    void refresh(const QString& _localRepositoryPath);
    const QString& getCurrentLocalBranch() const;
    QString getSelectedBranch() const;

protected:
    class MainWindow* mwin;
    QString currentLocalBranch;
    QString selectedBranch;
};

#endif
