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

#include <QFontDialog>
#include <QApplication>
#include "tagpreference.h"

#include <iostream>
using namespace std;

TagPreference::TagPreference(int _row,
                             const QString& _name,
                             const QString& _regexDefault,
                             QGridLayout* _layout) : QObject(NULL)
{
    tagType = new QLabel(_name);
    _layout->addWidget(tagType, _row, 0, 1, 1);

    fontButton = new QPushButton(QApplication::font().toString());
    connect(fontButton, SIGNAL(clicked()), this, SLOT(setFont()));
    _layout->addWidget(fontButton, _row, 1, 1, 1);

    regularExpression = new QLineEdit();
    _layout->addWidget(regularExpression, _row, 2, 1, 1);

    QString lookupRegexp = _name + "/regExp";
    QSettings settings;
    if (settings.contains(lookupRegexp))
    {
        regularExpression->setText(settings.value(lookupRegexp).toString());
        regExp = QRegExp(settings.value(lookupRegexp).toString());
    }
    else
    {
        regularExpression->setText(_regexDefault);
        regExp = QRegExp(_regexDefault);
    }

    connect(regularExpression, SIGNAL(textChanged(const QString&)), this, SLOT(setRegularExpression(const QString&)));

    QString lookupFont = _name + "/font";
    if (settings.contains(lookupFont))
    {
        fontButton->setText(settings.value(lookupFont).toString());
        font.fromString(settings.value(lookupFont).toString());
    }
    else
    {
        font.fromString(fontButton->text());
    }
}

const QFont& TagPreference::getFont() const
{
    return font;
}

const QRegExp& TagPreference::getRegExp() const
{
    return regExp;
}

void TagPreference::setFont()
{
    QFontDialog dialog(font, NULL);

    if (dialog.exec())
    {
        font = dialog.selectedFont();
        fontButton->setText(font.toString());
        QString settingsKey = tagType->text() + "/font";
        QSettings settings;
        settings.setValue(settingsKey, font.toString());
    }
}

void TagPreference::setRegularExpression(const QString& _regex)
{
    regExp = QRegExp(_regex);

    if (regExp.isValid())
    {
        QSettings settings;
        QString lookupRegexp = tagType->text() + "/regExp";
        settings.setValue(lookupRegexp, _regex);
        regularExpression->setStyleSheet("color: black;  background-color: white");
        emit regexpChanged();
    }
    else
    {
        regularExpression->setStyleSheet("color: black;  background-color: red");
    }
}
