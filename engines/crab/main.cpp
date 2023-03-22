#include "app.h"
#include "stdafx.h"

int WinMain(int argc, char *args[]) {
	App app;

	if (app.Init())
		app.Run();

	return 0;
}