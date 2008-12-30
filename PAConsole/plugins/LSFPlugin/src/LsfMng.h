/************************************************************************/
/**
 * @file $$File name$$
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

class CLsfMng
{
    public:
        CLsfMng();
        virtual ~CLsfMng();

    public:
        void init();
        void jobSubmit();

    private:
    	submit m_submitRequest;
};

#endif /* LSFMNG_H_ */
