/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.1-0                */
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

#include <QStatusBar>
#include <QWidget>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QTextStream>
#include <QScrollBar>
#include <QDockWidget>
#include <QtAlgorithms>
#include <QFileDialog>
#include <QCoreApplication>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QSettings>
#include <QDockWidget>
#include <QTextBrowser>

#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>

#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow(const QStringList& _argv) : QMainWindow(NULL), ctwin(NULL), pwin(NULL)
{
    // setup preferences dialog first
    pwin = new QDialog;
    gvtree_preferences.setupUi(pwin);
    restorePreferencesSettings();
    restoreColorSettings();

    // watchdog for the local repository directory
    watcher = new QFileSystemWatcher();
    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(directoryChanged(const QString&)));
    connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));

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

    // -- create browser for tags and branches
    tagwidget = new TagWidget(this);
    dock = new QDockWidget(tr("Version Information"), this);
    dock->setObjectName("Version Information");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(tagwidget);
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

    dock = new QDockWidget(tr("Compare Files"), this);
    dock->setObjectName("Compare Files");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(ctwin);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    windowmenu->addAction(dock->toggleViewAction());
    compareTreeDock = dock;
    dock->hide();

    // -- line dialog to search nodes by hash, date, tag or branch information
    search = new QLineEdit(this);
    connect(search, SIGNAL(textEdited(const QString&)), this, SLOT(lookupId(const QString&)));

    dock = new QDockWidget(tr("Search Version"), this);
    dock->setObjectName("Search Version");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(search);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    windowmenu->addAction(dock->toggleViewAction());
    searchDock = dock;
    dock->hide();

    connect(gvtree_comparetree.fromComboBox, SIGNAL(currentIndexChanged(int)),
            gvtree_comparetree.compareTree, SLOT(currentIndexChanged(int)));
    connect(gvtree_comparetree.toButton, SIGNAL(pressed()), graphwidget, SLOT(focusToVersion()));
    connect(gvtree_comparetree.fromButton, SIGNAL(pressed()), graphwidget, SLOT(focusFromVersion()));

    show();
    restoreWindowSettings();

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

                QString path, fname;
                splitRepositoryPath(repositoryPath, path, fname);
                lbRepositoryPath->setText(path);
                pbRepositoryName->setText(fname);
                gvtree_preferences.pbLocalRepositoryPath->setText(repositoryPath);
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
            cout << "gvtree V1.1-0" << endl;
        }
        else if (_argv.at(i) == "--css")
        {
            ++i;
            applyStyleSheetFile(QString(_argv.at(i)));
        }
        else if (_argv.at(i) == "-h")
        {
            cout << "gvtree V1.1-0" << endl;
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
            cout << "   Set a file constraint. Only the version tree of the file will" << endl;
            cout << "   be displayed." << endl;
            cout << endl;
            cout << "-r [local git repository directory]" << endl;
            cout << "   If not specified the current path is checked for a valid repository" << endl;
            cout << "   or the repository used in the previous session is displayed." << endl;
            cout << "   Which one to use can be controlled by preferences setting." << endl;
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
            cout << "     git log --graph --decorate --pretty=\"#%h#%an#%at#%d#\"" << endl;
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

QDockWidget* MainWindow::getSearchDock()
{
    return searchDock;
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

    if (!settings.contains("gitLogLines"))
        settings.setValue("gitLogLines", "1000");
    gvtree_preferences.git_log_lines->setText(settings.value("gitLogLines").toString());

    if (!settings.contains("xfactor"))
        settings.setValue("xfactor", 10);
    gvtree_preferences.xfactor->setValue(settings.value("xfactor").toInt());

    if (!settings.contains("yfactor"))
        settings.setValue("yfactor", 10);
    gvtree_preferences.yfactor->setValue(settings.value("yfactor").toInt());

    if (!settings.contains("gitShortHashes"))
        settings.setValue("gitShortHashes", false);
    gvtree_preferences.git_short_hashes->setChecked(settings.value("gitShortHashes").toBool());

    if (!settings.contains("topDownView"))
        settings.setValue("topDownView", false);
    gvtree_preferences.top_down_view->setChecked(settings.value("topDownView").toBool());

    if (!settings.contains("openGLRendering"))
        settings.setValue("openGLRendering", false);
    gvtree_preferences.open_gl_rendering->setChecked(settings.value("openGLRendering").toBool());

    if (settings.contains("diffLocalFile"))
        gvtree_preferences.diff_local_files->setChecked(settings.value("diffLocalFile").toBool());

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
        gvtree_preferences.rbConnectorStyle0->setChecked(true);

    if (settings.contains("remotes") && settings.value("remotes").toBool())
        gvtree_preferences.remotes->setChecked(true);
    else
        gvtree_preferences.remotes->setChecked(false);

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

        // force negative top left position to visible screen area
        QRect rootRect = QApplication::desktop()->availableGeometry();
        pos.setX((pos.x() < 0 ? 0 : pos.x()) % rootRect.width());
        pos.setY((pos.y() < 0 ? 0 : pos.y()) % rootRect.height());
        move(pos);
    }

    if (settings.contains("mainwindow/state"))
    {
        QByteArray state = settings.value("mainwindow/state", QByteArray()).toByteArray();
        restoreState(state);
    }
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
        }

        if (repoFound == true)
        {
            QString path, fname;
            splitRepositoryPath(repositoryPath, path, fname);
            lbRepositoryPath->setText(path);
            pbRepositoryName->setText(fname);
            gvtree_preferences.pbLocalRepositoryPath->setText(repositoryPath);

            graphwidget->setLocalRepositoryPath(repositoryPath);
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

    gridLayout->addTagPreference("HEAD", "(HEAD.*)");
    gridLayout->addTagPreference("Commit Date", "([0-9]+)");
    gridLayout->addTagPreference("User Name", "\\[([0-9a-zA-Z ]*)\\]");
    gridLayout->addTagPreference("Hash", "([0-9a-f]+)");
    gridLayout->addTagPreference("Branch", "^((?!.*tag: )\\b([\\/0-9a-zA-Z_]*)\\b)$");
    gridLayout->addTagPreference("Release Label", "tag: \\b(R[0-9.\\-]+)$");
    gridLayout->addTagPreference("Baseline Label", "tag: \\b(BASELINE_[0-9.\\-]+)$");
    gridLayout->addTagPreference("FIX/PQT Label", "tag: \\b((FIX_STR[0-9]+)|(PQT_STR[0-9]+))$");
    gridLayout->addTagPreference("HO Label", "tag: \\b(STR[0-9]+_HO[0-9]*)$");
    gridLayout->addTagPreference("Other Tags", "");
    gvtree_preferences.verticalLayout_3->addLayout(gridLayout);

    connect(gvtree_preferences.pbOK, SIGNAL(pressed()), this, SLOT(saveChangedSettings()));

    gvtree_preferences.mimeTypesTable->fromSettings();

    // -- Windows
    windowmenu = menuBar()->addMenu(tr("Windows"));
    preferencesAct = new QAction(tr("Preferences"), this);
    connect(preferencesAct, SIGNAL(triggered()), pwin, SLOT(show()));
    windowmenu->addAction(preferencesAct);
    windowmenu->addSeparator();

    QSettings settings;

    QStringList nodeInfo;
    nodeInfo << "HEAD" << "Commit Date" << "User Name" << "Hash" << "Branch" << "Release Label" << "Baseline Label" << "FIX/PQT Label" << "HO Label" << "Other Tags";

    foreach (const QString it, nodeInfo)
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
    helpmenu->addAction(aboutAct);
    helpmenu->addAction(licenseAct);

    // color preferences...
    connect(gvtree_preferences.pbColorBackground, SIGNAL(pressed()), this, SLOT(colorDialogBackground()));
    connect(gvtree_preferences.pbColorText, SIGNAL(pressed()), this, SLOT(colorDialogText()));
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

            // restore selection
            restoreVersion->setSelected(true);
        }
    }
    pbRepositoryRefresh->hide();
}

void MainWindow::changeCssFilePath()
{
    QFileDialog dialog(this, tr("Change Path to CSS Style Sheet File"), gvtree_preferences.pbCssPath->text());

    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        applyStyleSheetFile(dialog.selectedFiles().at(0));
    }
}

void MainWindow::changeTempPath()
{
    QFileDialog dialog(this, tr("Change Path for Temporary Files"), gvtree_preferences.pbTempPath->text());
    QStringList directoryNames;

    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        directoryNames = dialog.selectedFiles();
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

    if (_tmppath == fi.absoluteFilePath()
        && fi.permission(QFile::WriteUser | QFile::ReadUser))
        return true;

    return false;
}

void MainWindow::setGitLocalRepository()
{
    QFileDialog dialog(this, tr("Change Local git Repository Path"), gvtree_preferences.pbLocalRepositoryPath->text());
    QStringList directoryNames;
    QString oldRepositoryPath = repositoryPath;

    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        directoryNames = dialog.selectedFiles();
        QString selDir = directoryNames.at(0);
        if (checkGitLocalRepository(directoryNames.at(0), repositoryPath, fileConstraintPath, false))
        {
            QSettings settings;
            settings.setValue("localRepositoryPath", repositoryPath);
            graphwidget->setLocalRepositoryPath(repositoryPath);
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

    foreach(const QString &str, path_elements)
    {
        lookup_path = lookup_path + str + QString("/");
        QString lookup_git = lookup_path + QString(".git");
        check_for_repo.push_front(lookup_git);
    }

    foreach(const QString &str, check_for_repo)
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

    // set watchdog
    if (watcher)
    {
        if (watcher->directories().size())
            watcher->removePaths(watcher->directories());
        if (watcher->files().size())
            watcher->removePaths(watcher->files());
        QString gitpath = _repoPath + "/.git";
        watcher->addPath(gitpath);
    }

    return true;
}

void MainWindow::helpDialog()
{
    QDialog* hDialog = new QDialog;
    QString css = "background-color: white; color: black;";

    hDialog->setStyleSheet(css);

    gvtree_help.setupUi(hDialog);
    connect(gvtree_help.pbOK, SIGNAL(clicked()), hDialog, SLOT(close()));

    QFile file(":/doc/gvtree-1.1-0.xhtml");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    gvtree_help.textBrowser->setHtml(stream.readAll());

    hDialog->exec();
}

void MainWindow::licenseDialog()
{
    QDialog* lDialog = new QDialog;
    QString css = "background-color: white; color: black;";

    lDialog->setStyleSheet(css);

    gvtree_license.setupUi(lDialog);
    connect(gvtree_license.pbOK, SIGNAL(clicked()), lDialog, SLOT(close()));

    QFile file(":/html/GNU_GPL_v3.0.html");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    gvtree_license.textBrowser->setHtml(stream.readAll());

    lDialog->exec();
}

void MainWindow::aboutDialog()
{
    QMessageBox tmp(QMessageBox::Information, tr("About"), tr(""), QMessageBox::NoButton, this);
    QString css = "background-color: white; color: black;";

    tmp.setStyleSheet(css);

    tmp.setIconPixmap(QPixmap(":/images/gvtree_title_512.png"));
    tmp.exec();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::lookupId(const QString& _text, bool _exactMatch)
{
    if (_text.size() < 3)
        return;

    if (graphwidget->focusElement(_text, _exactMatch))
        search->setFocus();
}

QLineEdit* MainWindow::getSearchWidget() const
{
    return search;
}

CompareTree* MainWindow::getCompareTree()
{
    return gvtree_comparetree.compareTree;
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

TagWidget* MainWindow::getTagWidget() const
{
    return tagwidget;
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
    foreach(const QString &str, cleanupFiles)
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

bool MainWindow::getTopDownView() const
{
    return gvtree_preferences.top_down_view->isChecked();
}

bool MainWindow::getRemotes() const
{
    return gvtree_preferences.remotes->isChecked();
}

bool MainWindow::getOpenGLRendering() const
{
    return gvtree_preferences.open_gl_rendering->isChecked();
}

bool MainWindow::getDiffLocalFiles() const
{
    return gvtree_preferences.diff_local_files->isChecked();
}

void MainWindow::saveChangedSettings()
{
    QSettings settings;

    settings.setValue("gitLogLines", gvtree_preferences.git_log_lines->text());
    settings.setValue("topDownView", gvtree_preferences.top_down_view->isChecked());
    settings.setValue("gitShortHashes", gvtree_preferences.git_short_hashes->isChecked());
    settings.setValue("openGLRendering", gvtree_preferences.open_gl_rendering->isChecked());
    settings.setValue("diffLocalFile", gvtree_preferences.diff_local_files->isChecked());
    settings.setValue("remotes", gvtree_preferences.remotes->isChecked());
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

    pwin->hide();

    graphwidget->preferencesUpdated();
}

bool MainWindow::getXYFactor(int& _xfactor, int& _yfactor)
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
    restoreColorSettingsHelper(gvtree_preferences.pbColorBackground, QString("colorBackground"), QColor(255, 240, 192));
    restoreColorSettingsHelper(gvtree_preferences.pbColorText, QString("colorText"), QColor(0, 0, 0));
    restoreColorSettingsHelper(gvtree_preferences.pbColorVersion, QString("colorVersion"), QColor(255, 192, 0));
    restoreColorSettingsHelper(gvtree_preferences.pbColorEdge, QString("colorEdge"), QColor(64, 64, 64));
    restoreColorSettingsHelper(gvtree_preferences.pbColorMerge, QString("colorMerge"), QColor(64, 64, 192));
    restoreColorSettingsHelper(gvtree_preferences.pbColorSearch, QString("colorSearch"), QColor(20, 100, 200));
    restoreColorSettingsHelper(gvtree_preferences.pbColorSelected, QString("colorSelected"), QColor(255, 0, 0));
    restoreColorSettingsHelper(gvtree_preferences.pbColorFolded, QString("colorFolded"), QColor(64, 64, 64));
    restoreColorSettingsHelper(gvtree_preferences.pbColorUnfolded, QString("colorUnfolded"), QColor(192, 192, 192));
    restoreColorSettingsHelper(gvtree_preferences.pbColorFromTo, QString("colorFromTo"), QColor(128, 128, 255));
    restoreColorSettingsHelper(gvtree_preferences.pbColorFileConstraint, QString("colorFileConstraint"), QColor(255, 0, 0));
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
    restoreColorSettingsHelper(_pb, _key, tmpColor, true);
}

void MainWindow::colorDialogBackground()
{
    colorDialogCommon("colorBackground", gvtree_preferences.pbColorBackground);
}

void MainWindow::colorDialogText()
{
    colorDialogCommon("colorText", gvtree_preferences.pbColorText);
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

void MainWindow::directoryChanged(const QString&)
{
    pbRepositoryRefresh->show();
}

void MainWindow::fileChanged(const QString&)
{
    pbRepositoryRefresh->show();
}
