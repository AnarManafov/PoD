/************************************************************************/
/**
 * @file ServerInfo.h
 * @brief Interface
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

 version number:     $LastChangedRevision$
 created by:         Anar Manafov
 2007-05-28
 last changed by:    $LastChangedBy$ $LastChangedDate$

 Copyright (c) 2007 GSI GridTeam. All rights reserved.
 *************************************************************************/
#ifndef SERVERINFO_H_
#define SERVERINFO_H_
//=============================================================================
// STD
#include <string>
//=============================================================================
class CServerInfo
{
    public:
        CServerInfo()
        {}
        virtual ~CServerInfo()
        {}

    public:
        bool IsRunning( bool _check_all ) const;
        pid_t IsXROOTDRunning() const;
        pid_t IsPROOFAgentRunning() const;
        std::string GetXROOTDInfo() const;
        std::string GetPAInfo() const;

    private:
        pid_t _IsRunning( const std::string &_Srv ) const;
        void GetPROOFAgentVersion( std::string *_Ver ) const;
};

#endif /*SERVERINFO_H_*/
