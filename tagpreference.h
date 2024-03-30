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

#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QSettings>
#include <QString>
#include <QGridLayout>
#include <QObject>
#include <QColor>

class TagPreference : public QObject
{
    Q_OBJECT
public:
    TagPreference(int _row,
                  const QString& _name,
                  const QString& _regexDefault,
                  QGridLayout* _parent = NULL);

    const QFont& getFont() const;
    const QColor& getColor() const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QRegularExpression& getRegExp() const;
#else
    const QRegExp& getRegExp() const;
#endif
    void disableRegExp();

protected:
    void updateColorButton();

protected:
    QPushButton* tagType;
    QLineEdit* regularExpression;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QRegularExpression regExp;
#else
    QRegExp regExp;
#endif
    QPushButton* fontButton;
    QFont font;
    QColor color;

private slots:
    void setFont();
    void setColor();
    void setRegularExpression(const QString& _regex);

signals:
    void regexpChanged();
};

#endif
