
#include "common/stream.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/mem_management.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/classes/do_keyboard.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/classes/do_sound.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/define.h"
#include "watchmaker/globvar.h"
#include "watchmaker/game.h"
#include "watchmaker/game_options.h"
#include "watchmaker/init/nl_init.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_ffile.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/ll/ll_regen.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/message.h"
#include "watchmaker/renderer.h"
#include "watchmaker/saveload.h"
#include "watchmaker/schedule.h"
#include "watchmaker/types.h"
#include "watchmaker/utils.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/walk/ball.h"
#include "watchmaker/work_dirs.h"

#define T3D_NAMELEN                         32          // default name len

namespace Watchmaker {

// TODO:
char ExternalText[256] = "";

uint8 tasti_per_sfx1;
uint8 generate_data_and_exit = 0;
uint8 force_debug_window = 0;

void CleanUpAndPostQuit(WorkDirs &workDirs, GameOptions &gameOptions);

int WMakerMain() {
	warning("The Watchmaker");

	//CreateWindow()
	//int loaderFlags = ParseCommandLine()

	WGame *game = new WGame;

	if (!LoadExternalText(&game->init, ExternalText)) {                        // Legge file di external text
		warning("ExternalText error!");
		return 0;
	}
	if ((LoaderFlags & T3D_FASTFILE) && (t3dFastFileInit(WmGameDataPak_FilePath))) {        // Apre il filone
		//Fixup solo di quelle directory presenti nel ffile
		// Basically drop the WmGameDir prefix from the paths.
		assert(false);
		/*
		FixupPath(WmT3dDir, WmGameDir);
		FixupPath(WmBndDir, WmGameDir);
		FixupPath(WmCamDir, WmGameDir);
		FixupPath(WmMapsDir, WmGameDir);
		FixupPath(WmLightmapsDir, WmGameDir);
		FixupPath(WmMiscDir, WmGameDir);
		FixupPath(WmA3dDir, WmGameDir);
		FixupPath(WmWavDir, WmGameDir);
		 */
	} else {
		LoaderFlags &= ~T3D_FASTFILE;
		//rSetLoaderFlags(LoaderFlags);
	}

	game->initCharNames();

	// Setup sound/kbd/mouse
	if (!(LoaderFlags & T3D_NOSOUND) && !(InitMusic()))                // Attiva DirectSound
		warning("No Sound-Card detected");

	//  aggiorna i volumi
	/*
	sSetAllSoundsVolume(GameOptions.sound_volume);
	sSetAllSpeechVolume(GameOptions.speech_volume);
	mSetAllVolume(GameOptions.music_volume);


	if (LoaderFlags & T3D_FULLSCREEN) rSetFlagsFullScreen();             // Modalita' FullScreen
	if (LoaderFlags & T3D_NOLIGHTMAPS) rSetLightmapRendering(0);         // Disattiva le Lightmaps
	else rSetLightmapRendering(1);                                      // Attiva le Lightmaps
	*/
	warning("init engine...");
	if (!(rInitialize3DEnvironment(*game,/*hWnd, */ WmCfgName))) {               // Apre scheda 3D
		game->CleanUpAndPostQuit();
		return 1;
	} /*else
        g_bReady = true;*/

	t3dAllocMemoryPool(1000000);                                // Alloca memory-pool
	game->_messageSystem.init();

	if (LoaderFlags & T3D_DEBUGMODE) {
		if (!game->StartPlayingGame("")) {
			return 1;
		}
	} else {
		if (!game->LoadAndSetup("r97.t3d", 1)) {                       // Carica T3D e Bitmap
			return 1;
		}

		_vm->_messageSystem.doEvent(EventClass::MC_DIALOG, ME_DIALOGSTART, MP_DEFAULT, dRLOGHI, 0, 0, nullptr, nullptr, nullptr);
	}
#if 0
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
#endif

	game->GameLoop();

	warning("FIN");
	delete game;
	return 0;
}

/* -----------------16/10/98 12.07-------------------
 *                  CloseSys
 * --------------------------------------------------*/
void CloseSys(WGame &game) {
	warning("TODO, clean up properly");
	game.CleanUpAndPostQuit();
}


/* -----------------16/10/98 12.07-------------------
 *                  ReadTime
 * --------------------------------------------------*/
uint32 ReadTime() {
//	LARGE_INTEGER t;
//	QueryPerformanceCounter( &t );
	return (t3dReadTime());
}

/* -----------------16/10/98 12.24-------------------
 *              Render3DEnvironment
 * --------------------------------------------------*/
void Render3DEnvironment(WGame &game) {
	t3dV3F cp;
	int32 i;
//	Cancella buffers
	//if (!rClearBuffers(0, rCLEARBACKBUFFER | rCLEARZBUFFER))
	//  DebugLogWindow("Unable to clear backbuffers\n");
	t3dResetPipeline();
	// Update mouse coordinates and project them in 3D
	HandleMouseChanges();

	t3d3dMousePos = t3dV3F(game._renderer->screenSpaceToCameraSpace((t3dF32)mPosx, (t3dF32)mPosy));
	if (bT2DActive  /*&& ( bT2DActive != tOPTIONS )*/) {
	} else if (((InvStatus & INV_MODE2) && (InvStatus & INV_ON)) /*|| ( bT2DActive == tOPTIONS )*/) { // Se sono nell'inventario
		t3dCAMERA *tmp = t3dCurCamera;                                                          // Salvo camera attuale in tmp
		t3dMatIdentity(&t3dCurViewMatrix);
		t3dCurCamera = &game.init._globals._invVars.t3dIconCamera;                                                          // Attivo camera per icone
		t3dCurCamera->Fov = CAMERA_FOV_ICON;
		game._renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);
		auto windowInfo = game._renderer->getScreenInfos();
		game._renderer->setProjectionMatrix((float)(windowInfo.width), (float)(windowInfo.height), t3dCurCamera->Fov, 5000.0f, 15000.0f);
		t3dVectInit(&t3dCurCamera->Source, 1000.0f, 0.0f, 10000.0f);
		t3dVectInit(&t3dCurCamera->Target, 1000.0f, 0.0f, 0.0f);
		t3dMatView(&t3dCurViewMatrix, &t3dCurCamera->Source, &t3dCurCamera->Target);   // Creo matrice

		for (i = 0; i < (int32)game.init._globals._invVars.t3dIcons->NumMeshes(); i++) {
			game.init._globals._invVars.t3dIcons->MeshTable[i].Flags |= T3D_MESH_HIDDEN;                                    // Disattivo tutte le icone
			t3dMatCopy(&game.init._globals._invVars.t3dIcons->MeshTable[i].Matrix, &game.init._globals._invVars.BigIconM);

			if (BigInvObj) {
				if (game.init._globals._invVars.t3dIcons->MeshTable[i].name.equalsIgnoreCase((char *)game.init.InvObj[BigInvObj].meshlink.rawArray()))
					game.init._globals._invVars.t3dIcons->MeshTable[i].Flags &= ~T3D_MESH_HIDDEN;                           // Fuorche' quella selezinata
			}
		}

		if (!t3dTransformBody(game.init._globals._invVars.t3dIcons))
			DebugLogWindow("Can't transform Icons");
		t3dCurCamera = tmp;                                                                     // Rirpristino camera precedente
		t3dSortMeshes();                                                                        // Ordina le mesh
	} else {
		t3dOrigRoom = t3dCurRoom;                                                               // Si ricorda la stanza di partenza
		t3dMatIdentity(&t3dCurViewMatrix);                                                   // Aggiorna camera
		t3dMatView(&t3dCurViewMatrix, &t3dCurCamera->Source, &t3dCurCamera->Target);
		if (!t3dTransformBody(t3dCurRoom))                                                       // Proietta stanza
			DebugLogWindow("Can't transform %s", t3dCurRoom->name.c_str());
		t3dOrigRoom = nullptr;

		t3dProcessPortals();                                                                    // Processa i portali visibili

		game._cameraMan->GetRealCharPos(game.init, &cp, ocCURPLAYER, 0);
		PortalCrossed = t3dCheckPortalCrossed(&cp);                                              // Controlla se ha attraversato portali

		if (LoaderFlags & T3D_SKY)
			t3dTransformSky();
		else if (bGolfActive)
			t3dProcessGolfSky(t3dGolfSky);

		if (game.init.Obj[o48DARRELLSVENUTO].flags & ON) {
			for (i = 0; i < T3D_MAX_CHARACTERS; i++)
				if (Character[i] && (i != ocCURPLAYER) && (i != ocDARRELL))
					if (!t3dTransformCharacter(Character[i]))                                        // Proietta personaggi
						DebugLogWindow("Can't transform %s", Character[i]->Body->name.c_str());
		} else {
			for (i = 0; i < T3D_MAX_CHARACTERS; i++)
				if (Character[i] && (i != ocCURPLAYER))
					if (!t3dTransformCharacter(Character[i]))                                        // Proietta personaggi
						DebugLogWindow("Can't transform %s", Character[i]->Body->name.c_str());
		}

		if (game._gameVars.getCurRoomId() == r33) {
			for (i = 0; i < 4; i++)                                                              // Proietta oggetti speciali
				if (Lanc33[i] && !t3dTransformCharacter(Lanc33[i]))
					DebugLogWindow("Can't transform %s", Lanc33[i]->Body->name.c_str());
		}

		for (i = 0; i < MAX_GOPHERS; i++)
			if (GopherMark[i] && !t3dTransformCharacter(GopherMark[i]))
				DebugLogWindow("Can't transform %s", GopherMark[i]->Body->name.c_str());
		if (Freccia50 && !t3dTransformCharacter(Freccia50))
			DebugLogWindow("Can't transform %s", Freccia50->Body->name.c_str());
		if (Palla50 && !t3dTransformCharacter(Palla50))
			DebugLogWindow("Can't transform %s", Palla50->Body->name.c_str());
		if (Explode50 && !t3dTransformCharacter(Explode50))
			DebugLogWindow("Can't transform %s", Explode50->Body->name.c_str());

		t3dSortMeshes();                                                                        // Ordian le mesh
	}

	AfterRender(game);                                                                              // Disegna interfaccia
	Add3DStuff(game);                                                                           // Aggiunge layer 3D
	ProcessMaterialList();                                                                      // Compone la material list globale

	rRenderScene();                                                                             // Stampa triangoli

	game._renderer->add2DStuff(); // Aggiunge layer 2D
	game._renderer->showFrame();    // Visualizza il frame
}

} // End of namespace Watchmaker
