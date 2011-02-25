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
        CSSHTunnel(): m_pid( 0 )
        {
        }
        ~CSSHTunnel();
    public:
        void setPidFile( const std::string &_filename );
        void create( const CEnvironment &_env, const SOptions &_opt );
        pid_t pid();

    private:
        void killTunnel();

    private:
        std::string m_pidFile;
        pid_t m_pid;
};
//=============================================================================
#endif
