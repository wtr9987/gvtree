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

#ifndef __TAGPREFERENCE_H__
#define __TAGPREFERENCE_H__

#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
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
    const QRegExp& getRegExp() const;
    void disableRegExp();

protected:
    void updateColorButton();

protected:
    QPushButton* tagType;
    QLineEdit* regularExpression;
    QRegExp regExp;
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
