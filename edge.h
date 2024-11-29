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

#ifndef __EDGE_H__
#define __EDGE_H__

#include <QGraphicsItem>
#include "graphwidget.h"
#include "version.h"

/**
 * \brief Graphics item representing the edge of the version tree/graph.
 */

class Edge : public QGraphicsItem
{
public:

    /**
     * \brief Constructor for an Edge
     *        Two nodes source and destination are linked by the Edge.
     */
    Edge(class Version* _src,
             class Version* _dst,
                 GraphWidget* _graphWidget,
                 bool _merge = false,
                 bool _info = false,
                 bool _fileConstraint = false,
                 QGraphicsItem* _parent = NULL);

    enum {Type = UserType + 2};
    int type() const
    {
        return Type;
    }

    Node* sourceVersion() const;
    Node* destVersion() const;

    void setMerge(bool _val);
    bool getMerge() const;
    void setInfo(bool _val);
    bool getInfo() const;
    void setConnectorStyle(int _val);
    int getConnectorStyle() const;

    void adjust();
    virtual QRectF boundingRect() const;

    void compareVersions();
    void focusSource();
    void focusDestination();
    void focusNeighbourBox();

protected:
    virtual QPainterPath shape() const;
    virtual void paint(QPainter* _painter, const QStyleOptionGraphicsItem* _option, QWidget* _widget);

private:
    class Version* source;
    class Version* dest;

    GraphWidget* graph;

    QPointF sourcePoint;
    QPointF destPoint;

    qreal arrowSize;

    bool merge;
    bool info;
    bool fileConstraint;
    bool invalid;
};

#endif
