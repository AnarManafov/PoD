/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/

#include "PROOFAgent.h"

using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;

int main( int argc, char *argv[] )
{
    CPROOFAgent agent;
    cout << agent.Init( "/home/anar/svn/grid/D-Grid/PROOFAgent/trunk/PROOFAgent/documentation/PROOFAgent_config/proofagent.cfg.xml" ) << endl;

    return erOK;
}
