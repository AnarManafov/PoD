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
namespace PROOFAgent
{
    class CProofStatusFile;
    class CTest_CProofStatusFile
    {
        public:
            CTest_CProofStatusFile();

            bool getAdminPath( const std::string &_xpdCFGFileName,
                               boost::filesystem::path *_ret,
                               EAdminPathType _type );
    };
//=============================================================================
    class CProofStatusFile
    {
            friend class CTest_CProofStatusFile;

        public:
            CProofStatusFile();
            virtual ~CProofStatusFile();

        private:
            bool getAdminPath( const std::string &_xpdCFGFileName,
                               EAdminPathType _type );

        private:
            boost::filesystem::path m_adminPath;
    };

}

#endif /* PROOFSTATUSFILE_H_ */
