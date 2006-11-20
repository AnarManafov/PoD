/************************************************************************/
/*! \file submit_proofd.C
	RGlite test macro - Job submission *//*

         version number:    $LastChangedRevision$
         created by:        Anar Manafov
                            2006-11-13
         last changed by:   $LastChangedBy$ $LastChangedDate$

         Copyright (c) 2006 GSI GridTeam. All rights reserved.
*************************************************************************/
//
// Example of usage:
// root [1] .x tests/job_submission.C("/home/anar/jdl/test.jdl")
//

// STD
#include <iostream>
// RGlite
#include "TGridJobStatus.h"

using namespace std;

const size_t MAX_ITER_COUNT = 100;
const size_t SLEEP_TIME = 20000;// in milliseconds

Bool_t submit_proofd( const char * _JDL )
{
  TGrid::Connect("glite");

  TGridJob *job = gGrid->Submit( _JDL );

  TGridJobStatus *status = job->GetJobStatus();
  for( size_t i = 0; i < MAX_ITER_COUNT; ++i )
  {
	cout << " - - - - - - - - - - - - - " << endl;
	cout << "Iteration #" << i << endl;
	switch ( status->GetStatus() )
	{
	  case TGridJobStatus::kDONE:
	  case TGridJobStatus::kRUNNING:
		cout << "Connecting to PROOF master..." << endl;
		gROOT->Proof("proof://grid24.gsi.de:5151");
		cout << "Checking status..." << endl;
		gProof->Print();
		return kTRUE;
	  case TGridJobStatus::kABORTED:
	  case TGridJobStatus::kFAIL:
		return kFALSE;
	  default:
		gSystem->Sleep(SLEEP_TIME );
	}
  }
  return kFALSE;
}
