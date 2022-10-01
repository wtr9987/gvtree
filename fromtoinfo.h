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

#ifndef __FROMTOINFO_H__
#define __FROMTOINFO_H__

#include <QGraphicsItem>
#include <QWidget>
#include <QString>
#include <QRectF>

#include "version.h"
/**
 * \brief This graphics item is a markup for all versions which are displayed in the CompareTree.
 */
class FromToInfo : public QGraphicsItem
{
public:

    FromToInfo(class GraphWidget* _graph, QGraphicsItem* _parent=NULL);

    enum {Type = UserType + 4};
    int type() const
    {
        return Type;
    }
    virtual QRectF boundingRect() const;

    void setFromToPosition(QSet<Version*> _from, Version* _v);
    virtual void update(const QRectF & rect = QRectF());

protected:
    virtual void paint(QPainter* _painter, const QStyleOptionGraphicsItem* _option, QWidget* _widget);

private:
    QRectF box;
    QSet<Version*> from;
    Version* v;
    class GraphWidget* graph;
};

#endif
