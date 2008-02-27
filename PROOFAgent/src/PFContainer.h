/************************************************************************/
/**
 * @file PFContainer.h
 * @brief A container implemenetation, which holds objects of CPacketForwarder
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-06-27
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PFCONTAINER_H_
#define PFCONTAINER_H_

namespace PROOFAgent
{
    /**
     *
     * @brief A helper class, which helps to remove entries from the proof configuration file.
     *
     */
    struct SUpdatePROOFCfg: protected CPROOFCfgImpl<SUpdatePROOFCfg>
    {
        SUpdatePROOFCfg(const std::string &_sPROOFCfg): m_sPROOFCfg(_sPROOFCfg)
        {}
        void _RemoveEntry( const std::string &_sPROOFCfgString ) const
        {
            RemoveEntry( m_sPROOFCfg, _sPROOFCfgString );
        }
        const std::string m_sPROOFCfg;
    };
    /**
     *
     * @brief A shared pointer wrapper for the SUpdatePROOFCfg class.
     *
     */
    typedef boost::shared_ptr<SUpdatePROOFCfg> SUpdatePROOFCfgPtr;
    /**
     *
     * @brief A functor, which helps to delete entries from the internal PF container.
     *
     */
    template <class _T>
    struct SDelete: public std::binary_function<_T, SUpdatePROOFCfgPtr, bool>
    {
        bool operator() ( _T &_val ) const
        {
            if ( !_val.first )
                return true;

            delete _val.first;
            _val.first = NULL;
            return true;
        }
        bool operator() ( _T &_val, const SUpdatePROOFCfgPtr &_Updater ) const
        {
            if ( !_val.first )
                return true;

            if ( !_val.first->IsValid() )
            {
                _Updater->_RemoveEntry( _val.second );
                delete _val.first;
                _val.first = NULL;
            }

            return true;
        }
    };
    /**
     *
     * @brief A functor, which checks whether the first value of a pair is NULL.
     *
     */
    template <class _T>
    struct SIsNull1st: public std::unary_function<_T, bool>
    {
        bool operator() ( const _T &_val ) const
        {
            return ( !_val.first );
        }
    };
    /**
     *
     * @brief A container implemenetation, which holds objects of CPacketForwarder
     *
     */
    class CPFContainer
    {
            typedef CPacketForwarder pf_container_value;
            typedef std::pair<pf_container_value *, std::string> container_value;
            typedef std::list<container_value> pf_container_type;

        public:
            CPFContainer()
            {}
            ~CPFContainer()
            {
                std::for_each( m_container.begin(), m_container.end(), SDelete<container_value>() );
            }
            void add( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort, const std::string &_sPROOFCfgString )
            {
                CPacketForwarder * pf = new CPacketForwarder( _ClientSocket, _nNewLocalPort );
                pf->Start();
                m_container.push_back( std::make_pair(pf, _sPROOFCfgString) );
            }
            void clean_disconnects( const std::string &_sPROOFCfg )
            {
                SUpdatePROOFCfgPtr updater = SUpdatePROOFCfgPtr( new SUpdatePROOFCfg(_sPROOFCfg) );
                std::for_each( m_container.begin(), m_container.end(), std::bind2nd(SDelete<container_value>(), updater) );
                m_container.erase( std::remove_if(m_container.begin(), m_container.end(), SIsNull1st<container_value>()), m_container.end() );
            }

        private:
            pf_container_type m_container;
    };

};

#endif /*PFCONTAINER_H_*/
