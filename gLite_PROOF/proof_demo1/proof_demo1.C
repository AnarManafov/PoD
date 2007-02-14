
{
  //--------------------------------------------------------
  // connecting to gLite UI provider
  TGrid::Connect( "glite" );

  //--------------------------------------------------------
  // Submitting a job
  // which will configure environment and start proofd
  TGridJob *job = gGrid->Submit("/home/anar/svn/grid/D-Grid/gLite_PROOF/trunk/gLite_PROOF/proof.jdl");

  // Quering a status of the job
  TGridJobStatus *status = job->GetJobStatus();
  status->GetStatus();

  //-------------------------------
  // Checking file catalog files
  gGrid->Cd("/grid/dech/");
  TGridResult *all_res = gGrid->Ls();
  Int_t i = 0;
  while( all_res->GetFileName(i)) \
    cout << "File " << all_res->GetFileName(i++) << endl;

  //--------------------------------------------------------
  // A query for required "root" files
  TGridResult *res = gGrid->Query( "/grid/dech", ".*\\.root$" );

  // Creating a TDset of files to analyze
  TDSet *set;

  set = new TDSet("TTree","h42");

  for ( Int_t i = 0; res->GetKey( i, "sfn0" ); ++i )\
    set->Add( res->GetKey( i, "sfn0" ) );

  // Checking the TDSet
  set->GetListOfElements()->Print();

  // Quering a status of the job
  status->GetStatus();

  //--------------------------------------------------------
  // we will proceed further as soon as the status is kRUNNING
  // waiting for kRUNNING == status->GetStatus() ...
  //--------------------------------------------------------

  //--------------------------------------------------------
  // connecting to PROOF master
  // GLOBUS authentication is used -- pre-initialized by job's script
  TProof::Open("grid24.gsi.de:5151")

  // checking a status of the PROOF connection
  gProof->Print();

  //--------------------------------------------------------
  // Processing our test analysis
  set->Process("myselector.C");
}
