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

#include <QtGui>
#include <QSettings>

#include "mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/images/gvtree_logo_128.png"));

    app.setOrganizationName("gvtree");
    app.setOrganizationDomain("gvtree");
    app.setApplicationName("gvtree");

    // qsettings
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings;

    // will be overwritten by -s
    bool styleSheet = false;
    if (settings.contains("styleSheetFile"))
    {
        QFile styleFile(settings.value("styleSheetFile").toString());
        if (styleFile.open(QFile::ReadOnly))
        {
            QString style(styleFile.readAll());
            app.setStyleSheet(style);
            styleFile.close();
            styleSheet = true;
        }
    }

    if (styleSheet == false)
    {
        QFile styleFile(":/css/gvtree.css");
        if (styleFile.open(QFile::ReadOnly))
        {
            QString style(styleFile.readAll());
            app.setStyleSheet(style);
        }
    }

#if QT_VERSION < 0x050000
    // Codec for git log output
    if (settings.contains("codecForCStrings"))
    {
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName(settings.value("codecForCStrings").toString().toUtf8().data()));
    }
    else
    {
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    }
#endif

    // main window
    MainWindow* win = new MainWindow(QCoreApplication::arguments());
    win->show(); // done by constructor

    return app.exec();
}
