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
        int rad = dest->getDotRadius();
        if (merge || graph->getConnectorStyle() != 1)
        {
            int rads = source->getDotRadius();
            sourcePoint = line.p1() +
                QPointF((line.dx() * rads) / length, (line.dy() * rads) / length);
            destPoint = line.p2() -
                QPointF((line.dx() * rad) / length, (line.dy() * rad) / length);
        }
        else
        {
            destPoint = line.p2() - QPointF(0, rad);
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
    QVector<QPointF> border;
    float dx = destPoint.x() - sourcePoint.x();
    float dy = destPoint.y() - sourcePoint.y();
    float sx = dx > 0.0 ? 1.0 : -1.0;
    float sy = dy > 0.0 ? 1.0 : -1.0;

    if (merge || graph->getConnectorStyle() == 0)
    {
        QPointF cur = sourcePoint - QPointF(sx * 10.0, sy * 10.0);
        border.push_back(cur);
        cur += QPointF(sx * 20.0, 0.0);
        border.push_back(cur);
        cur += QPointF(dx, dy);
        border.push_back(cur);
        cur += QPointF(0.0, sy * 10.0);
        border.push_back(cur);
        cur -= QPointF(sx * 10.0, 0.0);
        border.push_back(cur);
        cur -= QPointF(dx, dy);
        border.push_back(cur);
    }
    else
    {
        QPointF cur = sourcePoint - QPointF(sx * 10.0, sy * 10.0);
        border.push_back(cur);
        cur += QPointF(sx * 20.0, 0.0);
        border.push_back(cur);
        cur += QPointF(0.0, 0.5 * dy);
        border.push_back(cur);
        cur += QPointF(dx, 0.0);
        border.push_back(cur);
        cur += QPointF(0.0, 0.5 * dy) + QPointF(0.0, sy * 20.0);
        border.push_back(cur);
        cur -= QPointF(sx * 20.0, 0.0);
        border.push_back(cur);
        cur -= QPointF(0.0, 0.5 * dy);
        border.push_back(cur);
        cur -= QPointF(dx, 0.0);
        border.push_back(cur);
    }

    QPainterPath result(border[0]);

    for (int i = 1; i < border.size(); i++)
    {
        result.lineTo(border[i]);
    }
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
    if (invalid || !source->isVisible() || !dest->isVisible())
        return;

    // basic line for edge
    QLineF line(sourcePoint, destPoint);

    if (qFuzzyCompare(line.length(), 0.0))
        return;

    // level of detail
    const qreal lod = _option->levelOfDetailFromTransform(_painter->worldTransform());

    // angle of the edge, only relevant for lod > 0.33
    double angle = ((!fileConstraint && !merge && graph->getConnectorStyle() == 1) || lod <= 0.33) ?
        M_PI / 2.0 :
        (::atan2(line.p2().y() - line.p1().y(), line.p2().x() - line.p1().x()));

    angle += M_PI;

    // double sign = graph->getTopDownView() ? -1.0:1.0;

    // color
    QColor col = info ? graph->getEdgeColor() : merge ? graph->getMergeColor() :
        graph->getEdgeColor();

    // line width
    int pw = (lod <= 0.33) ? 0 : (source->isMain() && dest->isMain()) ? 4 : 2;

    if (fileConstraint)
    {
        // file constraint lines
        pw = (lod > 0.2) ? 5 : 0;
        col = graph->getFileConstraintColor();
    }

    // dim if small
    if (lod < 0.4)
        col.setAlpha(128);

    // line properties
    _painter->setPen(
        QPen(col, pw,
             (lod > 0.2) ? info ? Qt::DotLine : merge ? Qt::DashLine : Qt::SolidLine : Qt::SolidLine,
             Qt::FlatCap,
             Qt::RoundJoin));

    // path
    QPainterPath path(sourcePoint);

    // 90 degree edges?
    if (!fileConstraint && !merge && graph->getConnectorStyle() == 1)
    {
        QPointF p1(sourcePoint.x(), sourcePoint.y() + 0.5 * (destPoint.y() - sourcePoint.y()));
        QPointF p2(destPoint.x(), p1.y());

        path.lineTo(p1);
        path.lineTo(p2);
    }

    // startposition of arrow or destination point
    QPointF p3 = destPoint + ((lod > 0.33) ? arrowSize * 0.707 * QPointF(cos(angle), sin(angle)) : QPointF());

    path.lineTo(p3);

    // paint line
    _painter->drawPath(path);

    // draw arrow cap if lod sufficient
    if (lod > 0.33)
    {
        QPointF off = arrowSize * 0.5 * QPointF(cos(angle + M_PI / 2.0), sin(angle + M_PI / 2.0));

        // solid and cosmetic
        _painter->setPen(QPen(col, 0, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));

        _painter->setBrush(col);
        _painter->drawPolygon(QPolygonF() << line.p2() << p3 + off << p3 - off);
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

    graph->displayHits(source);
}

void Edge::focusDestination()
{
    if (invalid)
      return;

    graph->displayHits(dest);
}

void Edge::focusNeighbourBox()
{
    if (invalid)
      return;

    QList<Version*> tmp;

    tmp.push_back(source);
    tmp.push_back(dest);
    graph->displayHits(tmp);
}
