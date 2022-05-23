/*
 * CommandCodes.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: David Wang
 */


#include <Arduino.h>

#include "CommandCodes.h"


#define etype(x,y) y
static const char *strCommandCodes[] = { COMMANDCODES };
#undef etype


const char *enum2str (CommandCodes f)
{
    return strCommandCodes[static_cast<int>(f)];
}

const char *enum2str (COMM_CODE_T f)
{
    return strCommandCodes[f];
}


CommandCodes str2enum (const char *f)
{
    const int n = sizeof(strCommandCodes) / sizeof(strCommandCodes[0]);
    for (int i = 0; i < n; ++i)
    {
        if (strcmp(strCommandCodes[i], f) == 0)
            return (CommandCodes) i;
    }
    return CC_NONE;
}
