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

#ifndef __VERSION_H__
#define __VERSION_H__

#include <QGraphicsItem>
#include <QList>
#include <QRegExp>
#include <QSet>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QDateTime>

#include "node.h"

class Edge;
class GraphWidget;
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class Version : public QGraphicsItem,
                public Node
{

public:
    /**
     * \brief Constructor for the verion tree root node
     */
    Version(GraphWidget* _graphWidget, QGraphicsItem* _parent = NULL);

    /**
     * \brief Constructor for a Version.
     *        The globalVersionInfo contains the keyInformation elements
     *        which are activated by mainwindows' View menu.
     */

    Version(const QStringList& _globalVersionInfo,
            GraphWidget* _graphWidget,
            QGraphicsItem* _parent = NULL);

    enum {Type = UserType + 1};
    int type() const
    {
        return Type;
    }


    bool isMain() const;
    bool isRoot() const;
    bool isFolder() const;
    bool isFolded() const;

    bool isFoldable() const;
    void setIsFoldable(bool _val);

    void setIsMain(bool _val);

    Version* lookupFolderVersion();
    Version* lookupBranchBaseline();

    const QMap<QString, QStringList>& getKeyInformation() const;

    void setKeyInformation(const QMap<QString, QStringList>& _data);

    QString getCommitDateString() const;

    /**
     * \brief The git log information _input is checked against
     *        rawInput. If changed the new tokens of _parts
     *        are proessed.
     *        _parts contains dummy, hash, commit date, user name and
     *        tag information.
     *
     * \return If changed true is returned
     */
    bool processGitLogInfo(const QString& _input, const QStringList& _parts);


    /**
     * \brief The %d information of the git log output is analyzed for
     *        certain tag patterns.
     */
    void processGitLogTagInformation(const QString& _tagInfo);
    void processGitLogCommentInformation(const QString& _tagInfo);
    void updateCommentInformation(int _columns, int _maxlen);

    //!> Edges

    virtual void addInEdge(Edge* edge);
    virtual void addOutEdge(Edge* edge);

    //!> QGraphicsItems

    virtual QRectF boundingRect() const;

    virtual QPainterPath shape() const;

    virtual void paint(QPainter* _painter, const QStyleOptionGraphicsItem* _option, QWidget* _widget);

    virtual void setSelected(bool _val);
    virtual bool isSelected() const;

    const QString& getHash() const;

    QList<Version*> getNeighbourBox();

    void calculateCoordinates(float _scaleX, float _scaleY);
    void linkTreenodes(Version* _parent);

    void setMatched(bool _val);
    bool getMatched() const;

    void addLocalVersionInfo(const QString& _val);

    void setUpdateBoundingRect(bool _val);
    bool findMatch(QRegExp& _pattern, const QString& _text, bool _exactMatch = false);
    void collectFolderVersions(Version* _rootNode, Version* _parent);
    void foldRecurse(bool _val);
    int numEdges() const;
    QList<Version*> getFolderVersions() const;
    void clearFolderVersions();

    void updateFolderBox();
    void applyHorizontalSort(int _sort);
    int calculateWeightRecurse();
    int getWeight() const;
    long getCommitDate() const;
    int getPredecessors(QSet<Version*>& _result);
    int getPredecessorHashes(QStringList& _result);
    void calculateLocalBoundingBox();
    bool getSubtreeHidden() const;
    void setSubtreeVisible(bool _value);

    void compareToSelected(bool _view = false);
    void compareToPrevious(bool _view = false);
    void compareToLocalHead(bool _view = false);
    void compareToBranchBaseline(bool _view = false);
    void viewThisVersion();
    void focusNeighbourBox();
    // all elements in linear will then get Node::height = 0
    void foldAction();

    void setBlockItemChanged(bool _val);
    bool getBlockItemChanged() const;

    void setFileConstraint(bool _val);
    bool getFileConstraint() const;

    void reduceToFileConstraint(Version* _parent, bool _merge = false);
    void clearFileConstraintEdgeList();
    QList<Edge*>& getFileConstraintInEdgeList();
    QList<Edge*>& getFileConstraintOutEdgeList();

    /**
     * \brief If version is folded away, ensure its visiblity.
     *
     * \return true, if folding has changed
     */
    bool ensureUnfolded();

protected:
    bool hasBranch() const;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _event);

    bool getTextBoundingBox(const QString& _key, const QStringList& _values, int& _height, QRectF& _updatedBox) const;
    bool drawTextBox(const QString& _key, const QStringList& _values, int& _height, const qreal& _lod, QPainter* _painter);

    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void adjustEdges();

private:
    QList<Edge*> edgeList;

    QList<Edge*> fileConstraintInEdgeList;
    QList<Edge*> fileConstraintOutEdgeList;

    // QString hash;
    QStringList tags;
    QStringList branchTags;
    GraphWidget* graph;

    QString commit_sse;
    const QStringList& globalVersionInfo;
    static QStringList dummy;

    bool matched;

    QMap<QString, QStringList> keyInformation;
    QSet<QString> localVersionInfo;
    QRectF localBoundingBox;
    QRectF folderBox;
    QString treeInfo;
    QString hash;
    QDateTime commitTimestamp;

    // Versions with no fork or merge are collected in the following list.
    // They can be folded or unfolded, then.
    QList<Version*> linear;

    void addToFolder(Version* _v);

    // fold/unfold action:
    // if the versions in linear are folded, the flag of the list owner
    // 'folded' is set. Initial value is true.
    bool folded;

    // flag to control if the version can be folded away
    bool foldable;

    // rootnode
    bool rootnode;

    // hidden subtree?
    bool subtreeHidden;

    // foldedHeight
    // Node::foldedHeight := 0 for all

    bool blockItemChanged;

    //
    QString rawInput;

    bool updateBoundingRect;

    bool main;

    bool fileConstraint;

    bool selected;

    int weight;

    long commitDate;
};

typedef struct Version* VersionPointer;
Q_DECLARE_METATYPE(VersionPointer)

#endif
