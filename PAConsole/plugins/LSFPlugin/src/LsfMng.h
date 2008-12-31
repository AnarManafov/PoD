/************************************************************************/
/**
 * @file LsfMng.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-12-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef LSFMNG_H_
#define LSFMNG_H_

// LSF API
#include <lsf/lsbatch.h>
// STD
#include <string>
#include <map>

class CLsfMng
{
	public:
		typedef enum EJobProperty
		{
			JP_SUB_JOB_NAME,		//!< job name specified
			JP_SUB_QUEUE,			//!< queue specified
			JP_SUB_HOST,			//!< execution host(s) specified
			JP_SUB_IN_FILE,			//!< input file specified
			JP_SUB_OUT_FILE,		//!< output file specified
			JP_SUB_ERR_FILE			//!< error file specified
		}EJobProperty_t;

		typedef std::map<EJobProperty_t, std::string> propertyDict_t;

    public:
        CLsfMng();
        virtual ~CLsfMng();

    public:
        void init();
        void addProperty( EJobProperty_t _type, const std::string &_val);
        // TODO: implement
        //void removeProperty();
		int jobSubmit();

    private:
    	propertyDict_t m_submitRequest;
};

#endif /* LSFMNG_H_ */
