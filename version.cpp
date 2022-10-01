/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.3-0                */
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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QColor>
#include <QStyleOption>
#include <QClipboard>
#include <QApplication>

#include <iostream>
#include <algorithm>

#include "edge.h"
#include "version.h"
#include "graphwidget.h"
#include "tagpreference.h"
#include "mainwindow.h"

QStringList Version::dummy;

Version::Version(GraphWidget* _graphWidget, QGraphicsItem* _parent) :
    QGraphicsItem(_parent),
    Node(),
    graph(_graphWidget),
    globalVersionInfo(dummy),
    matched(false),
    folded(false),
    foldable(true),
    rootnode(true),
    subtreeHidden(false),
    blockItemChanged(false),
    updateBoundingRect(false),
    main(false),
    fileConstraint(false),
    selected(false)
{
    // flags
    setFlag(ItemSendsGeometryChanges);
    // setCacheMode(DeviceCoordinateCache);
    setZValue(5);
}

Version::Version(const QStringList& _globalVersionInfo,
                 GraphWidget* _graphWidget,
                 QGraphicsItem* _parent) :
    QGraphicsItem(_parent),
    Node(),
    graph(_graphWidget),
    globalVersionInfo(_globalVersionInfo),
    matched(false),
    folded(true),
    foldable(true),
    rootnode(false),
    subtreeHidden(false),
    blockItemChanged(false),
    updateBoundingRect(false),
    main(false),
    fileConstraint(false),
    selected(false)
{
    // flags
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    folderBox = QRectF(-30, -30, 60, 60);
    setZValue(5);
}

void Version::compareToSelected()
{
    graph->compareToSelected(this);
}

void Version::compareToPrevious()
{
    graph->compareToPrevious(this);
}

void Version::compareToLocalHead()
{
    graph->compareToLocalHead(this);
}

void Version::compareToBranchBaseline()
{
    graph->compareToBranchBaseline(this);
}

void Version::viewThisVersion()
{
    graph->viewThisVersion(this);
}

void Version::focusNeighbourBox()
{
    graph->focusNeighbourBox(getNeighbourBox());
}

QRectF Version::getNeighbourBox() const
{
    // in case of a folder...
    if (isFolder() && isFolded())
        return linear.front()->getNeighbourBox();

    // normal case
    QRectF focusBox(QGraphicsItem::scenePos(), QSizeF(1.0, 1.0));

    foreach (const Edge * edge, edgeList)
    {
        // TODO with or without merge nodes?
        if (edge->getMerge())
            continue;

        Version* v =
            dynamic_cast<Version*>(
                this == edge->sourceVersion()
                ? edge->destVersion() : edge->sourceVersion());
        if (v)
            focusBox |= QRectF(v->scenePos(), QSizeF(1.0, 1.0));
    }

    focusBox.adjust(-graph->getXFactor() / 2, -graph->getYFactor() / 2, graph->getXFactor() / 2, graph->getYFactor() / 2);

    return focusBox;
}

void Version::setBlockItemChanged(bool _val)
{
    blockItemChanged = _val;
}

bool Version::getBlockItemChanged() const
{
    return blockItemChanged;
}

void Version::setSelected(bool _val)
{
    QApplication::clipboard()->setText(_val ? hash : QString(), QClipboard::Selection);
    selected = _val;
}

void Version::addInEdge(Edge* _edge)
{
    edgeList.append(_edge);  // all edges
}

void Version::addOutEdge(Edge* _edge)
{
    edgeList.append(_edge); // all edges
    if (_edge->getMerge() == false && _edge->sourceVersion() == this)
    {
        Node::addOutEdge(_edge);
    }
}

QRectF Version::boundingRect() const
{
    return localBoundingBox;
}

QPainterPath Version::shape() const
{
    QPainterPath path;

    if (isFolder())
        path.addRect(folderBox);
    else
        path.addEllipse(-15, -15, 30, 30);
    return path;
}

void Version::paint(QPainter* _painter, const QStyleOptionGraphicsItem* _option, QWidget*)
{

    if (getH() == 0)
        return;

    const qreal lod = _option->levelOfDetailFromTransform(_painter->worldTransform());

    if (isFolder())
    {
        _painter->setPen(QPen(isFolded() ? graph->getFoldedColor() : graph->getUnfoldedColor(), 0));
        _painter->drawRect(folderBox);
    }

    _painter->setPen(QPen(Qt::black, 0));

    if (matched)
    {
        _painter->setPen(QPen(graph->getSearchColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        _painter->setBrush(graph->getSearchColor().lighter());
        _painter->drawEllipse(-15, -15, 30, 30);
    }
    else
    {
        _painter->setPen(QPen(graph->getNodeColor(), 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        _painter->setBrush(graph->getNodeColor());
    }

    // dot is red if selected, blue if it is a merge version and black if normal
    if (isSelected())
    {
        _painter->setPen(QPen(graph->getSelectedColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        _painter->setBrush(graph->getSelectedColor().lighter());
    }

    _painter->drawEllipse(-10, -10, 20, 20);

    if (lod > 0.3)
    {
        // text color is black
        _painter->setPen(QPen(graph->getTextColor(), 0));

        int height = 0;

        foreach(const QString &info, graph->getMainWindow()->getNodeInfo())
        {
            if (globalVersionInfo.contains(info)
                || localVersionInfo.contains(info))
            {
                QMap<QString, QStringList>::iterator kit = keyInformation.find(info);
                if (kit != keyInformation.end())
                {
                    drawTextBox(info, kit.value(), height, _painter);
                    height += 1;
                }
            }
        }
    }
}

QVariant Version::itemChange(GraphicsItemChange _change, const QVariant& _value)
{
    if (_change == ItemPositionHasChanged && blockItemChanged == false)
        adjustEdges();

    return QGraphicsItem::itemChange(_change, _value);
}

void Version::adjustEdges()
{
    foreach(QGraphicsItem * it, childItems())
    {
        Version* v = dynamic_cast<Version*>(it);

        if (v)
            v->adjustEdges();
    }
    foreach (Edge * edge, edgeList)
    {
        edge->adjust();
    }
    foreach (Edge * edge, fileConstraintInEdgeList)
    {
        edge->adjust();
    }
    foreach (Edge * edge, fileConstraintOutEdgeList)
    {
        edge->adjust();
    }
}

QString Version::getCommitDate() const
{
    return keyInformation.value("Commit Date", QStringList()).join(QString());
}

bool Version::processGitLogInfo(const QString& _input, const QStringList& _parts)
{
    // only member, here, all other information is stored in keyInformation
    hash = _parts.at(1);

    // check if there is new information:
    if (keyInformation[QString("_input")].join(QString()) == _input)
        return false;

    // erase old information
    keyInformation.clear();

    // store the raw input in the key information, too
    keyInformation[QString("_input")] = QStringList(_input);
    keyInformation[QString("Hash")] = QStringList(hash);
    keyInformation[QString("Commit Date")] =
        QStringList(
            QDateTime::fromTime_t(_parts.at(2).toInt())
            .toString("yyyy.MM.dd HH:mm:ss"));
    keyInformation[QString("User Name")] = QStringList(_parts.at(3));

    // tag information
    processGitLogTagInformation(_parts.at(4));

    if (_parts.size() > 5)
    {
        processGitLogCommentInformation(_parts.at(5));
    }

    return true;
}

void Version::processGitLogCommentInformation(const QString& _comment)
{
    int maxlen = 40;
    int len = 0;
    QString part;
    QStringList tmp = _comment.split(' ');
    foreach (const QString &str, tmp)
    {
        if (len == 0)
        {
            part = str;
            len += str.size();
        }
        else if (len < maxlen)
        {
            part = part + " " + str;
            len += 1 + str.size();
        }
        else
        {
            keyInformation[QString("Comment")].push_back(part);
            part = QString();
            len = 0;
        }
    }

    if (len != 0)
    {
        keyInformation[QString("Comment")].push_back(part);
    }
}

void Version::processGitLogTagInformation(const QString& _tagInfo)
{
    int cstart = _tagInfo.indexOf(QChar('(')) + 1;
    int cend = _tagInfo.indexOf(QChar(')'));

    if (cstart >= 0 && cend >= 0)
    {
        QStringList matches = _tagInfo.mid(cstart, cend - cstart).split(',');
        QStringList tags;

        foreach (const QString &str, matches)
        {
            tags << str.trimmed();
        }

        QStringList scanItems;
        scanItems
            << QString("HEAD")
            << QString("Release Label")
            << QString("Baseline Label")
            << QString("FIX/PQT Label")
            << QString("HO Label")
            << QString("Branch");

        foreach(const QString &it, scanItems)
        {
            const TagPreference* tp =
                graph->getMainWindow()->getTagPreference(it);

            if (tp)
            {
                QRegExp r = tp->getRegExp();
                foreach (const QString &str, tags)
                {
                    if (r.indexIn(str, 0) != -1)
                    {
                        keyInformation[it].push_back(r.cap(1));
                        tags.removeOne(str);
                    }
                }
            }
        }

        keyInformation[QString("Other Tags")] = tags;
    }
}

bool Version::findMatch(QRegExp& _pattern, const QString& _text, bool _exactMatch)
{
    bool oldmatched = matched;
    bool newmatched = false;

    QSet<QString> oldLocalVersionInfo = localVersionInfo;
    localVersionInfo.clear();

    bool checkDetail = false;
    QString rawInput = keyInformation["_input"].join(" ");
    rawInput += " " + keyInformation["Commit Date"].join(" ");

    if (_pattern.isValid())
    {
        checkDetail = (_pattern.indexIn(rawInput, 0) != -1);
    }
    else
    {
        checkDetail = (rawInput.indexOf(_text, 0) != -1);
    }

    if (checkDetail)
    {

        for (QMap<QString, QStringList>::iterator kit = keyInformation.begin();
             kit != keyInformation.end();
             kit++)
        {

            if (kit.key() == QString("_input"))
                continue;

            if (_pattern.isValid() && _exactMatch == false)
            {
                QString tmp = kit.value().join(QString(" "));
                if (_pattern.indexIn(tmp, 0) != -1)
                {
                    newmatched = true;
                    localVersionInfo.insert(kit.key());
                }
            }
            else
            {
                if (_exactMatch == true)
                {
                    foreach (const QString &str, kit.value())
                    {
                        if (str == _text)
                        {
                            newmatched = true;
                            localVersionInfo.insert(kit.key());
                            break;
                        }
                    }
                    if (newmatched)
                        break;
                }
                else if (kit.value().join(QString(" ")).indexOf(_text) != -1)
                {
                    newmatched = true;
                    localVersionInfo.insert(kit.key());
                }
            }
        }
    }
    if (newmatched != oldmatched
        || localVersionInfo != oldLocalVersionInfo)
    {
        if (_exactMatch == true && newmatched == true)
        {
            ensureUnfolded();
        }

        setMatched(newmatched);
    }
    return matched;
}

void Version::mouseMoveEvent(QGraphicsSceneMouseEvent* _event)
{
    if (isRoot())
        return;

    if (isFolder())
        linear.front()->mouseMoveEvent(_event);
    else
        QGraphicsItem::mouseMoveEvent(_event);

    graph->updateFromToInfo();
}

const QString& Version::getHash() const
{
    return hash;
}

bool Version::getTextBoundingBox(const QString& _key, const QStringList& _values, int& _height, QRectF& _updatedBox) const
{
    const TagPreference* tp = graph->getMainWindow()->getTagPreference(_key);

    if (!tp)
        return false;

    foreach(const QString it, _values)
    {
        QRectF textbox = QFontMetricsF(tp->getFont()).boundingRect(it).translated(20, _height);

        textbox.adjust(0, 0, 32, 0);

        _height += textbox.height() + 2;
        _updatedBox |= textbox;
    }
    return true;
}

bool Version::drawTextBox(const QString& _key, const QStringList& _values, int& _height, QPainter* _painter)
{
    const TagPreference* tp = graph->getMainWindow()->getTagPreference(_key);

    if (!tp)
        return false;

    foreach(const QString &it, _values)
    {
        QRectF textbox = QFontMetricsF(tp->getFont()).boundingRect(it).translated(20, _height);

        textbox.adjust(0, 0, 32, 0);

        _height += textbox.height() + 2;
        _painter->setFont(tp->getFont());
        _painter->drawText(textbox, Qt::AlignLeft, it);
    }
    return true;
}

void Version::setMatched(bool _val)
{
    if (matched != _val)
    {
        if (_val == false)
        {
            localVersionInfo.clear();
        }
        matched = _val;
        calculateLocalBoundingBox();
        QGraphicsItem::update();
    }
}

bool Version::getMatched() const
{
    return matched;
}

void Version::calculateLocalBoundingBox()
{
    localBoundingBox = folderBox | QRectF(-30, -30, 60, 60);

    int height = 0;
    for (QMap<QString, QStringList>::const_iterator kit = keyInformation.begin();
         kit != keyInformation.end();
         kit++)
    {
        getTextBoundingBox(kit.key(), kit.value(), height, localBoundingBox);
    }

    // done
    updateBoundingRect = false;
}

void Version::linkTreenodes(Version* _parent)
{
    foreach (const Edge * edge, outEdges)
    {
        if (edge->getMerge() == false)
        {
            Version* v = dynamic_cast<Version*>(edge->destVersion());
            if (v)
            {
                v->linkTreenodes(this);
            }
        }
    }

    if (_parent)
    {
        setParentItem(_parent);
        setPos(QGraphicsItem::pos() - _parent->QGraphicsItem::pos());
    }
}

void Version::calculateCoordinates(float _scaleX, float _scaleY)
{
    setPos(_scaleX * getX(), _scaleY * getY());
}

int Version::numEdges() const
{
    return edgeList.size();
}

Version* Version::lookupBranchBaseline()
{
    Version* parent = NULL;

    foreach(Edge * it, edgeList)
    {
        if (it->getMerge() == false
            && it->getInfo() == false
            && it->destVersion() == this)
        {
            parent = dynamic_cast<Version*>(it->sourceVersion());
            break;
        }
    }
    if (parent && parent->hasBranch())
        return parent;

    if (parent == NULL)
        return this;

    return parent->lookupBranchBaseline();
}

bool Version::hasBranch() const
{
    int i = 0;

    foreach (const Edge * edge, outEdges)
    {
        i += (edge->getInfo() == false && edge->getMerge() == false) ? 1 : 0;
    }

    return (i > 1);
}

void Version::collectFolderVersions(Version* _rootNode, Version* _parent)
{
    if (_parent
        && _parent != _rootNode
        && _parent->getNumOutEdges() == 1
        && getNumOutEdges() <= 1
        && (numEdges() - getNumOutEdges() <= 1)
        && isFoldable()
       )
    {
        addToFolder(_parent);
    }

    foreach (const Edge * edge, outEdges)
    {
        Version* next = dynamic_cast<Version*>(edge->destVersion());

        if (next)
            next->collectFolderVersions(_rootNode, this);
    }
}

void Version::addToFolder(Version* _v)
{
    linear = _v->getFolderVersions();
    linear.push_back(_v);
    _v->clearFolderVersions();
    _v->setH(0);
    if (!isFolded())
        updateFolderBox();
}

QList<Version*> Version::getFolderVersions() const
{
    return linear;
}

void Version::clearFolderVersions()
{
    linear.clear();
}

Version* Version::lookupFolderVersion()
{
    if (isFolder())
        return this;

    if (outEdges.size() == 1)
    {
        Version* v = dynamic_cast<Version*>(outEdges.front()->destVersion());
        if (v)
            return v->lookupFolderVersion();
    }

    return NULL;
}

void Version::foldAction()
{
    if (isFolder() == false)
        return;

    folded = !folded;

    setZValue(4 + folded);

    foreach(Version * v, linear)
    {
        v->setH(folded ? 0 : 1);
        v->update();
        foreach(Edge * edge, v->getOutEdges())
        {
            edge->QGraphicsItem::setVisible(!folded);
        }
    }

    if (folded)
        folderBox = QRectF(-30, -30, 60, 60);
    else
    {
        float h = graph->getYFactor() * linear.size();
        folderBox = QRectF(-30, -30, 60, 60 + h)
            .translated(0, (graph->getTopDownView() ? 1.0 : -1.0) * h);
    }
    update();
}

void Version::updateFolderBox()
{
    folderBox = QRectF(-30, -30, 60, 60);
    if (folded)
        return;

    foreach(Version * v, linear)
    {
        folderBox |= QRectF(-30, -30, 60, 60).translated(v->QGraphicsItem::scenePos() - QGraphicsItem::scenePos());
    }
}

void Version::foldRecurse(bool _val)
{
    if (isFolder() && isFolded() != _val)
    {
        foldAction();
    }

    foreach (Edge * edge, outEdges)
    {
        Version* next = dynamic_cast<Version*>(edge->destVersion());

        if (next)
            next->foldRecurse(_val);
    }
}

bool Version::isRoot() const
{
    return rootnode;
}

bool Version::isFolder() const
{
    return linear.size() > 0;
}

bool Version::isFolded() const
{
    return folded;
}

int Version::getPredecessorHashes(QStringList& _result)
{
    _result.clear();

    QSet<Version*> predecessors;
    getPredecessors(predecessors);
    foreach(Version * it, predecessors)
    {
        _result.push_back(it->getHash());
    }
    return _result.size();
}

int Version::getPredecessors(QSet<Version*>& _result)
{
    _result.clear();

    if (isFolder() && isFolded())
        return linear.front()->getPredecessors(_result);

    foreach (Edge * edge, edgeList)
    {
        if (edge->getInfo() == false
            && edge->sourceVersion() != this)
        {
            Version* predecessor = dynamic_cast<Version*>(edge->sourceVersion());
            if (predecessor)
                _result.insert(predecessor);
        }
    }

    return _result.size();
}

bool Version::getSubtreeHidden() const
{
    return subtreeHidden;
}

void Version::setSubtreeVisible(bool _value)
{
    subtreeHidden = !_value;
    foreach (Edge * edge, outEdges)
    {
        edge->QGraphicsItem::setVisible(_value);
        if (edge->getMerge() == false)
        {
            Version* v = dynamic_cast<Version*>(edge->destVersion());
            if (v)
            {
                v->QGraphicsItem::setVisible(_value);
                v->setSubtreeVisible(_value);
            }
        }
    }
}

void Version::hideSubtree()
{
    setSubtreeVisible(false);
}

void Version::showSubtree()
{
    setSubtreeVisible(true);
}

void Version::setUpdateBoundingRect(bool _val)
{
    updateBoundingRect = _val;
}

void Version::setIsMain(bool _val)
{
    main = _val;
}

bool Version::isMain() const
{
    return main;
}

void Version::setFileConstraint(bool _val)
{
    fileConstraint = _val;
}

bool Version::getFileConstraint() const
{
    return fileConstraint;
}

void Version::reduceToFileConstraint(Version* _parent, bool _merge)
{
    // in case of rootVersion this and _paren are identical
    foreach (const Edge * edge, edgeList)
    {
        if (edge->sourceVersion() != this)
        {
            continue;
        }

        Version* v = dynamic_cast<Version*>(edge->destVersion());

        if (v)
        {
            if (v->getFileConstraint())
            {
                // create new edge
                Edge* e = new Edge (_parent, v, graph, edge->getMerge(), false, true);
                graph->scene()->addItem(e);
                e->adjust();
                e->show();
                if (!_merge)
                    v->reduceToFileConstraint(v, edge->getMerge());
            }
            else
            {
                if (!_merge)
                    v->reduceToFileConstraint(_parent, edge->getMerge());
            }
        }
    }
}

void Version::clearFileConstraintEdgeList()
{
    foreach(Edge * it, fileConstraintOutEdgeList)
    {
        graph->scene()->removeItem(it);
    }
    fileConstraintOutEdgeList.clear();
    fileConstraintInEdgeList.clear();
}

QList<Edge*>& Version::getFileConstraintInEdgeList()
{
    return fileConstraintInEdgeList;
}

QList<Edge*>& Version::getFileConstraintOutEdgeList()
{
    return fileConstraintOutEdgeList;
}

const QMap<QString, QStringList>& Version::getKeyInformation() const
{
    return keyInformation;
}

void Version::setKeyInformation(const QMap<QString, QStringList>& _data)
{
    keyInformation = _data;
}

bool Version::isSelected() const
{
    return selected;
}

bool Version::ensureUnfolded()
{
    bool changed = false;

    if (!isFolder())
    {
        Version* folder = lookupFolderVersion();
        if (folder && folder->isFolded())
        {
            folder->foldAction();
            changed = true;
        }
    }
    return changed;
}

bool Version::isFoldable() const
{
    return foldable;
}
void Version::setIsFoldable(bool _val)
{
    foldable = _val;
}
