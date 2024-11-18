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

#ifndef __TAGPREFERENCE_H__
#define __TAGPREFERENCE_H__

#include <QColor>
#include <QFont>
#include <QLabel>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QString>
#include <QWidget>

class TagPreference : public QLabel
{
    Q_OBJECT
public:
    TagPreference(const QString& _name,
                  QWidget* _parent = NULL);

    TagPreference(const QString& _name,
                  const QString& _regex,
                  const QString& _color,
                  const QString& _font,
                  const QColor& _bgcolor,
                  bool _visibility,
                  bool _changeable,
                  int _foldable,
                  QWidget* _parent = NULL);

    bool getVisibility() const;
    int getFold() const;
    const QFont& getFont() const;
    const QColor& getColor() const;
    bool getChangeable() const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QRegularExpression& getRegExp() const;
#else
    const QRegExp& getRegExp() const;
#endif
    void setBackgroundColor(const QColor& _bgcolor);

protected:
    void updateLabel();

protected:
    QString regExpText;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QRegularExpression regExp;
#else
    QRegExp regExp;
#endif
    QFont font;
    QColor color;
    QColor bgcolor;
    bool changeable;
    bool visibility;
    int foldable;

    QString path;

public slots:
    void onCustomContextMenu(const QPoint& _pos);

protected slots:
    void changeVisibility(bool _val);
    void changeFoldable(bool _val);
    void changeFont();
    void changeColor();
    void changeRegularExpression();

    void addTagPreference();
    void deleteTagPreference();
    void moveUp();
    void moveDown();

signals:
    void regexpChanged(const QString&);
    void visibilityChanged(const QString&);
    void foldableChanged(const QString&);
    void deleteTagPreference(const QString&);
    void addTagPreference(const QString&);
    void moveUp(TagPreference*);
    void moveDown(TagPreference*);
};

#endif
