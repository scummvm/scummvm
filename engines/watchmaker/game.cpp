/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "watchmaker/game.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/classes/do_system.h"
#include "watchmaker/3d/geometry.h"
#include "watchmaker/3d/loader.h"
#include "watchmaker/3d/math/llmath.h"
#include "watchmaker/3d/animation.h"
#include "watchmaker/define.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/ll/ll_diary.h"
#include "watchmaker/ll/ll_mesh.h"
#include "watchmaker/ll/ll_mouse.h"
#include "watchmaker/ll/ll_regen.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/ll/ll_util.h"
#include "watchmaker/walk/act.h"
#include "watchmaker/classes/do_player.h"
#include "watchmaker/classes/do_keyboard.h"
#include "watchmaker/classes/do_camera.h"
#include "watchmaker/walk/ball.h"
#include "watchmaker/sdl_wrapper.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

const char *WATCHMAKER_CFG_NAME = "wm.cfg";

WGame *_vm = nullptr;

// TODO: Unglobalize
const char *CharName_Strings[] = {
	"",
	"Darrell.t3d",
	"Victoria.t3d",
	"Cuoco.t3d",
	"Domestica.t3d",
	"Giardiniere.t3d",
	"Custode.t3d",
	"Servetta.t3d",
	"Supervisore.t3d",
	"MoglieSwim.t3d",
	"MoglieKimono.t3d",
	"Cacciatore.t3d",
	"Vecchio.t3d",
	"Chirurgo.t3d",
	"Traduttore.t3d",
	"Orologiaio.t3d",
	"Krenn.t3d",
	"Dukes.t3d",
	"Corona.t3d",
	"Valencia.t3d",
	"Notaio.t3d",
	"Moore.t3d",
	"DarrellALetto.t3d",
	"CacciatoreMalpreso.t3d",
	"MooreBucato.t3d",
	nullptr
};

const char *CharNameHI_Strings[] = {
	"",
	"DarrellHI.t3d",
	"VictoriaHI.t3d",
	"CuocoHI.t3d",
	"DomesticaHI.t3d",
	"GiardiniereHI.t3d",
	"CustodeHI.t3d",
	"ServettaHI.t3d",
	"SupervisoreHI.t3d",
	"MoglieSwimHI.t3d",
	"MoglieKimonoHI.t3d",
	"CacciatoreHI.t3d",
	"VecchioHI.t3d",
	"ChirurgoHI.t3d",
	"TraduttoreHI.t3d",
	"OrologiaioHI.t3d",
	"KrennHI.t3d",
	"DukesHI.t3d",
	"CoronaHI.t3d",
	"ValenciaHI.t3d",
	"NotaioHI.t3d",
	"MooreHI.t3d",
	"DarrellALettoHI.t3d",
	"CacciatoreMalpresoHI.t3d",
	"MooreBucatoHI.t3d",
	nullptr
};

/* -----------------16/11/00 17.45-------------------
 *                  CheckAndLoadMoglieSupervisoreModel
 * --------------------------------------------------*/
//	Controllo che la giusta MoglieSupervisore sia caricata
bool WGame::CheckAndLoadMoglieSupervisoreModel(int32 c) {
	warning("CheckAndLoadMoglieSupervisoreModel(workDirs, %d)", c);
	char RemoveName[128] = "";
	char RemoveNameHI[128] = "";

	if (c == ocMOGLIESUPERVISORE) {
		if (bMoglieGym && (!Character[c]->Body->name.contains("MoglieGym"))) {
			CharName_Strings[c] = "MoglieGym.t3d";
			Common::strlcpy(RemoveName, "MoglieSwim.t3d", 128);
			CharNameHI_Strings[c] = "MoglieGymHI.t3d";
			Common::strlcpy(RemoveNameHI, "MoglieSwimHI.t3d", 128);
		}

		if ((!bMoglieGym) && (!Character[c]->Body->name.contains("MoglieSwim"))) {
			CharName_Strings[c] = "MoglieSwim.t3d";
			Common::strlcpy(RemoveName, "MoglieGym.t3d", 128);
			CharNameHI_Strings[c] = "MoglieSwimHI.t3d";
			Common::strlcpy(RemoveNameHI, "MoglieGymHI.t3d", 128);
		}
	}

	if (c == ocMOGLIE_KIMONO) {
		if (bMoglieSangue && (!Character[c]->Body->name.contains("MoglieKimonoSangue"))) {
			CharName_Strings[c] = "MoglieKimonoSangue.t3d";
			Common::strlcpy(RemoveName, "MoglieKimono.t3d", 128);
			CharNameHI_Strings[c] = "MoglieKimonoSangueHI.t3d";
			Common::strlcpy(RemoveNameHI, "MoglieKimonoHI.t3d", 128);
		}

		if ((!bMoglieSangue) && (!Character[c]->Body->name.contains("MoglieKimono"))) {
			CharName_Strings[c] = "MoglieKimono.t3d";
			Common::strlcpy(RemoveName, "MoglieKimonoSangue.t3d", 128);
			CharNameHI_Strings[c] = "MoglieKimonoHI.t3d";
			Common::strlcpy(RemoveNameHI, "MoglieKimonoSangueHI.t3d", 128);
		}
	}


	if (RemoveName[0] || RemoveNameHI[0]) {
		if (LoaderFlags & T3D_HIPOLYPLAYERS)
			CharName[c] = CharNameHI_Strings[c];
		else
			CharName[c] = CharName_Strings[c];

		t3dReleaseCharacter(Character[c]);
		Character[c] = nullptr;

		_vm->_roomManager->releaseBody(RemoveName, RemoveNameHI);

		LoaderFlags |= T3D_PRELOADBASE;
		LoaderFlags |= T3D_STATIC_SET1;
		LoaderFlags |= T3D_STATIC_SET0;
		rSetLoaderFlags(LoaderFlags);
		if (!(Character[c] = t3dLoadCharacter(*this, CharName[c], (uint16)c))) {
			warning("Error loading %s", CharName[c]);
			return false;
		}
		LoaderFlags &= ~T3D_PRELOADBASE;
		LoaderFlags &= ~T3D_STATIC_SET1;
		LoaderFlags &= ~T3D_STATIC_SET0;
		rSetLoaderFlags(LoaderFlags);

	}//if removename

	return (true);
}

// TODO: This needs some heavy refactoring.
WGame::WGame() : workDirs(WATCHMAKER_CFG_NAME) {
	_vm = this;
	_meshModifiers = new MeshModifiers();
	_roomManager = RoomManager::create(this);
	_cameraMan = new CameraMan();
	configLoaderFlags(); // TODO: This should probably happen before the constructor

	// if LoaderFlags & T3D_DEBUGMODE
	// ...
	// }

	gameOptions.load(workDirs);

	// if (LoaderFlags & T3D_DEBUGMODE) {
	// ...
	// } else

	warning("Currently doing an unnecessary copy here");
	loadAll(workDirs, init);

	sdl = new sdl_wrapper();

	_renderer = new Renderer(this, sdl);

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("Watchmaker");
}

WGame::~WGame() {
	delete _renderer;
	delete sdl;
	delete _meshModifiers;
	delete _roomManager;
	delete _rnd;
	delete _cameraMan;
	_vm = nullptr;
}

Common::SharedPtr<Common::SeekableReadStream> WGame::resolveFile(const char *path, bool noFastFile) {
	return workDirs.resolveFile(path, noFastFile);
}

void WGame::initCharNames() {
	//Init characters names
	if (LoaderFlags & T3D_HIPOLYPLAYERS) {
		CharName[ocDARRELL]         = CharNameHI_Strings[ocDARRELL];
		CharName[ocVICTORIA]        = CharNameHI_Strings[ocVICTORIA];
		CharName[ocDARRELLALETTO]   = CharNameHI_Strings[ocDARRELLALETTO];
	} else {
		CharName[ocDARRELL]         = CharName_Strings[ocDARRELL];
		CharName[ocVICTORIA]        = CharName_Strings[ocVICTORIA];
		CharName[ocDARRELLALETTO]   = CharName_Strings[ocDARRELLALETTO];
	}

	for (int i = ocCUOCO; i <= ocLASTCHAR; i++)
		if (i != ocDARRELLALETTO) {
			if (LoaderFlags & T3D_HIPOLYCHARACTERS)
				CharName[i] = CharNameHI_Strings[i];
			else
				CharName[i] = CharName_Strings[i];
		}

	//l'orologiaio e' sempre hipoly, perche' nell'rtv della sua morte la mesh e' hipoly
	CharName[ocOROLOGIAIO] = CharNameHI_Strings[ocOROLOGIAIO];
}

/* -----------------04/10/00 15.45-------------------
 *                  StartPlayingGame
 * --------------------------------------------------*/
// Serve quando si deve far partire il gioco dall'inizio
int WGame::StartPlayingGame(const Common::String &LoaderName_override) {
	warning("StartPlayingGame");
	int retv = 0;

	if (!LoaderName_override.empty())
		retv = LoadAndSetup(LoaderName_override, 0);                  // Carica T3D e Bitmap
	else
		retv = LoadAndSetup(LoaderName, 0);                           // Carica T3D e Bitmap

	if (!retv) return false;

	UpdateAll();
	InitMain(*this);

//	resetto alcune variabili che potrebbero rimanere sporche dalla partita prima (uguale per StartPlayingGame(), DataLoad() )
	bCacciatore = 0;
	bSaveDisabled = 0;
	bNotSkippableSent = 0;
	bPorteEsternoBloccate = 0;
	bNoPlayerSwitch = 0;
	bDarkScreen = false;
	bSuperView = 0;
	bSezioneLabirinto = 0;
	bSkipTalk = false;
	bSomeOneSpeak = false;
	bPlayerSpeak = false;
	bWideScreen = 0;
	bTitoliCodaStatic = 0;
	bTitoliCodaScrolling = 0;
	Common::strlcpy(RoomInfo.name, "", 64);

	return true;
}

/* -----------------16/10/98 12.15-------------------
 *                  LoadAndSetup
 * --------------------------------------------------*/
bool WGame::LoadAndSetup(const Common::String &name, uint8 lite) {
	uint16 i = 0;

	warning("--=>> %s <<=--: LoaderFlags: %08X", name.c_str(), LoaderFlags);
	t3dCurCamera = &init._globals._invVars.t3dIconCamera;

	auto windowInfo = _renderer->getScreenInfos();

	if (LoaderFlags & T3D_STATIC_SET0) {
		if (LoaderFlags & T3D_PRELOADBASE) {
			_renderer->setVirtualScreen(SCREEN_RES_X, SCREEN_RES_Y);
			warning("Window %dx%d", windowInfo.width, windowInfo.height);
			_renderer->createScreenBuffer();
			_renderer->initBlitterViewPort();

			LoadMisc();
			t3dVectInit(&CharCorrection, 0.0f, 0.0f, /*14*/0.0f * 2.55f * SCALEFACTOR);

//			if( lite )
//				PrintLoading();

			if (!(LoaderFlags & T3D_NOICONS))
				if (!(init._globals._invVars.t3dIcons = _roomManager->loadRoom("Icons.t3d", init._globals._invVars.t3dIcons, &i, (LoaderFlags | T3D_NORECURSION | T3D_NOLIGHTMAPS | T3D_NOBOUNDS | T3D_NOCAMERAS | T3D_STATIC_SET1)))) {
					warning("Error loading Icons");
					return false;
				}
			if (!(Lanc33[0] = t3dLoadCharacter(*this, "Lanc33a.t3d", 99)) || !(Lanc33[1] = t3dLoadCharacter(*this, "Lanc33b.t3d", 99)) ||
			        !(Lanc33[2] = t3dLoadCharacter(*this, "Lanc33c.t3d", 99)) || !(Lanc33[3] = t3dLoadCharacter(*this, "Lanc33d.t3d", 99))) {
				warning("Error loading Lanc33");
				return false;
			}

			warning("Reached loop: %02X", LoadChar);
			for (i = 0; i < 32; i++) {
				if (i >= ocCURPLAYER) continue;

				if (LoadChar & (1 << i)) {
					if ((i + 1) == ocMOGLIESUPERVISORE) {
						if (bMoglieGym) {
							CharName_Strings[i + 1] = "MoglieGym.t3d";
							CharNameHI_Strings[i + 1] = "MoglieGymHI.t3d";
						} else {
							CharName_Strings[i + 1] = "MoglieSwim.t3d";
							CharNameHI_Strings[i + 1] = "MoglieSwimHI.t3d";
						}
					}
					if ((i + 1) == ocMOGLIE_KIMONO) {
						if (bMoglieSangue) {
							CharName_Strings[i + 1] = "MoglieKimonoSangue.t3d";
							CharNameHI_Strings[i + 1] = "MoglieKimonoSangueHI.t3d";
						} else {
							CharName_Strings[i + 1] = "MoglieKimono.t3d";
							CharNameHI_Strings[i + 1] = "MoglieKimono.t3d";
						}
					}
					warning("About to load %s", CharName[i + 1]);
					if (!(Character[i + 1] = t3dLoadCharacter(*this, CharName[i + 1], i))) {
						warning("Error loading %s", CharName[i + 1]);
						return false;
					}
				}
			}
			LoaderFlags &= ~T3D_PRELOADBASE;
			LoaderFlags &= ~T3D_STATIC_SET1;
			rSetLoaderFlags(LoaderFlags);
		}

		if (!lite) {
			if (LoaderFlags & T3D_PRELOAD_RXT) {
				t3dCurOliSet = -1;
				// TODO: Figure out how this i works out.
				if (!(t3dRxt = _roomManager->loadRoom("rxt.t3d", t3dCurRoom, &i, LoaderFlags | T3D_NOLIGHTMAPS))) {
					warning("Error loading room rxt.t3d");
					return false;
				}
			}
			if (LoaderFlags & T3D_SKY) {
				t3dLoadSky(*this, t3dCurRoom);
				t3dCreateProceduralSky();
			}

			LoaderFlags &= ~T3D_STATIC_SET0;
			rSetLoaderFlags(LoaderFlags);
		}

		i = 0;
	}

	if (!((LoaderFlags & T3D_PRELOAD_RXT) && name.equalsIgnoreCase("rxt.t3d"))) {
		if (!(t3dCurRoom = _roomManager->loadRoom(name, t3dCurRoom, &i, LoaderFlags))) {
			warning("Error loading room %s", name.c_str());
			return false;
		}
	} else
		t3dCurRoom = t3dRxt;

	if ((!lite) && (!(LoaderFlags & T3D_DEBUGMODE))) {
		if (!CheckAndLoadMoglieSupervisoreModel(ocMOGLIESUPERVISORE)) return false;
		if (!CheckAndLoadMoglieSupervisoreModel(ocMOGLIE_KIMONO)) return false;
	}

	if (bGolfActive) {
//		extern t3dS32 SkySurface;
		if (!(Character[1] = t3dLoadCharacter(*this, "Gopher1.t3d", 99))) {
			warning("Error loading Gopher1");
			return false;
		}
		if (!(Character[2] = t3dLoadCharacter(*this, "Gopher2.t3d", 99))) {
			warning("Error loading Gopher2");
			return false;
		}
		if (!(Character[3] = t3dLoadCharacter(*this, "Gopher3.t3d", 99))) {
			warning("Error loading Gopher3");
			return false;
		}
		if (!(GopherMark[0] = t3dLoadCharacter(*this, "Mark1.t3d", 99))) {
			warning("Error loading Mark1");
			return false;
		}
		if (!(GopherMark[1] = t3dLoadCharacter(*this, "Mark2.t3d", 99))) {
			warning("Error loading Mark2");
			return false;
		}
		if (!(GopherMark[2] = t3dLoadCharacter(*this, "Mark3.t3d", 99))) {
			warning("Error loading Mark3");
			return false;
		}
		if (!(Palla50 = t3dLoadCharacter(*this, "Palla.t3d", 99))) {
			warning("Error loading Palla50");
			return false;
		}
		if (!(Freccia50 = t3dLoadCharacter(*this, "Freccia.t3d", 99))) {
			warning("Error loading Freccia50");
			return false;
		}
		if (!(Explode50 = t3dLoadCharacter(*this, "Explode.t3d", 99))) {
			warning("Error loading Explode50");
			return false;
		}
//		t3dLoadSky(t3dCurRoom);
//		t3dCreateProceduralSky();
		t3dGolfSky = LinkMeshToStr(init, "p50-cielo");
//		t3dCurRoom->MatTable[Obj[oNEXTPORTAL].pos].Texture=rGetSurfaceTexture(SkySurface);

		Palla50->Flags &= ~T3D_CHARACTER_HIDE;
		Palla50->Mesh->Flags &= ~T3D_MESH_HIDDEN;

		Freccia50->Flags |= T3D_CHARACTER_HIDE;
		Freccia50->Mesh->Flags &= ~T3D_MESH_HIDDEN;

		Explode50->Flags |= T3D_CHARACTER_HIDE;
		Explode50->Mesh->Flags &= ~T3D_MESH_HIDDEN;

		Character[ocVICTORIA]->Flags &= ~T3D_CHARACTER_HIDE;
		CharSetPosition(ocVICTORIA, 7, nullptr);
		t3dClipToSurface(init, &Character[ocVICTORIA]->Mesh->Trasl);
		t3dLightChar(Character[ocVICTORIA]->Mesh, &Character[ocVICTORIA]->Mesh->Trasl);

		Character[ocCUOCO]->Flags &= ~T3D_CHARACTER_HIDE;
		CharSetPosition(ocCUOCO, 13, nullptr);
		t3dClipToSurface(init, &Character[ocCUOCO]->Mesh->Trasl);
		t3dLightChar(Character[ocCUOCO]->Mesh, &Character[ocCUOCO]->Mesh->Trasl);

		GopherMark[0]->Flags |= T3D_CHARACTER_HIDE;
		GopherMark[1]->Flags |= T3D_CHARACTER_HIDE;
		GopherMark[2]->Flags |= T3D_CHARACTER_HIDE;

		CurGopher = -1;
		NextGopher(*this);
		InitPhys(&Ball[CurGopher]);
	}

	t3dCurCharacter = Player = Character[ocCURPLAYER] = Character[ocDARRELL + CurPlayer];
	if (lite)   Player->Flags |= T3D_CHARACTER_HIDE;
	else        Player->Flags &= ~T3D_CHARACTER_HIDE;

	Character[ocDARRELL]->Flags |= (T3D_CHARACTER_CASTREALTIMESHADOWS | T3D_CHARACTER_REALTIMELIGHTING | T3D_CHARACTER_VOLUMETRICLIGHTING);
	Character[ocVICTORIA]->Flags |= (T3D_CHARACTER_CASTREALTIMESHADOWS | T3D_CHARACTER_REALTIMELIGHTING | T3D_CHARACTER_VOLUMETRICLIGHTING);
	Player->Flags               |= (T3D_CHARACTER_CASTREALTIMESHADOWS | T3D_CHARACTER_REALTIMELIGHTING | T3D_CHARACTER_VOLUMETRICLIGHTING);

	CurFloorY = t3dCurRoom->PanelHeight[t3dCurRoom->CurLevel];

	t3dMatIdentity(&init._globals._invVars.CameraDummy.Matrix);
	t3dStartTime();

	if (!t3dCurRoom->CameraTable.empty())
		if (!(t3dCurCamera = _cameraMan->PickCamera(t3dCurRoom, 0)))
			t3dCurCamera = &t3dCurRoom->CameraTable[0];
	if (t3dCurCamera)
		init._globals._invVars.t3dIconCamera = *t3dCurCamera;

	SetCurPlayerPosTo_9x(_gameVars, init);

	Player->Pos.y = Player->Mesh->Trasl.y = CurFloorY;

	_cameraMan->GetCameraTarget(init, &t3dCurCamera->Target);
	_renderer->setCurCameraViewport(t3dCurCamera->Fov, bSuperView);

	mPosx = windowInfo.width / 2;
	mPosy = windowInfo.height / 2;

	if (lite) {
		ResetScreenBuffer();    //resetto in modo che la scritta di loading non faccia casini
		UpdateAll();
		InitMain(*this);
	}

	return true;
}

/* -----------------13/07/00 15.21-------------------
 *                  UpdateAll
 * --------------------------------------------------*/
void WGame::UpdateAll() {
	UpdateRoomVisibility(*this);
	auto bodies = _roomManager->getLoadedFiles();
	for (auto loadedBody : bodies) {
		HideRoomMeshes(init, loadedBody);
		_meshModifiers->applyAllMeshModifiers(*this, loadedBody);
	}

	UpdateAllClocks(*this);

//	Init for Clock33
	t3dMatIdentity(&Lanc33[0]->Mesh->Matrix);
	t3dMatIdentity(&Lanc33[1]->Mesh->Matrix);
	t3dMatIdentity(&Lanc33[2]->Mesh->Matrix);
	t3dMatIdentity(&Lanc33[3]->Mesh->Matrix);
	t3dVectInit(&Lanc33[2]->Mesh->Trasl, -2402.881f, 350.092f, -5110.331f);
	Lanc33[2]->Flags &= ~T3D_CHARACTER_HIDE;
	t3dVectInit(&Lanc33[3]->Mesh->Trasl, -2402.881f, 350.092f, -5110.331f);
	Lanc33[3]->Flags &= ~T3D_CHARACTER_HIDE;
	t3dVectInit(&Lanc33[0]->Mesh->Trasl, -2441.371f, 350.092f, -5110.331f);
	Lanc33[0]->Flags &= ~T3D_CHARACTER_HIDE;
	t3dVectInit(&Lanc33[1]->Mesh->Trasl, -2441.371f, 350.092f, -5110.331f);
	Lanc33[1]->Flags &= ~T3D_CHARACTER_HIDE;
	t3dVectInit(&mPos, -2484.0f, 381.0f, -5085.0f);
	bClock33 = 0;
	doClock33(*this, o33LANCETTAHSX, &mPos);
	t3dVectInit(&mPos, -2390.0f, 330.0f, -5085.0f);
	bClock33 = 0;
	doClock33(*this, o33LANCETTAMSX, &mPos);
	t3dVectInit(&mPos, -2415.0f, 325.0f, -5085.0f);
	bClock33 = 0;
	doClock33(*this, o33LANCETTAHDX, &mPos);
	t3dVectInit(&mPos, -2476.0f, 397.0f, -5085.0f);
	bClock33 = 0;
	doClock33(*this, o33LANCETTAMDX, &mPos);
	bClock33 = 0;
}

/* -----------------27/10/98 16.27-------------------
 *
 * --------------------------------------------------*/
void WGame::LoadMisc() {
	warning("LoadMisc");
	char str[20];
	int32 i;

	auto windowInfo = _renderer->getScreenInfos();

	if (bMiscLoaded)    return;

	/*
	    rClear( BACK_BUFFER, 0, 0, MainDx, MainDy, 0, 0, 0 );
	    rBlitScreenBuffer();
	    rShowFrame();

	*/
	_fonts.loadFonts(*this, windowInfo);

	Console1 = LoadDDBitmap(*this, "Console1.tga", rSURFACESTRETCH);
	ConsoleFrecciaGiu = LoadDDBitmap(*this, "consoleFrecciaGiu.tga", rSURFACESTRETCH);
	ConsoleFrecciaSu = LoadDDBitmap(*this, "consoleFrecciaSu.tga", rSURFACESTRETCH);
	Console2 = LoadDDBitmap(*this, "Console2.tga", rSURFACESTRETCH);
	Console3 = LoadDDBitmap(*this, "Console3.tga", rSURFACESTRETCH);
	Console4 = LoadDDBitmap(*this, "Console4.tga", rSURFACESTRETCH);
	ConsoleNoSwitchDar = LoadDDBitmap(*this, "ConsoleNoSwitchDar.tga", rSURFACESTRETCH);
	ConsoleNoSwitchVic = LoadDDBitmap(*this, "ConsoleNoSwitchVic.tga", rSURFACESTRETCH);
	ConsoleNoSave = LoadDDBitmap(*this, "ConsoleNoSave.tga", rSURFACESTRETCH);
	ConsoleD1 = LoadDDBitmap(*this, "ConsoleD1.tga", rSURFACESTRETCH);
	ConsoleD2 = LoadDDBitmap(*this, "ConsoleD2.tga", rSURFACESTRETCH);
	Console5 = LoadDDBitmap(*this, "Console5.tga", rSURFACESTRETCH);
	Console6 = LoadDDBitmap(*this, "Console6.tga", rSURFACESTRETCH);
	NewLogImage = LoadDDBitmap(*this, "NewLogImage.tga", rSURFACESTRETCH);
	LoadingImage = LoadDDBitmap(*this, "Loading.tga", rSURFACESTRETCH);
	MousePointerDefault = LoadDDBitmap(*this, "Pointer.tga", 0);
	MousePointerPlus = LoadDDBitmap(*this, "PointerPlus.tga", 0);
	CurMousePointer = MousePointerDefault;
	if (bGolfActive) {
		GopherMap = LoadDDBitmap(*this, "GopherMap.tga", rSURFACESTRETCH);
		GopherPos[0] = LoadDDBitmap(*this, "Gopher1.tga", rSURFACESTRETCH);
		GopherPos[1] = LoadDDBitmap(*this, "Gopher2.tga", rSURFACESTRETCH);
		GopherPos[2] = LoadDDBitmap(*this, "Gopher3.tga", rSURFACESTRETCH);
		GopherBall = LoadDDBitmap(*this, "GopherBall.tga", rSURFACESTRETCH);
		EndPic = LoadDDBitmap(*this, "TrecLogo.tga", rSURFACESTRETCH);
	}
	MousePointerLim.x1 = 0;
	MousePointerLim.y1 = 0;
	MousePointerLim.x2 = windowInfo.width;
	MousePointerLim.y2 = windowInfo.height;
	//TrecLogo = LoadDDBitmap( "TrecLogo.tga", rSURFACESTRETCH );

	for (i = 1; i < 85; i++) {
		snprintf(str, 20, "I%03d.tga", i);
		IconsPics[i] = LoadDDBitmap(*this, str, rSURFACESTRETCH);
	}

	bMiscLoaded = 1;
	warning("LoadMisc Done");
}

void WGame::GameLoop() {
	bool done = false;
	//bool bGotMsg = false;
	//MSG  msg;

	// TODO: These two should be adjusted if the game loses focus or needs to get a new context.
	g_bActive = g_bReady = true;

	while (!done) {                           // MainLoop
		/* TODO: Throttle the loop
		if( g_bActive )
		    bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
		else
		    bGotMsg = GetMessage( &msg, NULL, 0U, 0U );

		if( bGotMsg )
		{
		    TranslateMessage( &msg );
		    DispatchMessage( &msg );
		}
		else
		{*/
		//sleep(1);

		// TODO: The SDL polling should go inside ProcessTime to match the expected behaviour wrt keyboard/mouse
		sdl->pollSDL();
		done = sdl->shouldQuit;
		if (g_bActive && g_bReady) {
			//warning("NextMessage");
			NextMessage(*this);
		}
		//}
	}
}

void WGame::CleanUpAndPostQuit() {
	gameOptions.save(workDirs);
	warning("STUBBED CleanupAndPostQuit");
#if 0
	extern char *TextBucket;

	if (CreditsNames) t3dFree(CreditsNames);
	if (CreditsRoles) t3dFree(CreditsRoles);

	if ((RoomInfo.tnum) && (RoomInfo.tnum != -1)) {
		rReleaseBitmap(RoomInfo.tnum);
		RoomInfo.tnum = 0;
	}

	CheckExtraLocalizationStrings(0);

	for (uint16 i = 0; i < T3D_MAX_CHARACTERS; i++) {
		if ((i != ocCURPLAYER) && (Character[i]))
			CharStop(i);

		if (i < (T3D_MAX_CHARACTERS - 1))
			t3dReleaseCharacter(Character[i]);
		Character[i] = nullptr;
	}
	Player = nullptr;
	for (uint16 i = 0; i < NumLoadedFiles; i++) {
		t3dReleaseBody(LoadedFiles[i].b);
		LoadedFiles[i].b = nullptr;
	}

	ReleasePreloadedAnims();
	t3dReleaseParticles();

	rReleaseAllTextures(0);
	rReleaseAllBitmaps(0);

	if (TextBucket) free(TextBucket);
	t3dFree(StandardFont.Table);

	t3dDeallocMemoryPool();
	t3dEndTime();

	DInputTermMouse();
	DInputTermKbd();
	CloseMusic();
	CloseLogWindow();
	rCleanup3DEnvironment();
	t3dFastFileFinish();

	PostQuitMessage(0);

	remove("temp.tmp");

	exit(0);
#endif
}

void WGame::addMeshModifier(const Common::String &name, int16 com, void *p) {
	_meshModifiers->addMeshModifier(name, com, p);
}

void WGame::loadMeshModifiers(Common::SeekableReadStream &stream) {
	delete _meshModifiers;
	_meshModifiers = new MeshModifiers(stream);
}

} // End of namespace Watchmaker
