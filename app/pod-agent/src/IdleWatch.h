/************************************************************************/
/**
 * @file IdleWatch.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef IDLEWATCH_H_
#define IDLEWATCH_H_

namespace PROOFAgent
{

    class CIdleWatch
    {
        public:
            CIdleWatch()
            {
                touch();
            }
            void touch()
            {
                m_startTime = time( NULL );
            }
            bool isTimedout( int _numSeconds )
            {
                if ( _numSeconds <= 0 )
                    return false;

                time_t curTime = time( NULL );
                return (( curTime - m_startTime ) >= _numSeconds );
            }

        private:
            time_t m_startTime;
    };

}

#endif /* IDLEWATCH_H_ */
