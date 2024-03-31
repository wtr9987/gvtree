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

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
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
    selected(false),
    weight(0),
    commitDate(0)
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
    selected(false),
    weight(0),
    commitDate(0)
{
    // flags
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    folderBox = QRectF(-30, -30, 60, 60);
    setZValue(5);
}

void Version::compareToSelected(bool _view)
{
    graph->compareToSelected(this, _view);
}

void Version::compareToPrevious(bool _view)
{
    graph->compareToPrevious(this, _view);
}

void Version::compareToLocalHead(bool _view)
{
    graph->compareToLocalHead(this, _view);
}

void Version::compareToBranchBaseline(bool _view)
{
    graph->compareToBranchBaseline(this, _view);
}

void Version::viewThisVersion()
{
    graph->viewThisVersion(this);
}

void Version::focusNeighbourBox()
{
    graph->displayHits(getNeighbourBox());
}

QList<Version*> Version::getNeighbourBox()
{
    // in case of a folder...
    if (isFolder() && isFolded())
        return linear.front()->getNeighbourBox();

    QList<Version*> result;

    result.push_back(this);

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
            result.push_back(v);
    }

    return result;
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
    QApplication::clipboard()->setText(_val ? hash : QString(), QClipboard::Clipboard);

    if (selected != _val)
    {
        selected = _val;

        adjustEdges();

        QTextEdit* t = graph->getMainWindow()->getCompareTreeSelectedLog();

        if (_val == false)
        {
            t->clear();
        }
        else
        {
            graph->commitInfo(this, t);
        }
    }
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
    {
        int rad = getDotRadius();
        path.addEllipse(-rad, -rad, 2 * rad, 2 * rad);
    }
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

    if (subtreeHidden)
    {
        _painter->setPen(QPen(graph->getEdgeColor(), 3, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
        _painter->setBrush(graph->getEdgeColor());
        _painter->drawLine(0, 0, 0, (graph->getMainWindow()->getTopDownView() ? -1 : 1) * graph->getYFactor() / 3);
    }

    _painter->setPen(QPen(Qt::black, 0));

    int rad = getDotRadius();

    if (matched)
    {
        _painter->setPen(QPen(graph->getSearchColor(), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        _painter->setBrush(graph->getSearchColor().lighter());
        _painter->drawEllipse(-rad + 4, -rad + 4, 2 * rad - 8, 2 * rad - 8);
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

    _painter->drawEllipse(-rad, -rad, 2 * rad, 2 * rad);

    if (lod > 0.3)
    {
        int height = -10;
        bool textborder = graph->getMainWindow()->getTextBorder();

        foreach(const QString& info, graph->getMainWindow()->getNodeInfo())
        {
            if (globalVersionInfo.contains(info)
                || localVersionInfo.contains(info))
            {
                QMap<QString, QStringList>::const_iterator kit = keyInformation.find(info);
                if (kit != keyInformation.end())
                {
                    drawTextBox(info, kit.value(), height, lod, _painter, textborder);
                }
            }
        }

        // debug : draw bounding box
        //_painter->setPen(QPen(QColor(255, 0, 0), 0));
        //_painter->setBrush(QBrush());
        //_painter->drawRect(localBoundingBox);
    }
}

int Version::getDotRadius() const
{
    // if foldd, the relevant geometry is contained in the folder node
    // it is needed for the adjustment of the in edges of the folded folder
    const Version* v = lookupFoldedFolderVersion();

    if (v->getMatched() || v->isSelected())
    {
        return 20;
    }
    else
        return 10;
}

QVariant Version::itemChange(GraphicsItemChange _change, const QVariant& _value)
{
    if (_change == ItemPositionHasChanged && blockItemChanged == false)
        adjustEdges();

    return QGraphicsItem::itemChange(_change, _value);
}

void Version::adjustEdges()
{
    if (isFolder() && isFolded())
    {
        // in case of a folded folder the version to start adjustment
        // is the first folder version
        getFolderVersions().front()->adjustEdgesRecurse();
    }
    else
    {
        adjustEdgesRecurse();
    }
}

void Version::adjustEdgesRecurse()
{
    foreach(QGraphicsItem * it, childItems())
    {
        Version* v = dynamic_cast<Version*>(it);

        if (v)
            v->adjustEdgesRecurse();
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

QString Version::getCommitDateString() const
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
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    keyInformation[QString("Commit Date")] =
        QStringList(
            QDateTime::fromSecsSinceEpoch(_parts.at(2).toInt())
            .toString("yyyy.MM.dd HH:mm:ss"));
#else
    keyInformation[QString("Commit Date")] =
        QStringList(
            QDateTime::fromTime_t(_parts.at(2).toInt())
            .toString("yyyy.MM.dd HH:mm:ss"));
#endif
    commitDate = _parts.at(2).toInt();
    keyInformation[QString("User Name")] = QStringList(_parts.at(3));

    // tag information
    processGitLogTagInformation(_parts.at(4));

    // commit comment
    processGitLogCommentInformation(_parts.at(5));

    return true;
}

void Version::updateCommentInformation(int _columns, int _maxlen)
{
    QStringList& commentRaw = keyInformation[QString("CommentRaw")];

    if (commentRaw.size() == 0)
        return;

    QString comment = commentRaw.join(" ");
    QString info = comment;

    if (_maxlen)
    {
        info = comment.mid(0, _maxlen);
        if (comment.size() > _maxlen)
            info = info + "...";
    }

    keyInformation[QString("Comment")] = QStringList();

    int len = 0;
    QString part;
    QStringList tmp = info.split(' ');

    foreach (const QString& str, tmp)
    {
        if (len == 0)
        {
            part = str;
            len = str.size();
        }
        else if (_columns == 0 || len < _columns)
        {
            part = part + " " + str;
            len += 1 + str.size();
        }

        if (_columns && len >= _columns)
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

void Version::processGitLogCommentInformation(const QString& _comment)
{
    keyInformation[QString("CommentRaw")].push_back(_comment);

    int columns, maxlen;

    graph->getMainWindow()->getCommentProperties(columns, maxlen);
    updateCommentInformation(columns, maxlen);
}

void Version::processGitLogTagInformation(const QString& _tagInfo)
{
    int cstart = _tagInfo.indexOf(QChar('(')) + 1;
    int cend = _tagInfo.indexOf(QChar(')'));

    if (cstart >= 0 && cend >= 0)
    {
        QStringList matches = _tagInfo.mid(cstart, cend - cstart).split(',');
        QStringList tags;

        foreach (const QString& str, matches)
        {
            tags << str.trimmed();
        }

        QStringList scanItems (QStringList()
                               << QString("HEAD")
                               << QString("Release Label")
                               << QString("Baseline Label")
                               << QString("FIX/PQT Label")
                               << QString("HO Label")
                               << QString("Branch")
                               << QString("Other Tags"));

        foreach(const QString& it, scanItems)
        {
            const TagPreference* tp =
                graph->getMainWindow()->getTagPreference(it);

            if (tp)
            {
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
                QRegularExpression r = tp->getRegExp();
                foreach (const QString& str, tags)
                {
                    QRegularExpressionMatch m = r.match(str);

                    if (m.hasMatch())
                    {
                        keyInformation[it].push_back(m.captured(1));
                        tags.removeOne(str);
                    }
                }
#else
                QRegExp r = tp->getRegExp();
                foreach (const QString& str, tags)
                {
                    if (r.indexIn(str, 0) != -1)
                    {
                        keyInformation[it].push_back(r.cap(1));
                        tags.removeOne(str);
                    }
                }
#endif
            }
        }
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
bool Version::findMatch(QRegularExpression& _pattern, const QString& _text, bool _exactMatch, QString _keyConstraint)
#else
bool Version::findMatch(QRegExp& _pattern, const QString& _text, bool _exactMatch, QString _keyConstraint)
#endif
{
    bool oldmatched = matched;
    bool newmatched = false;

    QSet<QString> oldLocalVersionInfo = localVersionInfo;

    localVersionInfo.clear();

    QString rawInput = keyInformation["_input"].join(" ")
        + " " + keyInformation["Commit Date"].join(" ");

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    bool checkDetail = _pattern.isValid() ? (_pattern.match(rawInput).hasMatch()) :
        (rawInput.indexOf(_text, 0) != -1);
#else
    bool checkDetail = _pattern.isValid() ? (_pattern.indexIn(rawInput, 0) != -1) :
        (rawInput.indexOf(_text, 0) != -1);
#endif

    if (checkDetail)
    {
        for (QMap<QString, QStringList>::iterator kit = keyInformation.begin();
             kit != keyInformation.end();
             kit++)
        {
            if (kit.key() == QString("_input"))
                continue;

            if (_keyConstraint.size() && kit.key() != _keyConstraint)
                continue;

            if (_exactMatch == true)
            {
                foreach (const QString& str, kit.value())
                {
                    if (_keyConstraint.size() && str == _text)
                    {
                        newmatched = true;
                        if (kit.key() == "CommentRaw")
                            localVersionInfo.insert("Comment");
                        else
                            localVersionInfo.insert(kit.key());
                        break;
                    }
                }
                if (newmatched)
                    break;
            }
            else
            {
                QString tmp = kit.value().join(QString(" "));

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
                if ((_pattern.isValid() && _pattern.match(tmp).hasMatch())
                    || (tmp.indexOf(_text) != -1))
#else
                if ((_pattern.isValid() && _pattern.indexIn(tmp, 0) != -1)
                    || (tmp.indexOf(_text) != -1))
#endif
                {
                    newmatched = true;
                    if (kit.key() == "CommentRaw")
                        localVersionInfo.insert("Comment");
                    else
                        localVersionInfo.insert(kit.key());
                }
            }
        }
    }
    if (newmatched != oldmatched
        || localVersionInfo != oldLocalVersionInfo)
    {
        if (newmatched == true)
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

    QRectF textbox = QFontMetricsF(tp->getFont()).boundingRect("X");
    int hadd = textbox.height() + 1;

    foreach(const QString it, _values)
    {
        textbox = QFontMetricsF(tp->getFont()).boundingRect(it);
        _height += hadd;
        _updatedBox |= textbox.translated(20, _height).adjusted(0, 0, 20, 0);
    }
    return true;
}

bool Version::drawTextBox(const QString& _key, const QStringList& _values, int& _height, const qreal& _lod, QPainter* _painter, bool _frame)
{
    const TagPreference* tp = graph->getMainWindow()->getTagPreference(_key);

    if (!tp)
        return false;

    QRectF textbox = QFontMetricsF(tp->getFont()).boundingRect("X");

    if (textbox.height() * _lod > 7)
    {
        int hadd = textbox.height() + 1;
        foreach(const QString& it, _values)
        {
            _height += hadd;

            textbox = QFontMetricsF(tp->getFont()).boundingRect(it);
            _painter->setFont(tp->getFont());

            if (_frame)
            {
                _painter->setPen(QPen(graph->getBackgroundColor(), 0));
                _painter->drawText(textbox.translated(19, _height).adjusted(0, 0, 20, 0), Qt::AlignLeft, it);
                _painter->drawText(textbox.translated(21, _height).adjusted(0, 0, 20, 0), Qt::AlignLeft, it);
                _painter->drawText(textbox.translated(20, _height - 1).adjusted(0, 0, 20, 0), Qt::AlignLeft, it);
                _painter->drawText(textbox.translated(20, _height + 1).adjusted(0, 0, 20, 0), Qt::AlignLeft, it);
            }

            _painter->setPen(QPen(tp->getColor(), 0));
            _painter->drawText(textbox.translated(20, _height).adjusted(0, 0, 20, 0), Qt::AlignLeft, it);
        }
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
        adjustEdges();
        calculateLocalBoundingBox();
        QGraphicsItem::update();
    }
}

void Version::addLocalVersionInfo(const QString& _val)
{
    localVersionInfo.insert(_val);
}

bool Version::getMatched() const
{
    return matched;
}

void Version::calculateLocalBoundingBox()
{
    localBoundingBox = folderBox | QRectF(-30, -30, 60, 60);

    if (subtreeHidden)
    {
        localBoundingBox.adjust(0, 0, 0, (graph->getMainWindow()->getTopDownView() ? -1 : 1) * graph->getYFactor() / 3);
    }

    int height = 0;

    foreach(const QString& info, graph->getMainWindow()->getNodeInfo())
    {
        if (globalVersionInfo.contains(info)
            || localVersionInfo.contains(info))
        {
            QMap<QString, QStringList>::const_iterator kit = keyInformation.find(info);
            if (kit != keyInformation.end())
            {
                getTextBoundingBox(kit.key(), kit.value(), height, localBoundingBox);
            }
        }
    }
    localBoundingBox.adjust(0, 0, 0, 10);

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
        && (numEdges() - getNumOutEdges() <= 1)
        && _parent->isFoldable()
       )
    {
        addToFolder(_parent);
    }
    else
    {
        if (_parent && !_parent->isFolder())
        {
            _parent->setFolded(false);
        }
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

const QList<Version*>& Version::getFolderVersions() const
{
    return linear;
}

void Version::clearFolderVersions()
{
    linear.clear();
}

const Version* Version::lookupFoldedFolderVersion() const
{
    if (folded && linear.size() == 0 && outEdges.size() == 1)
    {
        const Version* v = dynamic_cast<Version*>(outEdges.front()->destVersion());
        if (v && v->isFolded())
        {
            return v->lookupFoldedFolderVersion();
        }
    }
    return this;
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
        v->setFolded(folded);
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
            .translated(0, graph->getTopDownView() ? 0.0 : -1.0 * h);
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
                if (graph->isFromToVersion(v))
                {
                    graph->resetDiff();
                }
                if (_value == false && v->isSelected())
                {
                    graph->resetSelection();
                }
            }
        }
    }
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
    // in case of rootVersion this and _parent are identical
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

    if (!isFolder() && isFolded())
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

void Version::setFolded(bool _val)
{
    folded = _val;
}

void Version::applyHorizontalSort(int _sort)
{
    foreach (const Edge * edge, outEdges)
    {
        Version* next = dynamic_cast<Version*>(edge->destVersion());

        if (next)
            next->applyHorizontalSort(_sort);
    }

    if (outEdges.size() < 2)
        return;

    switch (_sort)
    {
        case 1:
        {
            struct
            {
                bool operator ()(const Edge* a, const Edge* b) const
                {
                    const Version* av = dynamic_cast<Version*>(a->destVersion());
                    const Version* bv = dynamic_cast<Version*>(b->destVersion());
                    int aw = !av ? 0 : av->getWeight();
                    int bw = !bv ? 0 : bv->getWeight();

                    return aw < bw;
                }
            } weightLt;
            std::sort(outEdges.begin(), outEdges.end(), weightLt);
        }

        break;
        case 2:
        {
            struct
            {
                bool operator ()(const Edge* a, const Edge* b) const
                {
                    const Version* av = dynamic_cast<Version*>(a->destVersion());
                    const Version* bv = dynamic_cast<Version*>(b->destVersion());
                    int aw = !av ? 0 : av->getWeight();
                    int bw = !bv ? 0 : bv->getWeight();

                    return aw > bw;
                }
            } weightGt;
            std::sort(outEdges.begin(), outEdges.end(), weightGt);
        }

        break;
        case 3:
        {
            struct
            {
                bool operator ()(const Edge* a, const Edge* b) const
                {
                    const Version* av = dynamic_cast<Version*>(a->destVersion());
                    const Version* bv = dynamic_cast<Version*>(b->destVersion());
                    long aw = av->getCommitDate();
                    long bw = bv->getCommitDate();

                    return aw < bw;
                }
            } commitDateLt;
            std::sort(outEdges.begin(), outEdges.end(), commitDateLt);
        }
        break;
        case 4:
        {
            struct
            {
                bool operator ()(const Edge* a, const Edge* b) const
                {
                    const Version* av = dynamic_cast<Version*>(a->destVersion());
                    const Version* bv = dynamic_cast<Version*>(b->destVersion());
                    long aw = av->getCommitDate();
                    long bw = bv->getCommitDate();

                    return aw > bw;
                }
            } commitDateGt;
            std::sort(outEdges.begin(), outEdges.end(), commitDateGt);
        }
        break;
    }
}

int Version::calculateWeightRecurse()
{
    weight = 1 + linear.size();
    foreach (const Edge * edge, outEdges)
    {
        Version* next = dynamic_cast<Version*>(edge->destVersion());

        weight += next ? next->calculateWeightRecurse() : 0;
    }
    return weight;
}

int Version::getWeight() const
{
    return weight;
}

long Version::getCommitDate() const
{
    return commitDate;
}
