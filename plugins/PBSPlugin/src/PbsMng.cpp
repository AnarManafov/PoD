/************************************************************************/
/**
 * @file PbsMng.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-22
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// PBS plug-in
#include "PbsMng.h"
//=============================================================================
bool CPbsMng::isValid( const CPbsMng::jobID_t &_id )
{
    return !_id.empty();
}
//=============================================================================
CPbsMng::jobID_t CPbsMng::jobSubmit( const std::string &_cmd )
{
    return jobID_t();
}