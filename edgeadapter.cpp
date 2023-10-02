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

#include "edgeadapter.h"

EdgeAdapter::EdgeAdapter(Edge* _e, QWidget* _parent):
        QWidget(_parent),
        e(_e)
{
}

void EdgeAdapter::compareVersions()
{
  e->compareVersions();
}

void EdgeAdapter::focusSource()
{
  e->focusSource();
}

void EdgeAdapter::focusDestination()
{
  e->focusDestination();
}

void EdgeAdapter::focusNeighbourBox()
{
  e->focusNeighbourBox();
}

