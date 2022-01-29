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

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QAction>
#include <QComboBox>
#include <QFileSystemWatcher>
#include <QLineEdit>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QString>
#include <QStringList>
#include <QTextBrowser>
#include <QTreeView>

#include "graphwidget.h"
#include "tagprefgridlayout.h"
#include "tagwidget.h"
#include "ui_gvtree_comparetree.h"
#include "ui_gvtree_difftool.h"
#include "ui_gvtree_help.h"
#include "ui_gvtree_license.h"
#include "ui_gvtree_preferences.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QStringList& _argv);

    // get dock and sub widgets...
    const TagPreference* getTagPreference(const QString& _key) const;
    CompareTree* getCompareTree();
    QDockWidget* getCompareTreeDock();
    QTextEdit* getCompareTreeFromTextEdit();
    QTextEdit* getCompareTreeToTextEdit();
    QPushButton* getCompareTreeFromPushButton();
    QPushButton* getCompareTreeToPushButton();
    QComboBox* getFromComboBox();
    QLabel* getToDateLabel();
    QLineEdit* getSearchWidget() const;
    QDockWidget* getSearchDock();
    TagWidget* getTagWidget() const;

    // dialog to store tools for a certain file type
    void getMimeTypeTools(const QString& _mimeType,
                          QString& _diff,
                          QString& _edit);

    void addToCleanupFiles(const QString& _path);

    // get preferences
    void getXYFactor(float& _xfactor, float& _yfactor) const;
    int getMaxLines() const;
    bool getShortHashes() const;
    bool getTopDownView() const;
    bool getRemotes() const;
    bool getOpenGLRendering() const;
    bool getDiffLocalFiles() const;
    int getConnectorStyle() const;
    bool getXYFactor(int& _xfactor, int& _yfactor);
    QColor getPreferencesColor(QString _key);
    QString getTempPath() const;
    bool getPrintCmdToStdout() const;

    //
    Ui_Dialog& getPreferences();

public slots:

    // Watchdog if there are changes in the local repository
    void fileChanged(const QString& _path);
    void directoryChanged(const QString& _path);

    // color preference changed
    void colorDialogBackground();
    void colorDialogText();
    void colorDialogVersion();
    void colorDialogEdge();
    void colorDialogMerge();
    void colorDialogSelected();
    void colorDialogSearch();
    void colorDialogFolded();
    void colorDialogUnfolded();
    void colorDialogFromTo();
    void colorDialogFileConstraint();

    // save all (changed) preferences to QSettings
    void saveChangedSettings();

    // File menu
    void reloadCurrentRepository();
    void setGitLocalRepository();
    void quit();

    // View menu
    void changeGlobalVersionInfo(QAction* _act);

    // Help menu
    void helpDialog();
    void licenseDialog();
    void aboutDialog();

    // search line edit
    void lookupId(const QString& _text, bool _exactMatch = false);

    // select path and apply CSS style sheet file
    void changeCssFilePath();

    // change temporary file path
    void changeTempPath();

    virtual void resizeEvent(QResizeEvent* event);

    //
    void removeFilter();

    //
    void updatePbFileConstraint(const QString& _fileConstraint);

protected:

    // try to load and apply a CSS style sheet file
    bool applyStyleSheetFile(QString _path);

    // color dialog helper for different objects
    void colorDialogCommon(QString _key, QPushButton* _pb);

    // check for .git in the path
    bool checkGitLocalRepository(const QString& _path,
                                 QString& _repoPath,
                                 QString& _fileConstraintPath,
                                 bool _silent = false);

    // split the repository path to get a separate repository name
    void splitRepositoryPath(const QString& _input, QString& _pathTo, QString& _repoName);

    // create main window menus
    void createMenus();

    // restore information from QSettings
    void restorePreferencesSettings();
    void restoreWindowSettings();
    void restoreLocalRepository();
    void restoreColorSettings();
    void restoreColorSettingsHelper(QPushButton* _pb, QString _settingsKey, QColor _defaultColor, bool _overwrite = false);
    void restoreDifftoolMimeTypes();

    // remove all temporary files of this session
    void doCleanupFiles();

    // check if the permissions of _tmppath are ok
    bool checkForValidTempPath(const QString& _tmppath);

    // in case of exit, save settings
    virtual void closeEvent(QCloseEvent* _event);

    // update git status on reload
    void updateGitStatus(const QString& _repoPath);

    TagWidget* tagwidget;
    
    QTextBrowser* gitstatus;

    // menu
    QMenu* filemenu;
    QMenu* viewmenu;
    QMenu* windowmenu;
    QMenu* helpmenu;
    QAction* preferencesAct;
    QAction* aboutAct;
    QAction* helpAct;
    QAction* licenseAct;
    QAction* refreshRepo;

    // docks
    GraphWidget* graphwidget;
    QWidget* ctwin;
    QTreeView* compareTree;
    QLineEdit* search;
    TagPrefGridLayout* gridLayout;

    // preferences dialog
    QDialog* pwin;

    // Storage for all files to remove when ending the session
    QSet<QString> cleanupFiles;

private:
    Ui_Dialog gvtree_preferences;
    Ui_DifftoolDialog gvtree_difftool;
    Ui_HelpDialog gvtree_help;
    Ui_LicenseDialog gvtree_license;
    Ui_CompareTreeForm gvtree_comparetree;
    QLabel* lbRepositoryPath;
    QPushButton* pbRepositoryName;
    QPushButton* pbFileConstraint;
    QString repositoryPath;
    QString fileConstraintPath;
    QFileSystemWatcher* watcher;
    QPushButton* pbRepositoryRefresh;
    QDockWidget* compareTreeDock;
    QDockWidget* searchDock;
};

#endif
