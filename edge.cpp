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

#include <QPainter>
#include <QColor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QClipboard>
#include <QApplication>

#include <math.h>

#include "edge.h"
#include "version.h"

Edge::Edge(Version* _src,
           Version* _dst,
           GraphWidget* _graphWidget,
           bool _merge,
           bool _info,
           bool _fileConstraint,
           QGraphicsItem* _parent)
    : QGraphicsItem(_parent),
    source(_src),
    dest(_dst),
    graph(_graphWidget),
    arrowSize(10),
    merge(_merge),
    info(_info),
    fileConstraint(_fileConstraint),
    invalid(!_src || !_dst)
{
    // source->dest
    if (source && !fileConstraint)
    {
        source->addOutEdge(this);
    }
    if (dest && !fileConstraint)
    {
        dest->addInEdge(this);
    }
    if (fileConstraint && source && dest)
    {
        source->getFileConstraintOutEdgeList().push_back(this);
        dest->getFileConstraintInEdgeList().push_back(this);
        setZValue(5);
    }
    else
    {
        setZValue(4);
    }

    // flags
    setFlag(ItemIsMovable, false);
    // setCacheMode(DeviceCoordinateCache);
}

Node* Edge::sourceVersion() const
{
    return source;
}

Node* Edge::destVersion() const
{
    return dest;
}

void Edge::adjust()
{
    if (invalid)
        return;

    sourcePoint = mapFromItem(source, 0, 0);
    destPoint = mapFromItem(dest, 0, 0);

    QLineF line(sourcePoint, destPoint);
    float length = line.length();

    prepareGeometryChange();

    if (length > 1)
    {
        if (merge || graph->getConnectorStyle() != 1)
        {
            QPointF edgeOffset((line.dx() * 10) / length, (line.dy() * 10) / length);
            sourcePoint = line.p1() + edgeOffset;
            destPoint = line.p2() - edgeOffset;
        }
        else
        {
            destPoint = line.p2() - QPointF(0, 10);
        }
    }
}

QRectF Edge::boundingRect() const
{
    if (invalid)
        return QRectF();

    float penWidth = 1.0f;
    float extra = (penWidth + arrowSize);

    return QRectF(sourcePoint, destPoint)
           .normalized()
           .adjusted(-extra, -extra, extra, extra);
}

QPainterPath Edge::shape() const
{
    QPainterPath result;

    QPointF v = destPoint - sourcePoint;

    v = 10.0 * v / sqrt(v.x() * v.x() + v.y() * v.y());
    QPointF v_(v.y(), -v.x());

    result = QPainterPath(sourcePoint + v + v_);
    result.lineTo(sourcePoint + v - v_);
    result.lineTo(destPoint - v - v_);
    result.lineTo(destPoint - v + v_);
    result.closeSubpath();

    return result;
}

void Edge::setMerge(bool _val)
{
    merge = _val;
}

bool Edge::getMerge() const
{
    return merge;
}

void Edge::setInfo(bool _val)
{
    setFlag(ItemIsSelectable, !_val);
    info = _val;
}

bool Edge::getInfo() const
{
    return info;
}

void Edge::paint(QPainter* _painter,
                 const QStyleOptionGraphicsItem* _option, QWidget*)
{
    if (invalid)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), 0.0))
        return;

    const qreal lod = _option->levelOfDetailFromTransform(_painter->worldTransform());

    QColor col = info ? graph->getEdgeColor() : merge ? graph->getMergeColor() :
        graph->getEdgeColor();

    int pw = (lod > 0.33 && source->isMain() && dest->isMain()) ? 3 : 0;

    if (fileConstraint)
    {
        pw = (lod > 0.2) ? 5 : 0;
        col = graph->getFileConstraintColor();
    }

    _painter->setPen(
        QPen(col, pw,
             info ? Qt::DotLine : merge ? Qt::DashLine : Qt::SolidLine,
             Qt::RoundCap,
             Qt::RoundJoin));

    QPainterPath path;
    path.moveTo(sourcePoint);
    if (!fileConstraint && !merge && graph->getConnectorStyle() == 1)
    {
        //
        QPointF p1(sourcePoint.x(), sourcePoint.y() +
                   (graph->getTopDownView() ? -0.5 : 0.5) * graph->getYFactor());
        QPointF p2(destPoint.x(), p1.y());

        path.lineTo(p1);
        path.lineTo(p2);
    }
    path.lineTo(destPoint);

    _painter->drawPath(path);

    if (lod > 0.33)
    {
        double angle = ::acos(line.dx() / line.length());
        if (!merge && graph->getConnectorStyle() == 1)
            angle = M_PI / 2.0;
        if (line.dy() >= 0)
            angle = 2.0 * M_PI - angle;

        QPointF destArrowP1 = destPoint + QPointF(sin(angle - M_PI / 3) * arrowSize,
                                                  cos(angle - M_PI / 3) * arrowSize);
        QPointF destArrowP2 = destPoint + QPointF(sin(angle - M_PI + M_PI / 3) * arrowSize,
                                                  cos(angle - M_PI + M_PI / 3) * arrowSize);

        _painter->setBrush(col);
        _painter->drawPolygon(QPolygonF() << line.p1());
        _painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
    }
}

void Edge::compareVersions()
{
    if (invalid)
        return;

    graph->compareVersions(source, dest);
}

void Edge::focusSource()
{
    if (invalid)
        return;

    QRectF box = source->getNeighbourBox();
    graph->focusNeighbourBox(box);
}

void Edge::focusDestination()
{
    if (invalid)
        return;

    QRectF box = dest->getNeighbourBox();
    graph->focusNeighbourBox(box);
}

void Edge::focusNeighbourBox()
{
    if (invalid)
        return;

    QRectF box = source->getNeighbourBox() | dest->getNeighbourBox();
    graph->focusNeighbourBox(box);
}
