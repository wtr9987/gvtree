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

#include <QFontDialog>
#include <QColorDialog>
#include <QApplication>
#include "tagpreference.h"

#include <iostream>
using namespace std;

TagPreference::TagPreference(int _row,
                             const QString& _name,
                             const QString& _regexDefault,
                             QGridLayout* _layout) : QObject(NULL)
{
    tagType = new QPushButton(_name);
    connect(tagType, SIGNAL(clicked()), this, SLOT(setColor()));
    _layout->addWidget(tagType, _row, 0, 1, 1);

    fontButton = new QPushButton(QApplication::font().toString());
    connect(fontButton, SIGNAL(clicked()), this, SLOT(setFont()));
    _layout->addWidget(fontButton, _row, 1, 1, 1);

    regularExpression = new QLineEdit();
    _layout->addWidget(regularExpression, _row, 2, 1, 2);

    QString lookupRegexp = _name + "/regExp";
    QSettings settings;

    if (settings.contains(lookupRegexp))
    {
        regularExpression->setText(settings.value(lookupRegexp).toString());
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        regExp = QRegularExpression(settings.value(lookupRegexp).toString());
#else
        regExp = QRegExp(settings.value(lookupRegexp).toString());
#endif
    }
    else
    {
        regularExpression->setText(_regexDefault);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        regExp = QRegularExpression(_regexDefault);
#else
        regExp = QRegExp(_regexDefault);
#endif
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

    QString lookupColor = _name + "/color";

    if (settings.contains(lookupColor))
    {
        color = settings.value(lookupColor).value<QColor>();
    }
    updateColorButton();
}

const QFont& TagPreference::getFont() const
{
    return font;
}

const QColor& TagPreference::getColor() const
{
    return color;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
const QRegularExpression& TagPreference::getRegExp() const
#else
const QRegExp& TagPreference::getRegExp() const
#endif
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

void TagPreference::setColor()
{
    QColor tmpColor = QColorDialog::getColor(color);

    if (tmpColor.isValid())
    {
        color = tmpColor;

        QString settingsKey = tagType->text() + "/color";
        QSettings settings;
        settings.setValue(settingsKey, color.name());
        updateColorButton();
    }
}

void TagPreference::updateColorButton()
{
    QColor fgColor = QColor(0, 0, 0);

    if (color.lightness() < 128)
        fgColor = QColor(255, 255, 255);
    QString css = "background-color: " + color.name() + "; color: " + fgColor.name() + ";";

    tagType->setStyleSheet(css);
}

void TagPreference::setRegularExpression(const QString& _regex)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    regExp = QRegularExpression(_regex);
#else
    regExp = QRegExp(_regex);
#endif

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

void TagPreference::disableRegExp()
{
    regularExpression->setEnabled(false);
}
