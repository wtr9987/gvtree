/* --------------------------------------------- */
/*                                               */
/*   Copyright (C) 2021 Wolfgang Trummer         */
/*   Contact: wolfgang.trummer@t-online.de       */
/*                                               */
/*                  gvtree V1.5-0                */
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

#include <stdio.h>
#include <iostream>

#include "execute_cmd.h"

void execute_cmd(const char* _cmd, QList<QString>& _output, bool _log)
{
    char buffer[65536];

    if (_log)
        std::cout << _cmd << std::endl;

    FILE* pipe = popen(_cmd, "r");
    if (pipe)
    {
        while (fgets(buffer, 65536, pipe) != NULL)
        {
            _output.push_back(QString(buffer));
        }
        pclose(pipe);
    }
}
