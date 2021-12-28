/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.1-0                */
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

#ifndef __NODE_H__
#define __NODE_H__

#include <QList>
#include <QMap>

/**
 * \brief This class just contains the basic information
 *        to rener a collision free tree graph.
 */
class Node
{
public:
    // CTOR
    Node();

    // collect out edges
    virtual void addOutEdge(class Edge* _edge);

    // tree graph geometry
    void setX(const float& _val);
    void addX(const float& _val);
    void setY(const int& _val);
    void setMod(const float& _val);
    void addMod(const float& _val);
    float getX() const;
    int getY() const;
    float getMod() const;
    void setH(int _val);
    int getH() const;

    void simpleTreeGeometry(Node* _sibling);
    void centerParents(Node* _parent);
    void shiftTree();
    void addShift(float _modSum);

    void getContour(float _modSum, QMap<int, float>& _contour, bool _right) const;

    QList<class Edge*>& getOutEdges();

    int getNumOutEdges() const;

protected:
    QList<class Edge*> outEdges;

    float xval;
    int yval;
    float mod;
    int hval;
};

#endif
