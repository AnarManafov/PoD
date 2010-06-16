/*
 *  worker.h
 *  pod-ssh
 *
 *  Created by Anar Manafov on 16.06.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
#include <string>

class CWorker
{
    public:
        CWorker( const std::string &_id,
                 const std::string &_addr,
                 const std::string &_sshOptions,
                 const std::string &_wrkDir,
                 size_t _nNumber );

        void printInfo( std::ostream &_stream ) const;
        std::string getID() const
        {
            return m_id;
        }

    private:
        std::string m_id;
        std::string m_addr;
        std::string m_sshOptions;
        std::string m_wrkDir;
};
