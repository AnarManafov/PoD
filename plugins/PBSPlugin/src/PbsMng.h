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

struct attrl;

class CPbsMng
{
    public:
        typedef std::string jobID_t;

    public:
        bool isValid( const jobID_t &_id ) const;
        jobID_t jobSubmit( const std::string &_script, const std::string &_queue,
                           size_t _nJobs,
                           const std::string &_outputPath ) const;
        void jobStatus( const jobID_t &_id );

    private:
        void cleanAttr( attrl **attrib ) const;
        void setDefaultPoDAttr( attrl **attrib, const std::string &_queue,
                                size_t _nJobs,
                                const std::string &_outputPath ) const;
};

#endif /* PBSMNG_H_ */
