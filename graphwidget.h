/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.7-0                */
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
    Version* gitlogSingle(QString _hash = QString(), bool _create = false);

    // Get hashes of one file and markup versions
    void setGitLogFileConstraint(const QString& _fileConstraint = QString());

    // Create the graph from git log information
    void process(QList<QString> _cache);

    void forceUpdate();

    // Compare or view versions
    void compareToSelected(Version* _v, bool _view = false);
    void compareToPrevious(Version* _v, bool _view = false);
    void compareToLocalHead(Version* _v, bool _view = false);
    void compareToBranchBaseline(Version* _v, bool _view = false);
    void viewThisVersion(Version* _v);
    void compareVersions(Version* _v1, Version* _v2, bool _showDiff = false);

    // focus
    int matchVersions(const QString& _text, QList<Version*>& _matches, bool _exactMatch = false, QString _keyConstraint = QString());
    bool focusElements(const QString& _text, bool _exactMatch = false, QString _keyConstraint = QString());
    bool focusElements(const QList<Version*>& _markup);
    void displayHits(Version* _v);
    void displayHits(const QList<Version*>& _hits, bool _unfold = true);

    void clear();
    void getMarkedupVersions(QList<Version*>& _markup, bool _selected = true);
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
    const QSet<Version*>& getPredecessors() const;
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

    bool isFromToVersion(Version* _v) const;

    const QImage* getImage(const QString& _name) const;

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
    void adjustComments();
    void adjustAllEdges();
    void setBlockItemChanged(bool _val);

protected:
    void focusFromTo(const QRectF& _from, const QRectF& _to);
    void animatedFocus(const QRectF& _from, const QRectF& _to);
    QRectF animatedFocus(const QRectF& _from, const QRectF& _to, double _morph);
    void aspectCenter(QRectF& _from, QRectF& _to);

    virtual void contextMenuEvent(QContextMenuEvent* _event);

    virtual void keyPressEvent(QKeyEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    void drawBackground(QPainter* painter, const QRectF& rect);
    void scaleView(qreal scaleFactor);
    void expandTree();
    void fillCompareWidgetFromToInfo();
    Version* findVersion(const QString& _hash);

    // to debug the git log --graph parser...
    void debugGraphParser(const QString& _tree, const QVector<Version*>& _slots);
    void debugExit(char _c,
                   int _column,
                   int _lineNumber,
                   const QString& _tree,
                   const QString& _previousTree,
                   const QString& _line);

private:

    // from/to version cursor
    QSet<Version*> fromVersions;
    Version* toVersion;
    FromToInfo* fromToInfo;

    // backup the hashes to restore after refresh
    QStringList fromHashSave;
    QString toHashSave;

    // root version node
    Version* rootVersion;
    Version* localHeadVersion; // local HEAD version
    Version* headVersion;      // top version element (without --remotes == localHeadVersion)

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
    QColor foldedColor;
    QColor unfoldedColor;
    QColor fileConstraintColor;
    int connectorStyle;

    int maxLines;
    int currentLines;
    bool shortHashes;
    bool reduceTree;
    bool topDownView;
    int horizontalSort;
    bool remotes;
    bool all;
    int xfactor;
    int yfactor;
    int commentColumns;
    int commentMaxlen;

    Version* selectedVersion;
    QString selectedVersionHash;

    //
    QMap<QString, QMap<QString, QStringList> > keyInformationCache;

    QMap<QString, const QImage*> imageDB;
};

#endif
