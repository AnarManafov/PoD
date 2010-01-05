/************************************************************************/
/**
 * @file ProofStatusFile.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-01-05
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFSTATUSFILE_H_
#define PROOFSTATUSFILE_H_
//=============================================================================
// STD
#include <string>
#include <vector>
// BOOST
#include <boost/filesystem/path.hpp>
//=============================================================================
enum EAdminPathType
{
    adminp_server = 0,
    adminp_worker = 1
};
const char *const mark[] =
{
    "# server.adminpath",
    "# worker.adminpath"
};
//=============================================================================
typedef std::vector<boost::filesystem::path> PathVector_t;
//=============================================================================
namespace PROOFAgent
{

    class CProofStatusFile
    {
        public:
            CProofStatusFile();
            virtual ~CProofStatusFile();

        public:
            bool readAdminPath( const std::string &_xpdCFGFileName,
                                EAdminPathType _type );
            void enumStatusFiles( uint16_t _xpdPort );
            boost::filesystem::path getAdminPath()
            {
                return m_adminPath;
            }
            PathVector_t getFiles()
            {
                return m_files;
            }


        private:
            boost::filesystem::path m_adminPath;
            PathVector_t m_files;
    };

}

#endif /* PROOFSTATUSFILE_H_ */
