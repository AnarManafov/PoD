/************************************************************************/
/**
 * @file proof_demo1.C
 * @brief Simple test of gLitePROOF #1
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-05-22
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/


{
  // Creating a TDset of files to analyze
  TDSet *set = new TDSet( "TTree","h42" );
  
  // Files to analyze 
  set->Add( "root://depc218.gsi.de:1094//tmp/dstarmb.root" );
  set->Add( "root://depc218.gsi.de:1094//tmp/dstarmb2.root" );
  
  // connecting to local PROOF server
  proof = TProof::Open( "localhost" );
  
  // Processing our test analysis
  set->Process( "myselector.C" );
}
