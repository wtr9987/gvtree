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

#ifndef __VERSIONADAPTER_H__
#define __VERSIONADAPTER_H__

#include <QWidget>
#include "version.h"

class VersionAdapter : public QWidget
{
    Q_OBJECT

public:
    VersionAdapter(Version* _v, QWidget* _parent = NULL);

public slots:
    void compareToSelected();
    void compareToPrevious();
    void compareToLocalHead();
    void compareToBranchBaseline();
    void diffToSelected();
    void diffToPrevious();
    void diffToLocalHead();
    void diffToBranchBaseline();
    void viewThisVersion();
    void focusNeighbourBox();
    void foldAction();
    void hideSubtree();
    void showSubtree();
private:
    Version* v;

    // a signal is emit to normalize and redraw the graph again.
signals:
    void foldSignal(Version*);
};

#endif
