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

/* Reference:
 * Rachel Lim's Blog
 * https://rachel53461.wordpress.com/
 * Algorithm for Drawing Trees
 *
 * Remark:
 * The shiftTree() here is a little different.
 * The 'middle' nodes are not shifted.
 */

#include <iostream>

#include "node.h"
#include "edge.h"

using namespace std;

Node::Node() : xval(0.0f), yval(0), mod(0.0f), hval(1)
{
}

void Node::addShift(float _modSum)
{
    xval += _modSum;
    _modSum += mod;

    foreach (const Edge * edge, outEdges)
    {
        edge->destVersion()->addShift(_modSum);
    }
}

void Node::simpleTreeGeometry(Node* _sibling)
{
    // just set y level for each node
    // and give siblings an 'index' position
    Node* sibling = NULL;

    foreach (const Edge * edge, outEdges)
    {
        edge->destVersion()->setY(yval + hval);
        edge->destVersion()->simpleTreeGeometry(sibling);
        sibling = edge->destVersion();
    }

    if (_sibling)
        xval = _sibling->getX() + 1.0f;
}

void Node::centerParents(Node* _parent)
{
    if (outEdges.size() == 0)
    {
        mod = _parent->getMod();
    }
    else
    {
        float minChildX = 10e6;
        float maxChildX = -10e6;

        foreach (const Edge * edge, outEdges)
        {
            edge->destVersion()->centerParents(this);

            float childX = edge->destVersion()->getX();
            minChildX = (childX < minChildX)?childX:minChildX;
            maxChildX = (childX > maxChildX)?childX:maxChildX;
        }

        mod = xval - (minChildX + (maxChildX - minChildX) / 2.0f);
    }
}

void Node::shiftTree()
{
    Node* sibling = NULL; // left sibling

    QMap<int, float> rightContour; // keep right contour

    foreach (const Edge * edge, outEdges)
    {
        Node* current = edge->destVersion();

        // recurse
        current->shiftTree();

        if (sibling)
        {
            sibling->getContour(0.0f, rightContour, true);

            // sibling is the left sibling, current is right
            QMap<int, float> leftContour;
            current->getContour(0.0f, leftContour, false);

            // get
            float shift = -1.0f;
            QMap<int, float>::iterator lit = leftContour.begin();
            QMap<int, float>::iterator rit = rightContour.begin();
            while (lit != leftContour.end() && rit != rightContour.end())
            {
                float shift_tmp = rit.value() - lit.value();
                if (shift_tmp > shift)
                    shift = shift_tmp;
                lit++;
                rit++;
            }
            if (shift >= 0.0f)
            {
                shift += 1.0f;
                current->addX(shift); 
                current->addMod(shift);
            }
        }
        sibling = current;
    }
}

void Node::getContour(float _modSum, QMap<int, float>& _contour, bool _right) const
{
    float val = xval + _modSum;

    if (!_contour.contains(yval) || ((val > _contour[yval]) == _right))
    {
        _contour[yval] = val;
    }

    _modSum += mod;
    foreach (const Edge * edge, outEdges)
    {
        edge->destVersion()->getContour(_modSum, _contour, _right);
    }
}

void Node::setMod(const float& _val)
{
    mod = _val;
}

void Node::setX(const float& _val)
{
    xval = _val;
}

void Node::addMod(const float& _val)
{
    mod += _val;
}

void Node::addX(const float& _val)
{
    xval += _val;
}

float Node::getMod() const
{
    return mod;
}

float Node::getX() const
{
    return xval;
}

int Node::getY() const
{
    return yval;
}

void Node::addOutEdge(Edge* _edge)
{
    outEdges << _edge;
}

void Node::setY(const int& _y)
{
    yval = _y;
}

void Node::setH(int _val)
{
    hval = _val;
}

int Node::getH() const
{
    return hval;
}

QList<class Edge*>& Node::getOutEdges()
{
    return outEdges;
}

int Node::getNumOutEdges() const
{
    return outEdges.size();
}
