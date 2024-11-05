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
                             const QString& _colorDefault,    // default, if not defined in settings
                             const QString& _fontDefault,      // default, if not defined in settings
                             QGridLayout* _layout) : QObject(NULL)
{
    QSettings settings;

    // regular expression
    regularExpression = new QLineEdit();
    _layout->addWidget(regularExpression, _row, 2, 1, 2);

    QString lookupRegexp = _name + "/regExp";

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

    // tag and color
    tagType = new QPushButton(_name);

    QString lookupColor = _name + "/color";
    color = settings.contains(lookupColor) ? settings.value(lookupColor).value<QColor>() : QColor(_colorDefault);
    updateColorButton();

    connect(tagType, SIGNAL(clicked()), this, SLOT(setColor()));
    _layout->addWidget(tagType, _row, 0, 1, 1);

    // font
    fontButton = new QPushButton(QApplication::font().toString());
    QString lookupFont = _name + "/font";
    font.fromString(settings.contains(lookupFont)?settings.value(lookupFont).toString():_fontDefault);
    updateFontButton();

    connect(fontButton, SIGNAL(clicked()), this, SLOT(setFont()));
    _layout->addWidget(fontButton, _row, 1, 1, 1);

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
        updateFontButton();
    }
}

void TagPreference::updateFontButton()
{
    QSettings settings;
    QString settingsKey = tagType->text() + "/font";

    settings.setValue(settingsKey, font.toString());
    fontButton->setText(font.toString());
}

void TagPreference::setColor()
{
    QColor tmpColor = QColorDialog::getColor(color);

    if (tmpColor.isValid())
    {
        color = tmpColor;
        updateColorButton();
    }
}

void TagPreference::updateColorButton()
{
    QSettings settings;
    QString settingsKey = tagType->text() + "/color";

    settings.setValue(settingsKey, color.name());

    QColor fgColor = (color.lightness() < 128) ? QColor(255, 255, 255) : QColor(0, 0, 0);

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
