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

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMenu>
#include <QSettings>
#include "tagpreference.h"

#include <iostream>
using namespace std;

TagPreference::TagPreference(const QString& _name,
                             QWidget* _parent)
    : QLabel(_name, _parent),
    regexpChangeable(true)
{
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    initDefault();
}

TagPreference::TagPreference(const QString& _name,
                             const QString& _regexDefault,
                             const QString& _colorDefault,    // default, if not defined in settings
                             const QString& _fontDefault,     // default, if not defined in settings
                             const QColor& _bgcolor,
                             bool _visibility,
                             bool _regexpChangeable,
                             int _fold,
                             QWidget* _parent)
    : QLabel(_name, _parent),
    bgcolor(_bgcolor),
    regexpChangeable(_regexpChangeable)
{
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    initDefault(_regexDefault, _colorDefault, _fontDefault, _visibility, _fold);

    // tag and color
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    updateLabel();
}

void TagPreference::initDefault(const QString& _regexDefault,
                                const QString& _colorDefault,
                                const QString& _fontDefault,
                                bool _visibility,
                                int _fold)
{
    QSettings settings;
    QString lookup;

    lookup = "tagpref/" + text() + "/regExp";
    if (settings.contains(lookup))
    {
        regExpText = settings.value(lookup).toString();
    }
    else
    {
        regExpText = _regexDefault;
        settings.setValue(lookup, _regexDefault);
    }

    lookup = "tagpref/" + text() + "/color";
    if (settings.contains(lookup))
    {
        color = settings.contains(lookup) ? settings.value(lookup).value<QColor>() : QColor(_colorDefault);
    }
    else
    {
        color = QColor(_colorDefault);
        settings.setValue(lookup, _colorDefault);
    }

    lookup = "tagpref/" + text() + "/font";
    QString fontString = QFont().toString();

    if (settings.contains(lookup))
    {
        fontString = settings.value(lookup).toString();
    }
    else
    {
        if (!_fontDefault.isEmpty())
        {
            fontString = _fontDefault;
        }
        settings.setValue(lookup, fontString);
    }
    font.fromString(fontString);

    lookup = "tagpref/" + text() + "/visibility";
    if (settings.contains(lookup))
    {
        visibility = settings.value(lookup).toBool();
    }
    else
    {
        visibility = _visibility;
        settings.setValue(lookup, visibility);
    }

    lookup = "tagpref/" + text() + "/fold";
    if (settings.contains(lookup))
    {
        fold = settings.value(lookup).toInt();
    }
    else
    {
        fold = _fold;
        settings.setValue(lookup, fold);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    regExp = QRegularExpression(regExpText);
#else
    regExp = QRegExp(regExpText);
#endif
}

int TagPreference::getFold() const
{
    return fold;
}

bool TagPreference::getVisibility() const
{
    return visibility;
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

bool TagPreference::getChangeable() const
{
    return regexpChangeable;
}

void TagPreference::updateLabel()
{
    QSettings settings;
    QString lookup;

    lookup = "tagpref/" + text() + "/font";
    settings.setValue(lookup, font.toString());

    lookup = "tagpref/" + text() + "/color";
    settings.setValue(lookup, color.name());

    setFont(font);

    QString css = "background-color: " + bgcolor.name() + "; color: " + color.name() + ";";

    setStyleSheet(css);
}

void TagPreference::changeFont()
{
    QFontDialog dialog(font, NULL);

    if (dialog.exec())
    {
        font = dialog.selectedFont();
        updateLabel();
    }
}

void TagPreference::changeColor()
{
    QColor result = QColorDialog::getColor(color);

    if (result.isValid())
    {
        color = result;
        updateLabel();
    }
}

void TagPreference::setBackgroundColor(const QColor& _bgcolor)
{
    bgcolor = _bgcolor;
    updateLabel();
}

void TagPreference::onCustomContextMenu(const QPoint& /*_pos*/)
{
    QMenu* menu = new QMenu(this);

    QAction* act = NULL;

    act = new QAction("Visible", this);
    act->setCheckable(true);
    act->setChecked(visibility);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(changeVisibility(bool)));
    menu->addAction(act);

    if (fold != -1)
    {
        act = new QAction("Fold", this);
        act->setCheckable(true);
        act->setChecked(fold);
        connect(act, SIGNAL(triggered(bool)), this, SLOT(changeFold(bool)));
        menu->addAction(act);
    }

    menu->addSeparator();

    act = new QAction("Add New", this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(addTagPreference()));

    QMenu* cmenu = new QMenu("Change");

    act = new QAction("Color", this);

    cmenu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(changeColor()));

    act = new QAction("Font", this);
    cmenu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(changeFont()));

    if (regexpChangeable)
    {
        act = new QAction("RegExp", this);
        cmenu->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(changeRegularExpression()));
    }
    menu->addMenu(cmenu);
    if (regexpChangeable)
    {
        act = new QAction("Delete", this);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(deleteTagPreference()));
    }

        act = new QAction("Up", this);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(moveUp()));
        act = new QAction("Down", this);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(moveDown()));

    menu->exec(QCursor::pos());
}

void TagPreference::moveUp()
{
  emit moveUp(this);
}

void TagPreference::moveDown()
{
  emit moveDown(this);
}

void TagPreference::changeRegularExpression()
{
    bool ok;
    QString tmpcss = styleSheet();

    setStyleSheet("");
    QString result = QInputDialog::getText(this, tr("Change Regular Expression"),
                                           tr("Regular Expression:"), QLineEdit::Normal,
                                           regExpText, &ok);

    setStyleSheet(tmpcss);

    if (ok && !result.isEmpty())
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        bool valid = QRegularExpression(result).isValid();
#else
        bool valid = QRegExp(result).isValid();
#endif

        if (valid)
        {
            regExpText = result;
            QSettings settings;
            QString lookup = "tagpref/" + text() + "/regExp";
            settings.setValue(lookup, regExpText);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            regExp = QRegularExpression(regExpText);
#else
            regExp = QRegExp(regExpText);
#endif
            emit regexpChanged(text());
        }
    }
}

void TagPreference::changeVisibility(bool _val)
{
    visibility = _val;
    QSettings settings;
    QString lookup = "tagpref/" + text() + "/visiblity";

    settings.setValue(lookup, _val);

    emit visibilityChanged(text());
}

void TagPreference::changeFold(bool _val)
{
    if (fold != -1)
    {
        fold = _val ? 1 : 0;
        QSettings settings;
        QString lookup = "tagpref/" + text() + "/fold";

        settings.setValue(lookup, _val);

        emit foldChanged(text());
    }
}

void TagPreference::addTagPreference()
{
    bool ok;
    QString tmpcss = styleSheet();

    setStyleSheet("");
    QString result = QInputDialog::getText(this, tr("Add new tag pattern"),
                                           tr("Label :"), QLineEdit::Normal,
                                           QString("name"), &ok);

    setStyleSheet(tmpcss);
    if (ok && !result.isEmpty())
    {
        emit addTagPreference(result);
    }
}

void TagPreference::deleteTagPreference()
{
    // delete from tagpref list
    emit deleteTagPreference(text());

    // delete from QSettings
    QSettings settings;
    QString lookup = "tagpref/" + text();

    settings.remove(lookup);
}
