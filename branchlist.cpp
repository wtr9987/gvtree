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

BranchList::BranchList(MainWindow* _parent) : QListWidget(_parent), mwin(_parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void BranchList::refresh(const QString& _localRepositoryPath)
{
    bool sigstat = blockSignals(true);

    QListWidget::clear();

    QString cmd = "git -C " + _localRepositoryPath + " branch -a";

    QList<QString> cache;
    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    bool selectedBranchPresent = false;

    QListWidgetItem* item = NULL;
    foreach(const QString &it, cache)
    {
      if (it.indexOf("->")>0)
        continue;
        if (it.at(0) == '*')
        {
            currentLocalBranch = it.mid(1).trimmed();

            if (selectedBranch == currentLocalBranch)
                selectedBranchPresent = true;

            item = new QListWidgetItem(it.mid(1).trimmed());
            addItem(item);
            item->setSelected(1);
        }
        else
        {
            if (selectedBranch == it.trimmed())
                selectedBranchPresent = true;
            item = new QListWidgetItem(it.trimmed());
            addItem(item);
        }
    }

    if (selectedBranchPresent == false)
        selectedBranch = currentLocalBranch;

    blockSignals(sigstat);
}

const QString& BranchList::getCurrentLocalBranch() const
{
    return currentLocalBranch;
}

QString BranchList::getSelectedBranch() const
{
    if (selectedItems().size() < 1)
        return currentLocalBranch;

    return selectedItems().front()->text();
}
