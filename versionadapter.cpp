/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.5-0                */
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

#include "versionadapter.h"

VersionAdapter::VersionAdapter(Version* _v, QWidget* _parent) :
    QWidget(_parent),
    v(_v)
{
}

void VersionAdapter::compareToSelected()
{
    v->compareToSelected();
}

void VersionAdapter::compareToPrevious()
{
    v->compareToPrevious();
}

void VersionAdapter::compareToLocalHead()
{
    v->compareToLocalHead();
}

void VersionAdapter::compareToBranchBaseline()
{
    v->compareToBranchBaseline();
}

void VersionAdapter::viewThisVersion()
{
    v->viewThisVersion();
}

void VersionAdapter::focusNeighbourBox()
{
    v->focusNeighbourBox();
}

void VersionAdapter::foldAction()
{
    v->foldAction();
    emit foldSignal(v);
}

void VersionAdapter::hideSubtree()
{
    v->setSubtreeVisible(false);
}

void VersionAdapter::showSubtree()
{
    v->setSubtreeVisible(true);
}
