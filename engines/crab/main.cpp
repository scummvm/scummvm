#include "stdafx.h"
#include "app.h"

int WinMain( int argc, char* args[] )
{
	App app;

	if(app.Init())
		app.Run();

	return 0;
}