/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.7-0                */
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
    v->compareToSelected(false);
}

void VersionAdapter::compareToPrevious()
{
    v->compareToPrevious(false);
}

void VersionAdapter::compareToLocalHead()
{
    v->compareToLocalHead(false);
}

void VersionAdapter::compareToBranchBaseline()
{
    v->compareToBranchBaseline(false);
}

void VersionAdapter::diffToSelected()
{
    v->compareToSelected(true);
}

void VersionAdapter::diffToPrevious()
{
    v->compareToPrevious(true);
}

void VersionAdapter::diffToLocalHead()
{
    v->compareToLocalHead(true);
}

void VersionAdapter::diffToBranchBaseline()
{
    v->compareToBranchBaseline(true);
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
