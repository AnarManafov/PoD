/************************************************************************/
/**
 * @file ServerInfo.h
 * @brief Interface
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-05-28
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
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
        pid_t IsXROOTDRunning();
        pid_t IsPROOFAgentRunning();
        std::string GetXROOTDInfo();
        std::string GetPAInfo();

    private:
        pid_t IsRunning( const std::string &_Srv );
        void GetPROOFAgentVersion( std::string *_Ver );
};

#endif /*SERVERINFO_H_*/
