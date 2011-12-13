/************************************************************************/
/**
 * @file PARes.h
 * @brief Resource file of the PROOFAgent project.
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PARES_H_
#define PARES_H_

// OUR
#include "def.h"

namespace PROOFAgent
{
    const size_t g_nBUF_SIZE = 1024;

    typedef enum { Unknown, Server, Client } EAgentMode_t;

    typedef enum { exitCode_OK = 0,
                   exitCode_GENERAL_ERROR = 1,
                   exitCode_CANT_FIND_XPROOFD = 400
                 } EExitCodes_t;

};

#endif /*PARES_H_*/
