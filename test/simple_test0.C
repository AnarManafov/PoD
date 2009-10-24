/************************************************************************/
/**
 * @file simple_test0.C
 * @brief Simple test of gLitePROOF #1
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-05-22
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "../etc/pod-master.h"

simple_test0()
{
  // Creating a TDset of files to analyze
  TDSet *set = new TDSet( "TTree","h42" );
  
  // Files to analyze 
  set->Add( Form("root://%s:%s/%s/test/dstarmb.root", POD_MASTER_HOST, POD_XROOTD_PORT, POD_LOCATION) );
  set->Add( Form("root://%s:%s/%s/test/dstarmb1.root", POD_MASTER_HOST, POD_XROOTD_PORT, POD_LOCATION) );

  // connecting to local PROOF server
  proof = TProof::Open( Form("%s:%s", POD_MASTER_HOST, POD_XPROOF_PORT) );

  proof->SetParameter( "PROOF_MaxSlavesPerNode", (Long_t)1000 );  

  // Processing our test analysis
  set->Process( "./myselector.C" );
}
