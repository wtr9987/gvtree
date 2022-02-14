/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.2-0                */
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

class TagPreference : public QObject
{
    Q_OBJECT
public:
    TagPreference(int _row,
                    const QString& _name, 
                    const QString& _regexDefault,
                    QGridLayout* _parent=NULL);

    const QFont& getFont() const;
    const QRegExp& getRegExp() const;

protected:
    QLabel* tagType;       // "Release Label"
    QLineEdit* regularExpression;
    QRegExp regExp;
    QPushButton* fontButton;
    QFont font;

private slots:
    void setFont();
    void setRegularExpression();
};

#endif
