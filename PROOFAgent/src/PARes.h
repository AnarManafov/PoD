/************************************************************************/
/**
 * @file Agent.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:           Anar Manafov
                                   2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PARES_H_
#define PARES_H_

// OUR
#include "def.h"

/**
 * @brief A general name space for PROOFAgent application 
 **/
namespace PROOFAgent
{

    const MiscCommon::LPCTSTR g_szPROTOCOL_VERSION = _T("PAprotocol:0.1.0");
    const MiscCommon::LPCTSTR g_szRESPONSE_OK = _T("PA_OK");
    const MiscCommon::LPCTSTR g_szSEND_USERNAME = _T("PA_CLT_USER:");

    const size_t g_nBUF_SIZE = 1024;

};

#endif /*PARES_H_*/
