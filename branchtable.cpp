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
#include "branchtable.h"
#include "mainwindow.h"

#include <iostream>

BranchTable::BranchTable(QWidget* _parent) : QTableWidget(_parent),
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

    setSizeAdjustPolicy(QTableWidget::AdjustToContents);

    setSelectionMode(QAbstractItemView::SingleSelection);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));
}

void BranchTable::setMainWindow(MainWindow* _mwin)
{
    mwin = _mwin;
}

void BranchTable::refresh(const QString& _localRepositoryPath)
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
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            currentBranch = item;
        }

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        setItem(rowCount() - 1, 0, item);
        item = new QTableWidgetItem(branchDate);
        item->setFlags(Qt::NoItemFlags);
        setItem(rowCount() - 1, 1, item);
    }
    selectCurrentBranch();
    setSortingEnabled(true);

    QSettings settings;

    if (!settings.contains("BranchTable/sortColumn"))
        settings.setValue("BranchTable/sortColumn", 1);
    if (!settings.contains("BranchTable/sortOrder"))
        settings.setValue("BranchTable/sortOrder", 1);

    sortByColumn(settings.value("BranchTable/sortColumn").toInt(),
                 settings.value("BranchTable/sortOrder").toInt() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder);

    connect(horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortChanged(int)));

    blockSignals(sigstat);
}

void BranchTable::sortChanged(int _col)
{
    QSettings settings;

    settings.setValue("BranchTable/sortColumn", _col);
    settings.setValue("BranchTable/sortOrder", horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder ? 0 : 1);
}

void BranchTable::selectionChanged(const QItemSelection&, const QItemSelection&)
{
    emit itemSelectionChanged();
}

QString BranchTable::getSelectedBranch() const
{
    foreach(QTableWidgetItem * it, selectedItems())
    {
        if (it->column() == 0)
        {
            return it->text();
        }
    }
    return QString();
}

void BranchTable::onCustomContextMenu(const QPoint& point)
{
    QMenu* menu = new QMenu(this);
    QAction* act = new QAction("Current branch", this);

    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(selectCurrentBranch()));
    menu->exec(viewport()->mapToGlobal(point));
}

void BranchTable::mousePressEvent (QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        event->accept();
    }
    else
        QTableWidget::mousePressEvent(event);
}

void BranchTable::selectCurrentBranch()
{
    if (currentBranch)
    {
        clearSelection();
        selectionModel()->select(indexFromItem(currentBranch), QItemSelectionModel::Current);
        selectionModel()->setCurrentIndex(indexFromItem(currentBranch), QItemSelectionModel::Current);
        selectedBranch = currentBranch;
    }
}
