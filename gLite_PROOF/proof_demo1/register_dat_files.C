{
	TGrid::Connect( "glite" );

	if ( !gGrid )
		return kFALSE;

	if ( !gGrid->Cd( "/grid/dech" ) )
		return kFALSE;

	const string newfile( "dstarmb.root" );
	const TUUID uuid;
	const string strGUID( uuid.AsString() );
	const string strURL( "root://grid24.gsi.de:5152//home/dech001/" + newfile );
	const string strServer( "grid24.gsi.de" );

	return gGrid->Register( newfile.c_str(), strURL.c_str(), 0, strServer.c_str(), strGUID.c_str() );
}