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

#include <QSettings>
#include "execute_cmd.h"
#include "branchtable.h"
#include "mainwindow.h"

#include <iostream>

BranchTable::BranchTable(QWidget* _parent) : QTableWidget(_parent),
    mwin(NULL), currentBranch(NULL), selectedBranch(NULL), blockReload(false)
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

    setSelectionMode(QAbstractItemView::NoSelection);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(lookupBranch(int,int)));
}

void BranchTable::setMainWindow(MainWindow* _mwin)
{
    mwin = _mwin;
}

void BranchTable::refresh(const QString& _localRepositoryPath)
{
    if (!mwin || blockReload)
        return;

    bool sigstat = blockSignals(true);

    clear();

    // Create header elements
    QStringList header;

    header << "Branch" << "Last Committer Date";
    setColumnCount(2);
    setHorizontalHeaderLabels(header);
    setSortingEnabled(false);

    // get branch data
    QString cmd = "git -C " + _localRepositoryPath + " branch -l --format=\"%(HEAD);%(refname:short);%(committerdate:iso8601);\" --all --sort=committerdate";
    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, mwin->getPrintCmdToStdout());

    // insert data
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

void BranchTable::lookupCurrent()
{
    lookupBranch(currentBranch->row(), 0);
}

void BranchTable::lookupBranch(int _row, int /*_col*/)
{
    selectedBranch = item(_row, 0);
    selectionModel()->setCurrentIndex(indexFromItem(selectedBranch), QItemSelectionModel::Current);
    branchSelectionChanged();
}

void BranchTable::branchSelectionChanged()
{
    if (!mwin)
        return;

    if (mwin->getAll() == false)
    {
        blockReload = true;
        mwin->reloadCurrentRepository();
        blockReload = false;
    }

    if (!mwin->getGraphWidget()->focusElements(getSelectedBranch(), true, QString("Branch")))
        mwin->getGraphWidget()->focusElements(getSelectedBranch(), false, QString("Branch"));
}

void BranchTable::onCustomContextMenu(const QPoint& point)
{
    QMenu* menu = new QMenu(this);
    QAction* act = new QAction("Current branch", this);

    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(lookupCurrent()));
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

QString BranchTable::getSelectedBranch() const
{
    return selectedBranch ? selectedBranch->text() : QString();
}
