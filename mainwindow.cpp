/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.9-0                */
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

#include <QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QScreen>
#else
#include <QTextCodec>
#include <QDesktopWidget>
#endif


#include <QColorDialog>
#include <QCoreApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QScrollBar>
#include <QSettings>
#include <QStatusBar>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#include <QtAlgorithms>

#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

#include "execute_cmd.h"
#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow(const QStringList& _argv) : QMainWindow(NULL), ctwin(NULL), blwin(NULL), pwin(NULL)
{
    // setup preferences dialog first
    pwin = new QDialog;
    gvtree_preferences.setupUi(pwin);
    restorePreferencesSettings();
    restoreColorSettings();

    // watchdog for the local repository directory
    watcher = new QFileSystemWatcher();
    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(showRefreshButton(const QString&)));
    connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(showRefreshButton(const QString&)));

    cbRemotes = new QCheckBox("--remotes");
    cbAll = new QCheckBox("--all");
    {
        QSettings settings;
        if (settings.contains("remotes") && settings.value("remotes").toBool())
            cbRemotes->setChecked(true);
        else
            cbRemotes->setChecked(false);
        if (settings.contains("all") && settings.value("all").toBool())
            cbAll->setChecked(true);
        else
            cbAll->setChecked(false);
    }
    connect(cbRemotes, SIGNAL(stateChanged(int)), this, SLOT(remotesChanged(int)));
    connect(cbAll, SIGNAL(stateChanged(int)), this, SLOT(allChanged(int)));

    // current repository path
    lbRepositoryPath = new QLabel("");
    lbRepositoryPath->setStyleSheet("font-size: 10pt; color: black;");
    pbRepositoryName = new QPushButton("No repository set.");
    pbRepositoryName->setStyleSheet("font-size: 14pt; color: green;");
    pbRepositoryName->setStatusTip(tr("Set path to git repository."));
    pbRepositoryName->setFlat(true);
    connect(pbRepositoryName, SIGNAL(pressed()), this, SLOT(setGitLocalRepository()));

    pbFileConstraint = new QPushButton("");
    pbFileConstraint->setStyleSheet("font-size: 14pt; color: red;");
    pbFileConstraint->setStatusTip(tr("Remove filter"));
    pbFileConstraint->setFlat(true);
    pbFileConstraint->hide();
    connect(pbFileConstraint, SIGNAL(pressed()), this, SLOT(removeFilter()));

    // if watchdog recognizes changes in the local repository this button will appear
    pbRepositoryRefresh = new QPushButton();
    pbRepositoryRefresh->setIcon(QIcon(":/images/gvt_refresh.png"));
    pbRepositoryRefresh->hide();
    connect(pbRepositoryRefresh, SIGNAL(pressed()), this, SLOT(reloadCurrentRepository()));

    // status bar
    statusBar()->show();
    statusBar()->addPermanentWidget(cbRemotes);
    statusBar()->addPermanentWidget(cbAll);
    statusBar()->addPermanentWidget(lbRepositoryPath);
    statusBar()->addPermanentWidget(pbRepositoryName);
    statusBar()->addPermanentWidget(pbFileConstraint);
    statusBar()->addPermanentWidget(pbRepositoryRefresh);

    // setup basics
    QWidget* centralWidget = new QWidget(this);

    setCentralWidget(centralWidget);
    QHBoxLayout* centralLayout = new QHBoxLayout(centralWidget);

    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    // setup central graph widget
    graphwidget = new GraphWidget(this);
    graphwidget->preferencesUpdated();
    centralLayout->addWidget(graphwidget);
    graphwidget->centerOn(graphwidget->mapToScene(QPoint(0, 0)));
    graphwidget->setFocus();

    // ...
    setMinimumSize(400, 400);
    resize(QSize(800, 600));

    // create top menu bar
    createMenus();

    // create dock widgets
    QDockWidget* dock = NULL;

    // -- create new
    tagtree = new TagTree(graphwidget, this);
    dock = new QDockWidget(tr("Version Information"), this);
    dock->setObjectName("Version Information");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(tagtree);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    windowmenu->addAction(dock->toggleViewAction());
    dock->hide();
    tagTreeDock = dock;

    // -- create text browser for current git status
    gitstatus = new QTextBrowser(this);
    dock = new QDockWidget(tr("Current git status"), this);
    dock->setObjectName("Current git status");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(gitstatus);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    windowmenu->addAction(dock->toggleViewAction());
    dock->hide();

    // -- create treeview to compare versions on file basis
    ctwin = new QWidget;
    gvtree_comparetree.setupUi(ctwin);
    graphwidget->setCompareTree(gvtree_comparetree.compareTree);
    gvtree_comparetree.compareTree->setMainWindow(this);
    gvtree_comparetree.compareTree->setGraphWidget(graphwidget);
    gvtree_comparetree.compareTree->resetCompareTree();

    dock = new QDockWidget(tr("Compare Versions"), this);
    dock->setObjectName("Compare Versions");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(ctwin);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    windowmenu->addAction(dock->toggleViewAction());
    compareTreeDock = dock;
    dock->hide();

    connect(gvtree_comparetree.fromComboBox, SIGNAL(currentIndexChanged(int)),
            gvtree_comparetree.compareTree, SLOT(currentIndexChanged(int)));
    connect(gvtree_comparetree.toButton, SIGNAL(pressed()), graphwidget, SLOT(focusToVersion()));
    connect(gvtree_comparetree.fromButton, SIGNAL(pressed()), graphwidget, SLOT(focusFromVersion()));

    // -- list of all branches
    blwin = new QWidget;
    gvtree_branchtable.setupUi(blwin);
    gvtree_branchtable.branchTable->setMainWindow(this);

    dock = new QDockWidget(tr("Branch Table"), this);
    dock->setObjectName("Branch Table");
    dock->setWidget(blwin);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    windowmenu->addAction(dock->toggleViewAction());
    branchDock = dock;
    dock->hide();

    // parse arguments
    QString fileConstraint;

    bool fromfile = false;

    for (int i = 0; i < _argv.size(); i++)
    {
        if (_argv.at(i) == "-t")
        {
            graphwidget->test();
            fromfile = true;
        }
        else if (_argv.at(i) == "-f" && i + 1 < _argv.size())
        {
            ++i;
            graphwidget->load(_argv.at(i));
            fromfile = true;
        }
        else if (_argv.at(i) == "-r" && i + 1 < _argv.size())
        {
            ++i;
            if (checkGitLocalRepository(_argv.at(i), repositoryPath, fileConstraintPath))
            {
                QSettings settings;
                settings.setValue("localRepositoryPath", repositoryPath);
                graphwidget->setLocalRepositoryPath(repositoryPath);
                gvtree_branchtable.branchTable->refresh(repositoryPath);

                QString path, fname;
                splitRepositoryPath(repositoryPath, path, fname);
                lbRepositoryPath->setText(path);
                pbRepositoryName->setText(fname);
                gvtree_preferences.pbLocalRepositoryPath->setText(repositoryPath);
                graphwidget->gitlog();
                refreshRepo->setEnabled(true);
            }
            else
            {
                lbRepositoryPath->setText("");
                pbRepositoryName->setText("No repository set.");
                gvtree_preferences.pbLocalRepositoryPath->setText("");
                refreshRepo->setEnabled(false);
            }
        }
        else if (_argv.at(i) == "--silent" && i + 1 < _argv.size())
        {
            QSettings settings;
            settings.setValue("printCmdToStdout", _argv[i] == "true" ? true : false);
        }
        else if (_argv.at(i) == "--version")
        {
            cout << VERSION_NAME << endl;
            QCoreApplication::exit(0);
            exit(0);
        }
        else if (_argv.at(i) == "--css")
        {
            ++i;
            applyStyleSheetFile(QString(_argv.at(i)));
        }
        else if (_argv.at(i) == "-h")
        {
            cout << VERSION_NAME << endl;
            cout << endl;
            cout << "Tool to display git log graph" << endl;
            cout << endl;
            cout << "gvtree Copyright (C) 2021 Wolfgang Trummer" << endl;
            cout << endl;
            cout << "  This program comes with ABSOLUTELY NO WARRANTY" << endl;
            cout << "  This is free software, and you are welcome to redistribute it" << endl;
            cout << "  under certain conditions" << endl;
            cout << endl;
            cout << "  This program is licensed under" << endl;
            cout << "  GNU GENERAL PUBLIC LICENSE" << endl;
            cout << "  Version 3, 29 June 2007" << endl;
            cout << endl;
            cout << "--------------------------------------------------------------------------------" << endl;
            cout << "Parameters:" << endl;
            cout << "[path]" << endl;
            cout << "   Set a file constraint. The version tree of the file will" << endl;
            cout << "   be displayed." << endl;
            cout << endl;
            cout << "-r [local git repository directory]" << endl;
            cout << "   If not specified the current path is checked for a valid repository" << endl;
            cout << "   or the repository used in the previous session is displayed." << endl;
            cout << "   Which one is used can be controlled by the preferences setting." << endl;
            cout << endl;
            cout << "--version Version string is printed to stdout" << endl;
            cout << endl;
            cout << "--silent true|false Silent mode." << endl;
            cout << "   If true, commands are not printed to stdout. The preferences" << endl;
            cout << "   \'print commandline to stdout\' is set to this value." << endl;
            cout << endl;
            cout << "--css [style sheet file]" << endl;
            cout << "   Load a css style sheet file." << endl;
            cout << "   If not specified the last file used will be taken." << endl;
            cout << "   Perhaps it is a good idea to copy gvtree.css to ~/.config/gvtree" << endl;
            cout << "   and run ./gvtree --css ~/.config/gvtree/gvtree.css once." << endl;
            cout << endl;
            cout << "-t Testing:" << endl;
            cout << "   Display the test tree graph from (3)." << endl;
            cout << endl;
            cout << "-f [gitlog] " << endl;
            cout << "   Testing:" << endl;
            cout << "   Load a file created with " << endl;
            cout << "     git log --graph --pretty=\"#%h#%at#%an#%d#%s#\"" << endl;
            cout << "   This has been helpful during development to import constraint and" << endl;
            cout << "   complex repository data." << endl;
            cout << endl;
            cout << "-h This information." << endl;
            cout << endl;
            cout << "--------------------------------------------------------------------------------" << endl;
            cout << endl;
            QCoreApplication::exit(0);
            exit(0);
        }
        else
        {
            if (i > 0)
                fileConstraint = _argv.at(i);
        }
    }

    show();
    restoreWindowSettings();

    if (fromfile == false)
    {
        restoreLocalRepository();
        if (fileConstraint.isEmpty() == false)
        {
            // setting a file constraint is rather difficult, because of path arithmetics ;)
            char* wdir = getcwd(NULL, 0);
            if (wdir == NULL)
            {
                cerr << "Allocation memory for getcwd failed." << endl;
                exit(-1);
            }

            QString fileConstraintPath(wdir);
            free(wdir);

            // now eliminate the repository path from the current pwd
            if (repositoryPath == fileConstraintPath)
            {
                fileConstraintPath = QString();
            }
            else
            {
                QString removePath = repositoryPath + "/";
                fileConstraintPath.replace(removePath, QString());
            }

            // ensure to have the pwd path relative to the reposirotyPath
            if (fileConstraintPath.isEmpty() == false)
                fileConstraint = fileConstraintPath + "/" + fileConstraint;

            // check if the constraint is existing at all...
            QFileInfo fi(repositoryPath + "/" + fileConstraint);
            if (fi.exists())
            {
                graphwidget->setGitLogFileConstraint(fileConstraint);
            }
            else
            {
                fileConstraint = QString();
            }
        }
    }
    else
    {
        lbRepositoryPath->setText("");
        gvtree_preferences.pbLocalRepositoryPath->setText("");
        pbRepositoryName->setText("No repository set.");
        refreshRepo->setEnabled(false);
    }

    graphwidget->updateColors();
}

void MainWindow::updatePbFileConstraint(const QString& _fileConstraint)
{
    if (_fileConstraint.size())
    {
        pbFileConstraint->setText(_fileConstraint);
        pbFileConstraint->show();
    }
    else
    {
        pbFileConstraint->setText("");
        pbFileConstraint->hide();
    }
}

QDockWidget* MainWindow::getCompareTreeDock()
{
    return compareTreeDock;
}

QDockWidget* MainWindow::getTagTreeDock()
{
    return tagTreeDock;
}

QDockWidget* MainWindow::getBranchDock()
{
    return branchDock;
}

bool MainWindow::applyStyleSheetFile(QString _path)
{
    QFile styleFile(_path);

    if (styleFile.open(QFile::ReadOnly))
    {
        QString style(styleFile.readAll());
        setStyleSheet(style);
        styleFile.close();
        // remember style sheet file
        QSettings settings;
        settings.setValue("styleSheetFile", _path);
        gvtree_preferences.pbCssPath->setText(_path);
        return true;
    }
    return false;
}

void MainWindow::splitRepositoryPath(const QString& _input,
                                     QString& _pathTo,
                                     QString& _repoName)
{
    QFileInfo fi(_input);

    _pathTo = fi.absoluteFilePath();

    QStringList path_elements = _pathTo.split(QChar('/'));

    _repoName = path_elements.back();
    _pathTo.chop(_repoName.size());
}

void MainWindow::restorePreferencesSettings()
{
    QSettings settings;

    if (!settings.contains("tempPath"))
        settings.setValue("tempPath", "/tmp");
    gvtree_preferences.pbTempPath->setText(settings.value("tempPath").toString());

    if (settings.contains("styleSheetFile"))
        gvtree_preferences.pbCssPath->setText(settings.value("styleSheetFile").toString());

    if (!settings.contains("gitLogLines"))
        settings.setValue("gitLogLines", "1000");
    gvtree_preferences.git_log_lines->setText(settings.value("gitLogLines").toString());

    if (!settings.contains("codecForCStrings"))
        settings.setValue("codecForCStrings", "UTF-8");
    initCbCodecForCStrings(settings.value("codecForCStrings").toString());

    if (!settings.contains("comment_columns"))
        settings.setValue("comment_columns", 20); // nice default
    gvtree_preferences.comment_columns->setValue(settings.value("comment_columns").toInt());

    if (!settings.contains("comment_maxlen"))
        settings.setValue("comment_maxlen", 0); // unlimited
    gvtree_preferences.comment_maxlen->setValue(settings.value("comment_maxlen").toInt());

    if (!settings.contains("xfactor"))
        settings.setValue("xfactor", 30);
    gvtree_preferences.xfactor->setValue(settings.value("xfactor").toInt());

    if (!settings.contains("yfactor"))
        settings.setValue("yfactor", 20);
    gvtree_preferences.yfactor->setValue(settings.value("yfactor").toInt());

    if (!settings.contains("gitShortHashes"))
        settings.setValue("gitShortHashes", true);
    gvtree_preferences.git_short_hashes->setChecked(settings.value("gitShortHashes").toBool());

    if (!settings.contains("topDownView"))
        settings.setValue("topDownView", 0);
    gvtree_preferences.top_down_sort->setCurrentIndex(settings.value("topDownView").toInt());

    if (!settings.contains("horizontalSort"))
        settings.setValue("horizontalSort", 0);
    gvtree_preferences.horizontal_sort->setCurrentIndex(settings.value("horizontalSort").toInt());

    if (!settings.contains("includeSelected"))
        settings.setValue("includeSelected", false);
    gvtree_preferences.include_selected->setChecked(settings.value("includeSelected").toBool());

    if (!settings.contains("animated"))
        settings.setValue("animated", false);
    gvtree_preferences.animated->setChecked(settings.value("animated").toBool());

    if (!settings.contains("textborder"))
        settings.setValue("textborder", true);
    gvtree_preferences.textborder->setChecked(settings.value("textborder").toBool());

    if (settings.contains("diffLocalFile"))
        gvtree_preferences.diff_local_files->setChecked(settings.value("diffLocalFile").toBool());

    if (settings.contains("reduceTree"))
        gvtree_preferences.reduce_tree->setChecked(settings.value("reduceTree").toBool());

    if (settings.contains("printCmdToStdout"))
        gvtree_preferences.print_cmd_to_stdout->setChecked(settings.value("printCmdToStdout").toBool());

    if (settings.contains("connectorStyle"))
    {
        if (settings.value("connectorStyle").toInt())
            gvtree_preferences.rbConnectorStyle1->setChecked(true);
        else
            gvtree_preferences.rbConnectorStyle0->setChecked(true);
    }
    else
        gvtree_preferences.rbConnectorStyle1->setChecked(true);

    // switches for version folding
    if (settings.contains("fold_no_head") && settings.value("fold_no_head").toBool())
        gvtree_preferences.fold_no_head->setChecked(true);
    else
        gvtree_preferences.fold_no_head->setChecked(false);

    if (settings.contains("fold_no_branch") && settings.value("fold_no_branch").toBool())
        gvtree_preferences.fold_no_branch->setChecked(true);
    else
        gvtree_preferences.fold_no_branch->setChecked(false);

    if (settings.contains("fold_no_release") && settings.value("fold_no_release").toBool())
        gvtree_preferences.fold_no_release->setChecked(true);
    else
        gvtree_preferences.fold_no_release->setChecked(false);

    if (settings.contains("fold_no_baseline") && settings.value("fold_no_baseline").toBool())
        gvtree_preferences.fold_no_baseline->setChecked(true);
    else
        gvtree_preferences.fold_no_baseline->setChecked(false);

    if (settings.contains("fold_no_HO") && settings.value("fold_no_HO").toBool())
        gvtree_preferences.fold_no_HO->setChecked(true);
    else
        gvtree_preferences.fold_no_HO->setChecked(false);

    if (settings.contains("fold_no_FIXPQT") && settings.value("fold_no_FIXPQT").toBool())
        gvtree_preferences.fold_no_FIXPQT->setChecked(true);
    else
        gvtree_preferences.fold_no_FIXPQT->setChecked(false);

    if (settings.contains("fold_no_tag") && settings.value("fold_no_tag").toBool())
        gvtree_preferences.fold_no_tag->setChecked(true);
    else
        gvtree_preferences.fold_no_tag->setChecked(false);

    if (settings.contains("fold_not_pattern") && settings.value("fold_not_pattern").toBool())
        gvtree_preferences.fold_not_pattern->setChecked(true);
    else
        gvtree_preferences.fold_not_pattern->setChecked(false);

    if (settings.contains("fold_not_regexp"))
    {
        gvtree_preferences.fold_not_regexp->setText(settings.value("fold_not_regexp").toString());
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        foldNotRegExp = QRegularExpression(gvtree_preferences.fold_not_regexp->text());
#else
        foldNotRegExp = QRegExp(gvtree_preferences.fold_not_regexp->text());
#endif
    }

    if (settings.contains("defaultLastRepo")
        && settings.value("defaultLastRepo").toInt())
    {
        gvtree_preferences.rbLastRepo->setChecked(true);
        gvtree_preferences.rbCurrentPathRepo->setChecked(false);
    }
    else
    {
        settings.setValue("defaultLastRepo", 0);
        gvtree_preferences.rbLastRepo->setChecked(false);
        gvtree_preferences.rbCurrentPathRepo->setChecked(true);
    }
}

void MainWindow::restoreWindowSettings()
{
    // window settings
    QSettings settings;

    if (settings.contains("mainwindow/size"))
    {
        QSize size = settings.value("mainwindow/size", QSize(400, 400)).toSize();
        resize(size);
    }

    if (settings.contains("mainwindow/pos"))
    {
        QPoint pos = settings.value("mainwindow/pos", QPoint(200, 200)).toPoint();

        // We perform an initial move to (0,0) before actually moving
        // to our intended position. If this is omitted, the window manager
        // seems to not include window decorations into the rendering which
        // results in an effective negative x and/or y offset for the window.
        move(0, 0);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QRect rootRect;
        QList<QScreen*> screens = QGuiApplication::screens();
        if (screens.size() > 0)
        {
            QRect rootRect = screens.front()->availableGeometry();
            pos.setX((pos.x() < 0 ? 0 : pos.x()) % rootRect.width());
            pos.setY((pos.y() < 0 ? 0 : pos.y()) % rootRect.height());
            move(pos);
        }
#else
        // force negative top left position to visible screen area
        QRect rootRect = QApplication::desktop()->availableGeometry();
        pos.setX((pos.x() < 0 ? 0 : pos.x()) % rootRect.width());
        pos.setY((pos.y() < 0 ? 0 : pos.y()) % rootRect.height());
        move(pos);
#endif
    }

    if (settings.contains("mainwindow/state"))
    {
        QByteArray state = settings.value("mainwindow/state", QByteArray()).toByteArray();
        restoreState(state);
    }
}

void MainWindow::resetCurrentRepository()
{
    graphwidget->gitlog(true);
    refreshRepo->setEnabled(true);
}

void MainWindow::restoreLocalRepository()
{
    QSettings settings;

    // no -r specified...
    if (graphwidget->getLocalRepositoryPath().isEmpty())
    {
        // decide if
        bool repoFound = false;
        QString lrp;
        if (settings.contains("defaultLastRepo")
            && settings.value("defaultLastRepo").toInt()
            && settings.contains("localRepositoryPath"))
        {
            lrp = settings.value("localRepositoryPath").toString();
            repoFound = checkGitLocalRepository(lrp, repositoryPath, fileConstraintPath, true);
        }

        if (repoFound == false)
        {
            char* wdir = getcwd(NULL, 0);
            if (wdir == NULL)
            {
                cerr << "Allocation memory for getcwd failed." << endl;
                exit(-1);
            }

            lrp = QString(wdir);
            repoFound = checkGitLocalRepository(lrp, repositoryPath, fileConstraintPath, true);
            free(wdir);
        }

        if (repoFound == true)
        {
            QString path, fname;
            splitRepositoryPath(repositoryPath, path, fname);
            lbRepositoryPath->setText(path);
            pbRepositoryName->setText(fname);
            gvtree_preferences.pbLocalRepositoryPath->setText(repositoryPath);

            graphwidget->setLocalRepositoryPath(repositoryPath);
            gvtree_branchtable.branchTable->refresh(repositoryPath);
            graphwidget->gitlog();
            refreshRepo->setEnabled(true);
        }
        else
        {
            lbRepositoryPath->setText("");
            gvtree_preferences.pbLocalRepositoryPath->setText("");
            pbRepositoryName->setText("No repository set.");
            refreshRepo->setEnabled(false);
            // open repo dialog
            setGitLocalRepository();
        }
    }
}

void MainWindow::createMenus()
{
    // -- File
    filemenu = menuBar()->addMenu(tr("File"));

    QAction* setGitRepo = new QAction(tr("Set git repository"), this);

    setGitRepo->setStatusTip(tr("Set path to git repository."));
    connect(setGitRepo, SIGNAL(triggered()), this, SLOT(setGitLocalRepository()));
    connect(gvtree_preferences.pbLocalRepositoryPath, SIGNAL(pressed()), this, SLOT(setGitLocalRepository()));
    connect(gvtree_preferences.pbCssPath, SIGNAL(pressed()), this, SLOT(changeCssFilePath()));
    connect(gvtree_preferences.pbTempPath, SIGNAL(pressed()), this, SLOT(changeTempPath()));
    filemenu->addAction(setGitRepo);
    refreshRepo = new QAction(tr("Reload repository"), this);
    refreshRepo->setStatusTip(tr("Reload current repository. The selected version will stay in place. If no version is selected the whole tree will be displayed."));
    refreshRepo->setEnabled(false);
    connect(refreshRepo, SIGNAL(triggered()), this, SLOT(reloadCurrentRepository()));
    filemenu->addAction(refreshRepo);
    filemenu->addSeparator();

    QAction* action = new QAction(tr("Quit"), this);

    action->setStatusTip(tr("Exit gvtree..."));
    connect(action, SIGNAL(triggered()), this, SLOT(quit()));
    filemenu->addAction(action);

    // -- View
    viewmenu = menuBar()->addMenu(tr("View"));

    gridLayout = new TagPrefGridLayout();
    gridLayout->addTagPreference("HEAD", "(HEAD.*)", QString("#ffff0004"));
    gridLayout->addTagPreference("Commit Date", "", QString("#ff3584e4"));
    gridLayout->addTagPreference("User Name", "", QString("#ff1c71d8"));
    gridLayout->addTagPreference("Hash", "", QString("#ff26a269"));
    gridLayout->addTagPreference("Branch", "^((?!.*tag: )\\b([\\/0-9a-zA-Z_]*)\\b)$", QString("#ff5e5c64"), QString(), true);
    gridLayout->addTagPreference("Release Label", "tag: \\b(v[0-9.\\-]+)|(R[0-9.\\-]+(_RC[0-9]+)?)$", QString("#ffe01b24"), QString("Nimbus Sans L,18,-1,5,75,0,0,0,0,0"), true);
    gridLayout->addTagPreference("Baseline Label", "tag: \\b(BASELINE_[0-9.\\-]+)$", QString("#ff3d3846"), QString("Nimbus Sans L,18,-1,5,75,0,0,0,0,0"), true);
    gridLayout->addTagPreference("FIX/PQT Label", "tag: \\b(((FIX|PQT)_STR[0-9]+(DEV|DOC)?(_RR[0-9]+)?))$", QString(), QString(), true);
    gridLayout->addTagPreference("HO Label", "tag: \\b(STR[0-9]+(DEV|DOC)?_HO[0-9]*)$", QString("#ff5e5c64"), QString(), true);
    gridLayout->addTagPreference("Other Tags", "tag: \\b(.*)$", QString("#ff000000"));
    gridLayout->addTagPreference("Comment", "", QString("#ff63452c"), QString("Nimbus Sans,8,-1,5,50,0,0,0,0,0"));
    gvtree_preferences.verticalLayout_3->addLayout(gridLayout);

    connect(gridLayout, SIGNAL(regexpChanged()), this, SLOT(resetCurrentRepository()));

    connect(gvtree_preferences.pbOK, SIGNAL(pressed()), this, SLOT(saveChangedSettings()));

    gvtree_preferences.mimeTypesTable->fromSettings();

    // -- Windows
    windowmenu = menuBar()->addMenu(tr("Windows"));
    preferencesAct = new QAction(tr("Preferences"), this);
    connect(preferencesAct, SIGNAL(triggered()), pwin, SLOT(show()));
    windowmenu->addAction(preferencesAct);
    windowmenu->addSeparator();

    nodeInfo << "HEAD" << "Commit Date" << "User Name" << "Hash" << "Branch" << "Release Label" << "Baseline Label" << "FIX/PQT Label" << "HO Label" << "Other Tags" << "Comment";

    QSettings settings;

    foreach (const QString& it, nodeInfo)
    {
        QAction* item = new QAction(it, this);

        item->setCheckable(true);

        if (settings.contains(it) == false)
        {
            settings.setValue(it, true);
        }

        bool item_state = settings.value(it).toBool();

        item->setChecked(item_state);
        graphwidget->setGlobalVersionInfo(it, item_state);
        viewmenu->addAction(item);
    }
    connect(viewmenu, SIGNAL(triggered(QAction*)),
            this, SLOT(changeGlobalVersionInfo(QAction*)));
    viewmenu->addSeparator();
    action = viewmenu->addAction(QString("Fit in view"));
    connect(action, SIGNAL(triggered()), graphwidget, SLOT(fitInView()));

    // -- Help
    helpmenu = menuBar()->addMenu(tr("Help"));
    helpAct = new QAction(tr("Help"), this);
    connect(helpAct, SIGNAL(triggered()), this, SLOT(helpDialog()));
    aboutAct = new QAction(tr("About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));
    licenseAct = new QAction(tr("License"), this);
    connect(licenseAct, SIGNAL(triggered()), this, SLOT(licenseDialog()));
    helpmenu->addAction(helpAct);
    helpmenu->addSeparator();
    helpmenu->addAction(aboutAct);
    helpmenu->addAction(licenseAct);

    // color preferences...
    connect(gvtree_preferences.pbColorBackground, SIGNAL(pressed()), this, SLOT(colorDialogBackground()));
    connect(gvtree_preferences.pbColorSelected, SIGNAL(pressed()), this, SLOT(colorDialogSelected()));
    connect(gvtree_preferences.pbColorSearch, SIGNAL(pressed()), this, SLOT(colorDialogSearch()));
    connect(gvtree_preferences.pbColorVersion, SIGNAL(pressed()), this, SLOT(colorDialogVersion()));
    connect(gvtree_preferences.pbColorEdge, SIGNAL(pressed()), this, SLOT(colorDialogEdge()));
    connect(gvtree_preferences.pbColorMerge, SIGNAL(pressed()), this, SLOT(colorDialogMerge()));
    connect(gvtree_preferences.pbColorFolded, SIGNAL(pressed()), this, SLOT(colorDialogFolded()));
    connect(gvtree_preferences.pbColorUnfolded, SIGNAL(pressed()), this, SLOT(colorDialogUnfolded()));
    connect(gvtree_preferences.pbColorFromTo, SIGNAL(pressed()), this, SLOT(colorDialogFromTo()));
    connect(gvtree_preferences.pbColorFileConstraint, SIGNAL(pressed()), this, SLOT(colorDialogFileConstraint()));
}

void MainWindow::changeGlobalVersionInfo(QAction* _act)
{
    graphwidget->setGlobalVersionInfo(_act->text(), _act->isChecked());
    QSettings settings;

    settings.setValue(_act->text(), _act->isChecked());
    graphwidget->forceUpdate();
}

void MainWindow::removeFilter()
{
    graphwidget->removeFilter();
    pbFileConstraint->setText("");
    pbFileConstraint->hide();
}

void MainWindow::reloadCurrentRepository()
{
    QPoint restoreVersionPosition;
    QTransform restoreTransform;
    QString restoreVersionHash;

    // if there is one selected version, keep it in place
    Version* restoreVersion = graphwidget->getSelectedVersion();

    if (restoreVersion)
    {
        // hash to find it after reload
        restoreVersionHash = restoreVersion->getHash();

        // scene position of the selected object
        QPointF spos = restoreVersion->pos();

        // position in current view rectangle, left top corner is (0,0)
        restoreVersionPosition = graphwidget->mapFromScene(spos);

        // save transformation for scaling m11 m22
        restoreTransform = graphwidget->transform();
    }

    graphwidget->gitlog();

    if (restoreVersion)
    {
        // get the version again
        restoreVersion = graphwidget->getVersionByHash(restoreVersionHash);

        if (restoreVersion)
        {
            // restore transformation
            graphwidget->setTransform(QTransform(restoreTransform.m11(), 0, 0, restoreTransform.m22(), 0, 0));
            // reset view to position (0,0)
            graphwidget->horizontalScrollBar()->setValue(0);
            graphwidget->verticalScrollBar()->setValue(0);

            // Get the new position of the former selected version
            // in view coordinates and get the difference to know
            // how to shift the (invisible) scrollbars.
            QPoint shift = graphwidget->mapFromScene(restoreVersion->pos()) - restoreVersionPosition;
            graphwidget->horizontalScrollBar()->setValue(shift.x());
            graphwidget->verticalScrollBar()->setValue(shift.y());
        }
    }
    updateGitStatus(repositoryPath);
    pbRepositoryRefresh->hide();
}

void MainWindow::changeCssFilePath()
{
    QFileDialog dialog(this, tr("Change Path to CSS Style Sheet File"), gvtree_preferences.pbCssPath->text());

    dialog.setFilter(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllEntries);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec())
    {
        applyStyleSheetFile(dialog.selectedFiles().at(0));
    }
}

void MainWindow::changeTempPath()
{
    QFileDialog dialog(this, tr("Change Path for Temporary Files"), gvtree_preferences.pbTempPath->text());

    dialog.setFilter(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllDirs);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec())
    {
        QStringList directoryNames = dialog.selectedFiles();
        QString selDir = directoryNames.at(0);

        if (checkForValidTempPath(selDir))
        {
            gvtree_preferences.pbTempPath->setText(selDir);
            QSettings settings;
            settings.setValue("tempPath", selDir);
        }
    }
}

bool MainWindow::checkForValidTempPath(const QString& _tmppath)
{
    QFileInfo fi(_tmppath);

    return (_tmppath == fi.absoluteFilePath()
            && fi.permission(QFile::WriteUser | QFile::ReadUser));
}

void MainWindow::setGitLocalRepository()
{
    QFileDialog dialog(this, tr("Change Local git Repository Path"), gvtree_preferences.pbLocalRepositoryPath->text());

    QString oldRepositoryPath = repositoryPath;

    dialog.setFilter(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllDirs);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec())
    {
        QStringList directoryNames = dialog.selectedFiles();
        QString selDir = directoryNames.at(0);
        if (checkGitLocalRepository(directoryNames.at(0), repositoryPath, fileConstraintPath, false))
        {
            QSettings settings;
            settings.setValue("localRepositoryPath", repositoryPath);
            graphwidget->setLocalRepositoryPath(repositoryPath);
            gvtree_branchtable.branchTable->refresh(repositoryPath);
            gvtree_comparetree.compareTree->resetCompareTree();

            QString path, fname;
            splitRepositoryPath(repositoryPath, path, fname);
            lbRepositoryPath->setText(path);
            pbRepositoryName->setText(fname);
            gvtree_preferences.pbLocalRepositoryPath->setText(repositoryPath);

            graphwidget->gitlog(oldRepositoryPath != repositoryPath);
            graphwidget->updateColors();
            refreshRepo->setEnabled(true);
        }
    }
}

bool MainWindow::checkGitLocalRepository(const QString& _path,
                                         QString& _repoPath,
                                         QString& _fileConstraintPath,
                                         bool _silent)
{
    // simplify: get absolute path
    QFileInfo fi(_path);
    QString path = fi.absoluteFilePath();

    // search for .git directory ...
    bool success = false;
    QStringList path_elements = path.split(QChar('/'));

    QStringList check_for_repo;

    QString lookup_path;

    foreach(const QString& str, path_elements)
    {
        lookup_path = lookup_path + str + QString("/");
        QString lookup_git = lookup_path + QString(".git");

        check_for_repo.push_front(lookup_git);
    }

    foreach(const QString& str, check_for_repo)
    {
        QFileInfo fi(str);

        if (fi.exists())
        {
            success = true;
            lookup_path = str;
            lookup_path.chop(5); // ... remove "/.git" from path
            break;
        }
    }

    if (success == false)
    {
        if (_silent == false)
        {
            QMessageBox msgBox;
            QString message = "The given path \"" + _path + "\" does not contain a local git repository.";
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
        return false;
    }
    else
    {
        _repoPath = lookup_path;
        _fileConstraintPath = path.mid(_repoPath.size());
    }

    updateGitStatus(_repoPath);

    return true;
}

void MainWindow::helpDialog()
{
    QMessageBox help(QMessageBox::Information, tr("Help"), tr(""), QMessageBox::NoButton, this);
    QString msg = "For more detailed information, please refer to<br/><br/><b><i>"
        + QString(INSTALLATION_PATH) + "/share/doc/gvtree/"
        + QString(VERSION_NAME) + ".pdf</b></i>";

    help.information(this, "Help", msg);
}

void MainWindow::licenseDialog()
{
    QDialog* lDialog = new QDialog;
    QString css = "background-color: white; color: black;";

    lDialog->setStyleSheet(css);

    gvtree_license.setupUi(lDialog);
    connect(gvtree_license.pbOK, SIGNAL(clicked()), lDialog, SLOT(close()));

    QFile file(":/doc/GNU_GPL_v3.0.html");

    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);

    gvtree_license.textBrowser->setHtml(stream.readAll());
    file.close();

    lDialog->exec();
}

void MainWindow::aboutDialog()
{
    QMessageBox about(QMessageBox::Information, tr("About"), tr(""), QMessageBox::NoButton, this);
    QString css = "background-color: white; color: black;";

    about.setStyleSheet(css);

    about.setIconPixmap(QPixmap(":/images/gvtree_title_512.png"));
    about.exec();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

CompareTree* MainWindow::getCompareTree()
{
    return gvtree_comparetree.compareTree;
}

QTextEdit* MainWindow::getCompareTreeSelectedLog() const
{
    return gvtree_comparetree.selectedLog;
}

QTextEdit* MainWindow::getCompareTreeFromTextEdit()
{
    return gvtree_comparetree.fromEdit;
}

QTextEdit* MainWindow::getCompareTreeToTextEdit()
{
    return gvtree_comparetree.toEdit;
}

QPushButton* MainWindow::getCompareTreeToPushButton()
{
    return gvtree_comparetree.toButton;
}

QPushButton* MainWindow::getCompareTreeFromPushButton()
{
    return gvtree_comparetree.fromButton;
}

const TagPreference* MainWindow::getTagPreference(const QString& _key) const
{
    return gridLayout->getTagPreference(_key);
}

void MainWindow::closeEvent(QCloseEvent* _event)
{
    // save window settings
    QSettings settings;

    settings.setValue("mainwindow/pos", pos());
    settings.setValue("mainwindow/size", size());
    settings.setValue("mainwindow/state", saveState());

    doCleanupFiles();

    QMainWindow::closeEvent(_event);
}

void MainWindow::quit()
{
    QSettings settings;

    settings.setValue("mainwindow/pos", pos());
    settings.setValue("mainwindow/size", size());
    settings.setValue("mainwindow/state", saveState());

    doCleanupFiles();

    QCoreApplication::exit(0);
}

TagTree* MainWindow::getTagTree() const
{
    return tagtree;
}

void MainWindow::getMimeTypeTools(const QString& _mimeType,
                                  QString& _diff,
                                  QString& _edit)
{
    _diff = QString("gvim -d %1");
    _edit = QString("gvim %1");

    // if entry exists...
    if (gvtree_preferences.mimeTypesTable->get(_mimeType, _diff, _edit))
        return;

    QDialog* mimeTypeDialog = new QDialog;

    gvtree_difftool.setupUi(mimeTypeDialog);
    gvtree_difftool.mimeType->setText(_mimeType);
    gvtree_difftool.leDiffTool->setText(_diff);
    gvtree_difftool.leEditTool->setText(_edit);
    connect(gvtree_difftool.pbOK, SIGNAL(clicked()), mimeTypeDialog, SLOT(accept()));
    connect(gvtree_difftool.pbCancel, SIGNAL(clicked()), mimeTypeDialog, SLOT(reject()));

    if (mimeTypeDialog->exec() == QDialog::Accepted)
    {
        gvtree_preferences.mimeTypesTable->insert(
            _mimeType,
            gvtree_difftool.leDiffTool->text(),
            gvtree_difftool.leEditTool->text());
        _diff = gvtree_difftool.leDiffTool->text();
        _edit = gvtree_difftool.leEditTool->text();
    }
}

void MainWindow::addToCleanupFiles(const QString& _path)
{
    cleanupFiles.insert(_path);
}

void MainWindow::doCleanupFiles()
{
    foreach(const QString& str, cleanupFiles)
    {
        // tempPath must contain an absolute path
        if (str[0].toLatin1() == '/')
            unlink(str.toUtf8().data());
    }
}

int MainWindow::getMaxLines() const
{
    return gvtree_preferences.git_log_lines->text().toInt();
}

bool MainWindow::getShortHashes() const
{
    return gvtree_preferences.git_short_hashes->isChecked();
}

bool MainWindow::getReduceTree() const
{
    return gvtree_preferences.reduce_tree->isChecked();
}

bool MainWindow::getTopDownView() const
{
    return gvtree_preferences.top_down_sort->currentIndex() > 0;
}

int MainWindow::getHorizontalSort() const
{
    return gvtree_preferences.horizontal_sort->currentIndex();
}

bool MainWindow::getRemotes() const
{
    return cbRemotes->isChecked();
}

bool MainWindow::getAll() const
{
    return cbAll->isChecked();
}

bool MainWindow::getIncludeSelected() const
{
    return gvtree_preferences.include_selected->isChecked();
}

bool MainWindow::getAnimated() const
{
    return gvtree_preferences.animated->isChecked();
}

bool MainWindow::getTextBorder() const
{
    return gvtree_preferences.textborder->isChecked();
}

bool MainWindow::getDiffLocalFiles() const
{
    return gvtree_preferences.diff_local_files->isChecked();
}

void MainWindow::remotesChanged(int _val)
{
    QSettings settings;

    settings.setValue("remotes", _val);
    graphwidget->preferencesUpdated(true);
}

void MainWindow::allChanged(int _val)
{
    QSettings settings;

    settings.setValue("all", _val);
    graphwidget->preferencesUpdated(true);
}

void MainWindow::saveChangedSettings()
{
    QSettings settings;

    settings.setValue("gitLogLines", gvtree_preferences.git_log_lines->text());

    bool forceUpdate = false;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QString codec = gvtree_preferences.cbCodecForCStrings->currentText();
    if (settings.value("codecForCStrings").toString() != codec)
    {
        settings.setValue("codecForCStrings", codec);
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName(codec.toUtf8().data()));
        forceUpdate = true;
    }
#endif
    settings.setValue("topDownView", gvtree_preferences.top_down_sort->currentIndex());
    settings.setValue("horizontalSort", gvtree_preferences.horizontal_sort->currentIndex());
    settings.setValue("gitShortHashes", gvtree_preferences.git_short_hashes->isChecked());
    settings.setValue("includeSelected", gvtree_preferences.include_selected->isChecked());
    settings.setValue("animated", gvtree_preferences.animated->isChecked());
    settings.setValue("textborder", gvtree_preferences.textborder->isChecked());
    settings.setValue("diffLocalFile", gvtree_preferences.diff_local_files->isChecked());
    settings.setValue("reduceTree", gvtree_preferences.reduce_tree->isChecked());

    forceUpdate = forceUpdate || !settings.contains("fold_no_tag")
        || settings.value("fold_no_tag").toBool() != gvtree_preferences.fold_no_tag->isChecked()
        || !settings.contains("fold_no_head")
        || settings.value("fold_no_head").toBool() != gvtree_preferences.fold_no_head->isChecked()
        || !settings.contains("fold_no_branch")
        || settings.value("fold_no_branch").toBool() != gvtree_preferences.fold_no_branch->isChecked()
        || !settings.contains("fold_no_baseline")
        || settings.value("fold_no_baseline").toBool() != gvtree_preferences.fold_no_baseline->isChecked()
        || !settings.contains("fold_no_release")
        || settings.value("fold_no_release").toBool() != gvtree_preferences.fold_no_release->isChecked()
        || !settings.contains("fold_no_FIXPQT")
        || settings.value("fold_no_FIXPQT").toBool() != gvtree_preferences.fold_no_FIXPQT->isChecked()
        || !settings.contains("fold_no_HO")
        || settings.value("fold_no_HO").toBool() != gvtree_preferences.fold_no_HO->isChecked()
        || !settings.contains("fold_not_pattern")
        || settings.value("fold_not_pattern").toBool() != gvtree_preferences.fold_not_pattern->isChecked()
        || !settings.contains("fold_not_regexp")
        || settings.value("fold_not_regexp").toString() != gvtree_preferences.fold_not_regexp->text();

    settings.setValue("fold_no_tag", gvtree_preferences.fold_no_tag->isChecked());
    settings.setValue("fold_no_head", gvtree_preferences.fold_no_head->isChecked());
    settings.setValue("fold_no_branch", gvtree_preferences.fold_no_branch->isChecked());
    settings.setValue("fold_no_baseline", gvtree_preferences.fold_no_baseline->isChecked());
    settings.setValue("fold_no_release", gvtree_preferences.fold_no_release->isChecked());
    settings.setValue("fold_no_FIXPQT", gvtree_preferences.fold_no_FIXPQT->isChecked());
    settings.setValue("fold_no_HO", gvtree_preferences.fold_no_HO->isChecked());
    settings.setValue("fold_not_pattern", gvtree_preferences.fold_not_pattern->isChecked());
    settings.setValue("fold_not_regexp", gvtree_preferences.fold_not_regexp->text());
    gvtree_preferences.fold_not_regexp->setText(settings.value("fold_not_regexp").toString());
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    foldNotRegExp = QRegularExpression(gvtree_preferences.fold_not_regexp->text());
#else
    foldNotRegExp = QRegExp(gvtree_preferences.fold_not_regexp->text());
#endif

    settings.setValue("connectorStyle", getConnectorStyle());
    settings.setValue("defaultLastRepo", gvtree_preferences.rbLastRepo->isChecked() ? 1 : 0);
    settings.setValue("printCmdToStdout", gvtree_preferences.print_cmd_to_stdout->isChecked());

    int xval = settings.value("xfactor").toInt();
    int yval = settings.value("yfactor").toInt();

    if (xval != gvtree_preferences.xfactor->value()
        || yval != gvtree_preferences.yfactor->value())
    {
        settings.setValue("xfactor", gvtree_preferences.xfactor->value());
        settings.setValue("yfactor", gvtree_preferences.yfactor->value());
    }

    int comment_columns = settings.value("comment_columns").toInt();
    if (comment_columns != gvtree_preferences.comment_columns->value())
    {
        settings.setValue("comment_columns", gvtree_preferences.comment_columns->value());
    }

    int comment_maxlen = settings.value("comment_maxlen").toInt();
    if (comment_maxlen != gvtree_preferences.comment_maxlen->value())
    {
        settings.setValue("comment_maxlen", gvtree_preferences.comment_maxlen->value());
    }

    pwin->hide();

    graphwidget->preferencesUpdated(forceUpdate);
}

void MainWindow::getCommentProperties(int& _columns, int& _limit) const
{
    QSettings settings;

    _columns = settings.value("comment_columns").toInt();
    _limit = settings.value("comment_maxlen").toInt();
}

bool MainWindow::getXYFactor(int& _xfactor, int& _yfactor) const
{
    QSettings settings;
    int xval = settings.value("xfactor").toInt();
    int yval = settings.value("yfactor").toInt();
    bool result = xval != _xfactor || yval != _yfactor;

    _xfactor = xval;
    _yfactor = yval;
    return result;
}

int MainWindow::getConnectorStyle() const
{
    return gvtree_preferences.rbConnectorStyle1->isChecked();
}

void MainWindow::restoreColorSettings()
{
    restoreColorSettingsHelper(gvtree_preferences.pbColorBackground, QString("colorBackground"), QColor("#ffdeddda"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorVersion, QString("colorVersion"), QColor("#ffffb000"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorEdge, QString("colorEdge"), QColor("#ff404040"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorMerge, QString("colorMerge"), QColor("#ff4040b0"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorSearch, QString("colorSearch"), QColor("#ff1080d0"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorSelected, QString("colorSelected"), QColor("#ffff0000"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorFolded, QString("colorFolded"), QColor("#ff9a9996"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorUnfolded, QString("colorUnfolded"), QColor("#ffc0c0c0"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorFromTo, QString("colorFromTo"), QColor("#ff62a0ea"));
    restoreColorSettingsHelper(gvtree_preferences.pbColorFileConstraint, QString("colorFileConstraint"), QColor("#ffff0000"));
}

void MainWindow::restoreColorSettingsHelper(
    QPushButton* _pb,
    QString _settingsKey,
    QColor _defaultColor,
    bool _overwrite)
{
    QSettings settings;
    QColor tmpColor = _defaultColor;

    if (settings.contains(_settingsKey) && _overwrite == false)
        tmpColor = settings.value(_settingsKey).value<QColor>();
    else
        settings.setValue(_settingsKey, QVariant(tmpColor));

    QColor fgColor = QColor(0, 0, 0);

    if (tmpColor.lightness() < 128)
        fgColor = QColor(255, 255, 255);
    QString css = "background-color: " + tmpColor.name() + "; color: " + fgColor.name() + ";";

    _pb->setStyleSheet(css);
}

void MainWindow::colorDialogCommon(QString _key, QPushButton* _pb)
{
    QSettings settings;
    QColor tmpColor = settings.value(_key).value<QColor>();

    tmpColor = QColorDialog::getColor(tmpColor);
    if (tmpColor.isValid())
    {
        restoreColorSettingsHelper(_pb, _key, tmpColor, true);
    }
}

void MainWindow::colorDialogBackground()
{
    colorDialogCommon("colorBackground", gvtree_preferences.pbColorBackground);
}

void MainWindow::colorDialogFolded()
{
    colorDialogCommon("colorFolded", gvtree_preferences.pbColorFolded);
}

void MainWindow::colorDialogFromTo()
{
    colorDialogCommon("colorFromTo", gvtree_preferences.pbColorFromTo);
}

void MainWindow::colorDialogUnfolded()
{
    colorDialogCommon("colorUnfolded", gvtree_preferences.pbColorUnfolded);
}

void MainWindow::colorDialogVersion()
{
    colorDialogCommon("colorVersion", gvtree_preferences.pbColorVersion);
}

void MainWindow::colorDialogEdge()
{
    colorDialogCommon("colorEdge", gvtree_preferences.pbColorEdge);
}

void MainWindow::colorDialogMerge()
{
    colorDialogCommon("colorMerge", gvtree_preferences.pbColorMerge);
}

void MainWindow::colorDialogSelected()
{
    colorDialogCommon("colorSelected", gvtree_preferences.pbColorSelected);
}

void MainWindow::colorDialogSearch()
{
    colorDialogCommon("colorSearch", gvtree_preferences.pbColorSearch);
}

void MainWindow::colorDialogFileConstraint()
{
    colorDialogCommon("colorFileConstraint", gvtree_preferences.pbColorFileConstraint);
}

QColor MainWindow::getPreferencesColor(QString _key)
{
    QSettings settings;

    return settings.value(_key).value<QColor>();
}

QComboBox* MainWindow::getFromComboBox()
{
    return gvtree_comparetree.fromComboBox;
}

QLabel* MainWindow::getToDateLabel()
{
    return gvtree_comparetree.toDate;
}

QString MainWindow::getTempPath() const
{
    return gvtree_preferences.pbTempPath->text();
}

bool MainWindow::getPrintCmdToStdout() const
{
    return gvtree_preferences.print_cmd_to_stdout->isChecked();
}

Ui_Dialog& MainWindow::getPreferences()
{
    return gvtree_preferences;
}

void MainWindow::showRefreshButton(const QString&)
{
    pbRepositoryRefresh->show();
}

void MainWindow::updateGitStatus(const QString& _repoPath)
{
// set watchdog
    if (watcher)
    {
        if (watcher->directories().size())
            watcher->removePaths(watcher->directories());
        if (watcher->files().size())
            watcher->removePaths(watcher->files());
    }

    gitstatus->clear();

    // get data
    QString cmd = "git -C " + _repoPath + " status";
    QList<QString> cache;

    execute_cmd(cmd.toUtf8().data(), cache, getPrintCmdToStdout());
    foreach(const QString& str, cache)
    {
        gitstatus->insertPlainText(str);
    }
    gitstatus->moveCursor(QTextCursor::Start);

    if (watcher)
    {
        QString gitpath = _repoPath + "/.git";
        watcher->addPath(gitpath);
    }
}

bool MainWindow::initCbCodecForCStrings(QString _default)
{
    gvtree_preferences.cbCodecForCStrings->clear();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_UNUSED(_default);
    gvtree_preferences.cbCodecForCStrings->addItem(QString("UTF-8"));
    gvtree_preferences.cbCodecForCStrings->setCurrentIndex(0);
#else
    gvtree_preferences.cbCodecForCStrings->addItem(QString());
    foreach (QByteArray codec, QTextCodec::availableCodecs())
    {
        gvtree_preferences.cbCodecForCStrings->addItem(QString(codec));
    }

    int index = gvtree_preferences.cbCodecForCStrings->findText(_default, Qt::MatchExactly);
    if (index < 0)
    {
        gvtree_preferences.cbCodecForCStrings->setCurrentIndex(0);
        return false;
    }

    gvtree_preferences.cbCodecForCStrings->setCurrentIndex(index);
#endif
    return true;
}

QString MainWindow::getSelectedBranch()
{
    return gvtree_branchtable.branchTable->getSelectedBranch();
}

const QStringList& MainWindow::getNodeInfo() const
{
    return nodeInfo;
}

bool MainWindow::getVersionIsFoldable(const QMap<QString, QStringList>& _keyinformation) const
{
    bool fold_no_head = gvtree_preferences.fold_no_head->isChecked();
    bool fold_no_branch = gvtree_preferences.fold_no_branch->isChecked();
    bool fold_no_release = gvtree_preferences.fold_no_release->isChecked();
    bool fold_no_baseline = gvtree_preferences.fold_no_baseline->isChecked();
    bool fold_no_HO = gvtree_preferences.fold_no_HO->isChecked();
    bool fold_no_FIXPQT = gvtree_preferences.fold_no_FIXPQT->isChecked();
    bool fold_no_tag = gvtree_preferences.fold_no_tag->isChecked();
    bool fold_not_pattern = gvtree_preferences.fold_not_pattern->isChecked();

    for (QMap<QString, QStringList>::const_iterator it = _keyinformation.begin();
         it != _keyinformation.end();
         it++)
    {
        if (it.value().size() > 0)
        {
            if (it.key() == "HEAD" && (fold_no_tag || fold_no_head))
                return false;

            if (it.key() == "Release Label" && (fold_no_tag || fold_no_release))
                return false;

            if (it.key() == "Baseline Label" && (fold_no_tag || fold_no_baseline))
                return false;

            if (it.key() == "Branch" && (fold_no_tag || fold_no_branch))
                return false;

            if (it.key() == "FIX/PQT Label" && (fold_no_tag || fold_no_FIXPQT))
                return false;

            if (it.key() == "HO Label" && (fold_no_tag || fold_no_HO))
                return false;

            if (it.key() == "Other Tags" && fold_no_tag)
            {
                return false;
            }
            if (fold_not_pattern && foldNotRegExp.isValid())
            {
                foreach(const QString& str, it.value())
                {
                    if (!foldNotRegExp.match(str).hasMatch())
                        return false;
                }
            }
        }
    }
    return true;
}

GraphWidget* MainWindow::getGraphWidget()
{
    return graphwidget;
}
