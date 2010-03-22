/************************************************************************/
/**
 * @file PbsMng.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-22
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PBSMNG_H_
#define PBSMNG_H_
// STD
#include <string>


class CPbsMng
{
public:
    typedef std::string jobID_t;
    
public:
    bool isValid( const jobID_t &_id );
    jobID_t jobSubmit( const std::string &_cmd );
};

#endif /* PBSMNG_H_ */
