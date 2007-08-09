/************************************************************************/
/**
 * @file ServerInfo.h
 * @brief Interface
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision: 840 $
        created by:         Anar Manafov
                            2007-05-28
        last changed by:    $LastChangedBy: manafov $ $LastChangedDate: 2007-05-29 18:28:05 +0200 (Tue, 29 May 2007) $

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef SERVERINFO_H_
#define SERVERINFO_H_

// STD
#include <string>

class CServerInfo
{
    public:
        CServerInfo()
        {}
        virtual ~CServerInfo()
        {}

    public:
        pid_t IsXROOTDRunning() const;
        pid_t IsPROOFAgentRunning() const;
        std::string GetXROOTDInfo() const;
        std::string GetPAInfo() const;

    private:
        pid_t IsRunning( const std::string &_Srv ) const;
        void GetPROOFAgentVersion( std::string *_Ver ) const;
};

#endif /*SERVERINFO_H_*/
