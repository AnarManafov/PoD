/************************************************************************/
/**
 * @file PROOFCfgImpl.h
 * @brief This files contains a class, which creates proof.conf for server and client
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-06-27
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFCFGIMP_H_
#define PROOFCFGIMP_H_

namespace PROOFAgent
{
    /**
     * @brief This class creates proof.conf for server and client
     * @note
     example of proof.conf for server
     @verbatim
      
     master depc218.gsi.de  workdir=~/proof
     worker manafov@localhost port=20001 perf=100 workdir=~/
      
     @endverbatim
     example of proof.conf for client
     @verbatim
      
     master lxial24.gsi.de
     worker lxial24.gsi.de perf=100
      
     @endverbatim
     **/
    template <class _T>
    struct CPROOFCfgImpl
    {
        void CreatePROOFCfg( const std::string &_PROOFCfg) const
        {
            std::ofstream f_out( _PROOFCfg.c_str() );
            // TODO: check file-errors
            const _T *pThis = reinterpret_cast<const _T*>( this );

            // getting local host name
            std::string host;
            MiscCommon::get_hostname( &host );
            // master host name is the same for Server and Worker and equal to local host name
            f_out << "#master " << host << std::endl;
            f_out << "master " << host << std::endl;

            if ( pThis->GetMode() == Client )
            {
                f_out << "worker " << host << " perf=100" << std::endl;
            }
        }
        void AddWrk2PROOFCfg( const std::string &_PROOFCfg, const std::string &_UsrName,
                              unsigned short _Port, const std::string &_RealWrkHost, std::string *_RetVal = NULL ) const
        {
            const _T * pThis = reinterpret_cast<const _T*>( this );
            if ( pThis->GetMode() != Server )
                return ;

            std::ofstream f_out( _PROOFCfg.c_str(), std::ios_base::out | std::ios_base::app );
            if ( !f_out.is_open() )
                throw std::runtime_error("Can't open the PROOF configuration file: " + _PROOFCfg );

            std::stringstream ss;
            ss << "#worker " << _UsrName << "@" << _RealWrkHost << " (redirect through localhost:" << _Port << ")";

            f_out << ss.str() << std::endl;
            f_out << "worker " << _UsrName << "@localhost port="  << _Port << " perf=100" << std::endl;
            if ( _RetVal )
                *_RetVal = ss.str();
        }
        void RemoveEntry( const std::string &_PROOFCfg, const std::string &_sPROOFCfgString ) const
        {
            // Read proof.conf in order to update it
            std::ifstream f( _PROOFCfg.c_str() );
            if ( !f.is_open() )
                return;
            
            MiscCommon::custom_istream_iterator<std::string> in_begin(f);
            MiscCommon::custom_istream_iterator<std::string> in_end;
            MiscCommon::StringVector_t vec( in_begin, in_end );
            
            f.close();
            std::ofstream f_out( _PROOFCfg.c_str() );
            
            MiscCommon::StringVector_t::const_iterator iter = vec.begin();
            MiscCommon::StringVector_t::const_iterator iter_end = vec.end();
            for ( ; iter != iter_end; ++iter )
            {
                if ( *iter == _sPROOFCfgString )
                {
                    ++iter;
                    continue;
                }
                f_out << *iter << std::endl;
            }
        }

    };

};

#endif /*PROOFCFGIMP_H_*/
