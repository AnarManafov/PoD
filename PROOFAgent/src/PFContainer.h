/************************************************************************/
/**
 * @file PFContainer.h
 * @brief 
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-06-27
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PFCONTAINER_H_
#define PFCONTAINER_H_

namespace PROOFAgent
{
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

    typedef boost::shared_ptr<SUpdatePROOFCfg> SUpdatePROOFCfgPtr;

    template <class _T>
    struct SDelete: public std::binary_function<_T, SUpdatePROOFCfgPtr, bool>
    {
        bool operator() ( _T _val ) const
        {
            if ( !_val.first )
                return true;

            delete _val.first;
            _val.first = NULL;
            return true;
        }
        bool operator() ( _T _val, SUpdatePROOFCfgPtr _Updater ) const
        {
            if ( !_val.first )
                return true;

            if ( !_val.first->IsValid() )
            {
                delete _val.first;
                _Updater->_RemoveEntry( _val.second );
                _val.first = NULL;
            }

            return true;
        }
    };

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
            }

        private:
            pf_container_type m_container;

    };

};

#endif /*PFCONTAINER_H_*/
