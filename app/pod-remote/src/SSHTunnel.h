//
//  SSHTunnel.h
//  PoD
//
//  Created by Anar Manafov on 16.02.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef SSHTUNNEL_H
#define SSHTUNNEL_H
//=============================================================================
// API
#include <unistd.h>
// STD
#include <string>
// PoD
#include "Environment.h"
#include "Options.h"
//=============================================================================
class CSSHTunnel
{
    public:
        CSSHTunnel(): m_pid( 0 ), m_needToKill( true )
        {
        }
        ~CSSHTunnel();
    public:
        void setPidFile( const std::string &_filename );
        void create( const std::string &_connectionStr,
                     size_t _port, const std::string &_openDomain );
        pid_t pid();
        // the tunnel will be not closed when object is deleted
        void deattach()
        {
            m_needToKill = false;
        }

    private:
        void killTunnel();

    private:
        std::string m_pidFile;
        pid_t m_pid;
        bool m_needToKill;
};
//=============================================================================
#endif
