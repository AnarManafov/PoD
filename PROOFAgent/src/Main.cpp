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
    // TODO: Implement application's parameters check
    // "/home/anar/svn/grid/D-Grid/PROOFAgent/trunk/PROOFAgent/documentation/PROOFAgent_config/proofagent.cfg.xml"
    CPROOFAgent agent;
    cout << agent.Init( argv[1] ) << endl;

    return erOK;
}
