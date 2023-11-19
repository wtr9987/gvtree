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

#include <QSettings>
#include "execute_cmd.h"
#include "branchlist.h"
#include "mainwindow.h"

#include <iostream>

BranchList::BranchList(QWidget* _parent) : QTableWidget(_parent),
    mwin(NULL), currentBranch(NULL), selectedBranch(NULL)
{
    verticalHeader()->hide();

    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setHighlightSections(false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif

    //setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

   setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    
  
}

void BranchList::setMainWindow(MainWindow* _mwin)
{
    mwin = _mwin;
}

void BranchList::refresh(const QString& _localRepositoryPath)
{
    bool sigstat = blockSignals(true);

    clear();

    QStringList header;

    header << "branch" << "date";
    setColumnCount(2);
    setHorizontalHeaderLabels(header);
    setSortingEnabled(false);

    // default sort is latest committed branch on top
    QString cmd = "git -C " + _localRepositoryPath + " branch -l --format=\"%(HEAD);%(refname:short);%(committerdate:short);\" --all --sort=committerdate";
    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    // insert data
    QString branchName;
    QString branchDate;

    foreach(const QString& it, cache)
    {
        if (it.indexOf("->") > 0)
            continue;

        QStringList line = it.split(';');

        QString col = line.front();
        bool current = (col.at(0) == '*');

        line.pop_front();
        if (line.isEmpty())
          continue;

        QString branchName = line.front();
        line.pop_front();
        if (line.isEmpty())
          continue;

        QString branchDate = line.front();

        insertRow(rowCount());
        QTableWidgetItem* item = new QTableWidgetItem(branchName);

        if (current)
        {
          item->setBackground(QBrush(QColor(255,192,192)));
          currentBranch=item;
        }

        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        setItem(rowCount() - 1, 0, item);
        item = new QTableWidgetItem(branchDate);
        item->setFlags(Qt::NoItemFlags); 
        setItem(rowCount() - 1, 1, item);

    }
    selectCurrentBranch();
    setSortingEnabled(true);

    QSettings settings;

    if (!settings.contains("branchList/sortColumn"))
        settings.setValue("branchList/sortColumn", 1);
    if (!settings.contains("branchList/sortOrder"))
        settings.setValue("branchList/sortOrder", 1);

    sortByColumn(settings.value("branchList/sortColumn").toInt(),
                    settings.value("branchList/sortOrder").toInt()==0?Qt::AscendingOrder:Qt::DescendingOrder);

    connect(horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortChanged(int)));

    blockSignals(sigstat);
}

void BranchList::sortChanged(int _col)
{
  QSettings settings;
  settings.setValue("branchList/sortColumn", _col);
  settings.setValue("branchList/sortOrder", horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder?0:1);
}

void BranchList::selectionChanged(const QItemSelection& , const QItemSelection& )
{
    emit itemSelectionChanged();
}

QString BranchList::getSelectedBranch() const
{
  foreach(QTableWidgetItem* it, selectedItems())
  {
    if (it->column() == 0)
    {
      return it->text();
    }
  }
    return QString();
}

void BranchList::onCustomContextMenu(const QPoint& point)
{
    QMenu* menu = new QMenu(this);
    QAction* act = new QAction("Current branch", this);

    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(selectCurrentBranch()));
    menu->exec(viewport()->mapToGlobal(point));
}

void BranchList::mousePressEvent (QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        event->accept();
    }
    else
        QTableWidget::mousePressEvent(event);
}

void BranchList::selectCurrentBranch()
{
  if (currentBranch)
  {
    clearSelection();
    selectionModel()->select(indexFromItem(currentBranch),QItemSelectionModel::Current);
    selectionModel()->setCurrentIndex(indexFromItem(currentBranch),QItemSelectionModel::Current);
    selectedBranch = currentBranch;
  }
}
