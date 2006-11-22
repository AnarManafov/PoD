Bool_t RegisterFile( const string _FileName )
{
	if ( !gGrid->Cd( "/grid/dech" ) )
		return kFALSE;

	const TUUID uuid;
	const string strGUID( uuid.AsString() );
	const string strURL( "root://grid24.gsi.de:5152//home/dech001/" + _FileName );
	const string strServer( "grid24.gsi.de" );

        gGrid->Register( _FileName.c_str(), strURL.c_str(), 0, strServer.c_str(), strGUID.c_str() );
        
}
Bool_t register_dat_files()
{
  TGrid::Connect( "glite" );
  if ( !gGrid )
	return kFALSE;

  RegisterFile("dstarmb.root");
  RegisterFile("dstarp1a.root");
  return kTRUE;
}
