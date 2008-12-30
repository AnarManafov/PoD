/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-12-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
// Misc
#include "def.h"
// LSF plug-in
#include "LsfMng.h"

using namespace std;
using namespace MiscCommon;

const LPCTSTR g_szAppName = "PoD LSF plug-in";

CLsfMng::CLsfMng()
{

}

CLsfMng::~CLsfMng()
{

}

void CLsfMng::init()
{
    // initialize LSBLIB  and  get  the  configuration environment
    // FIX: for some reason lsb_init requares char * insted of const char *. This needs to be investigated
    if ( lsb_init( const_cast<char*>( g_szAppName ) ) < 0 )
        throw runtime_error( "Can't initialize LSF." ); // TODO: get error description here (get it from LSF, lsberrno)
}
