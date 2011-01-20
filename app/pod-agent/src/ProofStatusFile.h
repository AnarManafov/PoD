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

        Copyright (c) 2010-2011 GSI, Scientific Computing devision. All rights reserved.
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
//The status of a proofserv session is updated periodically in the file 'activesessions' area.
//  There is a new file there
//
//   <admin_path>/.xproofd.<port>/activesessions/<user>.<group>.<pid>.status
//
//  which contains a integer with the possible values
//
//               0              idle
//               1              running
//               2              terminated (not implemented)
//               3              enqueued
//
//  The update period is controlled by
//
//             xpd.proofservmgr checkfq:secs
//
//  the default being 30 s .
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
enum EProofStatus
{
    proofstatus_idle = 0,
    proofstartus_running = 1,
    proofstatus_terminated = 2,
    proofstatus_enqueued = 3
};
//=============================================================================
typedef std::vector<boost::filesystem::path> PathVector_t;
typedef std::vector<EProofStatus> ProofStatusContainer_t;
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
            void enumStatusFiles();
            boost::filesystem::path getAdminPath() const
            {
                return m_adminPath;
            }
            PathVector_t getFiles() const
            {
                return m_files;
            }
            ProofStatusContainer_t getStatus() const
            {
                return m_status;
            }
            uint16_t xpdPort() const
            {
                return m_xpdPort;
            }
            pid_t xpdPid() const
            {
                return m_xpdPid;
            }

        private:
            uint16_t m_xpdPort;
            pid_t m_xpdPid;
            boost::filesystem::path m_adminPath;
            ProofStatusContainer_t m_status;
            PathVector_t m_files;
    };

}

#endif /* PROOFSTATUSFILE_H_ */
