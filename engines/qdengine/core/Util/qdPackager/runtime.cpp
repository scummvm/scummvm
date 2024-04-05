/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_core.h"
#include "app_error_handler.h"
#include "qd_game_dispatcher.h"
#include "qd_game_scene.h"

#include "infozip_api.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine, int nCmdShow)
{
	infozip_api::init();

	infozip_api::add_to_zip("test.pak","zip\\api.hh");

	return 0;
}
