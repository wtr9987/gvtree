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
    changeable(true),
    visibility(true),
    fold(0)
{
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    path = QString("tagpref/" + _name + "/");
}

TagPreference::TagPreference(const QString& _name,
                             const QString& _regex,
                             const QString& _color,    // default, if not defined in settings
                             const QString& _font,     // default, if not defined in settings
                             const QColor& _bgcolor,
                             bool _visibility,
                             bool _changeable,
                             int _fold,
                             QWidget* _parent)
    : QLabel(_name, _parent),
    regExpText(_regex),
    bgcolor(_bgcolor),
    changeable(_changeable),
    visibility(_visibility),
    fold(_fold)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    regExp = QRegularExpression(_regex);
#else
    regExp = QRegExp(_regex);
#endif

    font.fromString(_font);
    color = QColor(_color);

    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // tag and color
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onCustomContextMenu(const QPoint&)));

    QSettings settings;

    path = QString("tagpref/" + _name + "/");
    settings.setValue(path + "regExp", _regex);
    settings.setValue(path + "font", _font);
    settings.setValue(path + "color", _color);
    settings.setValue(path + "visibility", visibility);
    settings.setValue(path + "changeable", changeable);
    settings.setValue(path + "fold", fold);

    updateLabel();
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
    return changeable;
}

void TagPreference::updateLabel()
{
    QSettings settings;

    settings.setValue(path + "font", font.toString());
    settings.setValue(path + "color", color.name());

    setFont(font);

    QString css = "background-color: " + bgcolor.name() + "; color: " + color.name() + ";";

    setStyleSheet(css);

    emit visibilityChanged(text());
}

void TagPreference::changeFont()
{
    QFontDialog dialog(font);

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

    if (changeable)
    {
        act = new QAction("RegExp", this);
        cmenu->addAction(act);
        connect(act, SIGNAL(triggered()), this, SLOT(changeRegularExpression()));
    }
    menu->addMenu(cmenu);
    if (changeable)
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
            settings.setValue(path + "regExp", regExpText);

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

    settings.setValue(path + "visibility", _val);

    emit visibilityChanged(text());
}

void TagPreference::changeFold(bool _val)
{
    if (fold != -1)
    {
        fold = _val ? 1 : 0;
        QSettings settings;

        settings.setValue(path + "fold", _val);

        emit visibilityChanged(text());
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

    settings.remove(path);
}
