/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.2-0                */
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

#ifndef __GRAPHWIDGET_H__
#define __GRAPHWIDGET_H__

#include <QGraphicsView>
#include <QModelIndex>
#include <QDockWidget>
#include <QTreeView>
#include <QString>
#include <QList>
#include <QRectF>
#include <QTextEdit>

#include "tagwidget.h"
#include "fromtoinfo.h"
#include "comparetree.h"

class Version;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT

public:
    // Constructor
    GraphWidget(class MainWindow* parent = NULL);

    // Test tree
    void test();

    // Test load git log file
    void load(const QString& _path);

    // Get real git log information of a local repository
    void gitlog(bool _dirChanged = false);
    Version* gitlogSingle(QString _hash = QString());

    // Get hashes of one file and markup versions
    void setGitLogFileConstraint(const QString& _fileConstraint = QString());

    // Create the graph from git log information
    void process(QList<QString> _cache);

    void forceUpdate();

    // Compare or view versions
    void compareToSelected(Version* _v);
    void compareToPrevious(Version* _v);
    void compareToLocalHead(Version* _v);
    void compareToBranchBaseline(Version* _v);
    void viewThisVersion(Version* _v);
    void compareVersions(Version* _v1, Version* _v2);

    // focus
    void focusNeighbourBox(const QRectF& _rect);
    bool focusElement(const QString& _text, bool _exactMatch = false);

    void clear();
    void resetMatches();
    void setMinSize(bool _resize = true);

    // change the position of HEAD
    void flipY();
    void setGlobalVersionInfo(const QString& _item, bool _value);

    void setLocalRepositoryPath(const QString& _dir);
    const QString& getLocalRepositoryPath() const;

    const class MainWindow* getMainWindow() const;
    void commitInfo(const Version* _v, QTextEdit* _tedi);

    void setCompareTree(CompareTree* _tree);
    void resetCompareTree();
    QStringList getFromHashes() const;
    QString getToHash() const;
    const QString& getFileConstraint() const;
    void calculateGraphicsViewPosition();
    const QList<Version*>& getPredecessors() const;
    Version* getVersionByHash(const QString& _hash);
    Version* getSelectedVersion();
    void preferencesUpdated(bool _forceUpdate = false);

    /**
     * \brief After reload or refresh check if the current
     *        versions are contained in the new version tree.
     *        If not false is returned and the compare widget
     *        can be cleared, too.
     */
    bool restoreImportantVersions();
    void saveImportantVersions();

    float getXFactor() const;
    float getYFactor() const;
    bool getTopDownView() const;

    Version* getLocalHeadVersion() const;
    void updateFromToInfo();

    // style
    const QColor& getBackgroundColor() const
    {
        return backgroundColor;
    }
    const QColor& getFromToColor() const
    {
        return fromToColor;
    }
    const QColor& getNodeColor() const
    {
        return nodeColor;
    }
    const QColor& getSelectedColor() const
    {
        return selectedColor;
    }
    const QColor& getSearchColor() const
    {
        return searchColor;
    }
    const QColor& getEdgeColor() const
    {
        return edgeColor;
    }
    const QColor& getMergeColor() const
    {
        return mergeColor;
    }
    const QColor& getTextColor() const
    {
        return textColor;
    }
    const QColor& getFoldedColor() const
    {
        return foldedColor;
    }
    const QColor& getUnfoldedColor() const
    {
        return unfoldedColor;
    }
    const QColor& getFileConstraintColor() const
    {
        return fileConstraintColor;
    }
    int getConnectorStyle() const
    {
        return connectorStyle;
    }

public slots:
    void diffStagedChanges();
    void diffLocalChanges();
    void resetDiff();
    void resetSelection();
    void focusCurrent();
    void fitInView();
    void foldAll();
    void unfoldAll();
    void normalizeGraph();
    void updateGraphFolding(Version* _v = NULL);
    void zoomIn();
    void zoomOut();
    void updateColors();

    void focusFromVersion();
    void focusToVersion();
    void removeFilter();
    void adjustAllEdges();
    void setBlockItemChanged(bool _val);

protected:
    virtual void contextMenuEvent(QContextMenuEvent* _event);

    virtual void keyPressEvent(QKeyEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    void drawBackground(QPainter* painter, const QRectF& rect);
    void scaleView(qreal scaleFactor);
    void focusVersion(const Version* _v);
    void expandTree();
    void fillCompareWidgetFromToInfo();
    Version* findVersion(const QString& _hash);

private:

    // from/to version cursor
    QList<Version*> fromVersions;
    Version* toVersion;
    FromToInfo* fromToInfo;

    // backup the hashes to restore after refresh
    QStringList fromHashSave;
    QString toHashSave;

    // root version node
    Version* rootVersion;
    Version* localHeadVersion; // local HEAD version
    Version* branchVersion;    // top version element

    QStringList globalVersionInfo;

    // other widgets
    class MainWindow* mwin;
    CompareTree* compareTree;

    // mouse pan
    bool pan;
    QPoint mpos;

    // argv
    QString localRepositoryPath;
    QString fileConstraint;
    QSet<QString> fileConstraintHashes;

    // preferences
    QColor backgroundColor;
    QColor fromToColor;
    QColor nodeColor;
    QColor selectedColor;
    QColor searchColor;
    QColor edgeColor;
    QColor mergeColor;
    QColor textColor;
    QColor foldedColor;
    QColor unfoldedColor;
    QColor fileConstraintColor;
    int connectorStyle;

    int maxLines;
    int currentLines;
    bool shortHashes;
    bool topDownView;
    bool remotes;
    bool foldHead;
    int xfactor;
    int yfactor;

    Version* selectedVersion;
    QString selectedVersionHash;

    //
    QMap<QString, QMap<QString, QStringList> > keyInformationCache;
};

#endif
