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
                  const QString& _regexDefault,
                  const QString& _colorDefault,
                  const QString& _fontDefault,
                  const QColor& _bgcolor,
                  bool _visibility,
                  bool _regexpChangeable,
                  QWidget* _parent = NULL);

    void initDefault(const QString& _regexDefault = QString(),
                     const QString& _colorDefault = QString(),
                     const QString& _fontDefault = QString(),
                     bool _visibility = false);

    bool getVisibility() const;
    const QFont& getFont() const;
    const QColor& getColor() const;
    bool getChangeable() const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QRegularExpression& getRegExp() const;
#else
    const QRegExp& getRegExp() const;
#endif

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
    bool regexpChangeable;
    bool visibility;

public slots:
    void setBackgroundColor(const QColor& _bgcolor);
    void onCustomContextMenu(const QPoint& _pos);

protected slots:
    void changeVisibility(bool _val);
    void changeFont();
    void changeColor();
    void changeRegularExpression();

    void addTagPreference();
    void deleteTagPreference();

signals:
    void regexpChanged();
    void visibilityChanged();
    void deleteTagPreference(const QString&);
    void addTagPreference(const QString&);
};

#endif
