/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.6-0                */
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

#ifndef __EDGEADAPTER_H__
#define __EDGEADAPTER_H__

#include <QWidget>
#include "edge.h"

class EdgeAdapter : public QWidget
{
    Q_OBJECT
public:

    EdgeAdapter(Edge* _e, QWidget* _parent = NULL);

public slots:
    void compareVersions();
    void focusSource();
    void focusDestination();
    void focusNeighbourBox();

private:
    Edge* e;
};

#endif
