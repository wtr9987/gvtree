/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.8-0                */
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
#include <QGraphicsScene>
#include <QClipboard>
#include <QApplication>

#include "fromtoinfo.h"
#include "graphwidget.h"

FromToInfo::FromToInfo(GraphWidget* _graph, QGraphicsItem* _parent) : QGraphicsItem(_parent), v(NULL), graph(_graph)
{
    setFlag(ItemSendsGeometryChanges);
    setZValue(1);
}

QRectF FromToInfo::boundingRect() const
{
    return box;
}

void FromToInfo::setFromToPosition(QSet<Version*> _from, Version* _v)
{
    from = _from;
    v = _v;
    if (v==NULL && from.size())
      v = *from.begin();
    else if (v && v->isVisible() == false && from.size())
      v = *from.begin();
    update();
}

void FromToInfo::update(const QRectF&)
{
    if (v)
    {
        setPos(v->QGraphicsItem::scenePos());
        box = QRectF(-20, -20, 40, 40);
        foreach(Version * it, from)
        {
            QPointF toPosition = it->QGraphicsItem::scenePos() - QGraphicsItem::scenePos();
            box |= QRectF(-20, -20, 40, 40).translated(toPosition);
        }
    }
    QGraphicsItem::update();
}

void FromToInfo::paint(QPainter* _painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (v)
    {
        _painter->setPen(QPen(graph->getFromToColor(), 10));
        _painter->setBrush(graph->getFromToColor());

        QPainterPath path;
        path.moveTo(QPointF(0, 0));
        path.addEllipse(QRectF(-15, -15, 30, 30));

        foreach(Version * it, from)
        {
            path.moveTo(QPointF(0, 0));
            QPointF toPosition = it->QGraphicsItem::scenePos() - QGraphicsItem::scenePos();

            QLineF line(QPointF(0, 0), toPosition);
            if (!qFuzzyCompare(line.length(), 0.0))
            {
                path.lineTo(toPosition);
                path.addEllipse(QRectF(-15, -15, 30, 30).translated(toPosition));
            }
        }

        _painter->drawPath(path);
    }
}
