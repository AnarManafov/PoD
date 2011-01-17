//
//  Environment.h
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
//=============================================================================
#include <string>
//=============================================================================
class CEnvironment
{
    public:
        CEnvironment();

    public:
        void init();
        std::string version() const
        {
            return m_localVer;
        }
        std::string PoDPath() const
        {
            return m_PoDPath;
        }
        bool isLocalServer() const
        {
            return m_isLocalServer;
        }

    private:
        void getLocalVersion();
        bool checkForLocalServer();

    private:
        std::string m_PoDPath;
        std::string m_localVer;
        bool m_isLocalServer;
};

#endif
