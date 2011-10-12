//
//  Utils.h
//  PoD
//
//  Created by Anar Manafov on 11.10.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef UTILS_H
#define UTILS_H

#include "version.h"

// STD
#include <string>

// MiscCommon
#include "Process.h"

template<class _T>
void kill_process( const std::string &_pidFileName, _T &_logFun )
{
    // TODO: make wait for the process here to check for errors
    const pid_t pid_to_kill = MiscCommon::CPIDFile::GetPIDFromFile( _pidFileName );
    if( pid_to_kill > 0 && MiscCommon::IsProcessExist( pid_to_kill ) )
    {
        std::stringstream ss;
        ss
                << PROJECT_NAME << ": stopping ("
                << pid_to_kill
                << ")...";
        _logFun( ss.str() );

        // TODO: Maybe we need more validations of the process before
        // sending a signal. We don't want to kill someone else.
        kill( pid_to_kill, SIGTERM );

        // Waiting for the process to finish
        size_t iter( 0 );
        const size_t max_iter = 30;
        while( iter <= max_iter )
        {
            if( !MiscCommon::IsProcessExist( pid_to_kill ) )
            {
                std::cout << std::endl;
                break;
            }
            sleep( 1 ); // sleeping for 1 second
            ++iter;
        }
        if( MiscCommon::IsProcessExist( pid_to_kill ) )
            throw std::runtime_error( "FAILED to close the process." );
    }

}

#endif
