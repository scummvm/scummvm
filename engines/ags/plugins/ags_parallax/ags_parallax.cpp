/*

This is not the AGS Parallax plugin by Scorpiorus
but a workalike plugin created by JJS for the AGS engine ports.

*/

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning(disable : 4244)
#endif

#if !defined(BUILTIN_PLUGINS)
#define THIS_IS_THE_PLUGIN
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "plugin/agsplugin.h"

#if defined(BUILTIN_PLUGINS)
namespace ags_parallax {
#endif

//#define DEBUG

const unsigned int Magic = 0xCAFE0000;
const unsigned int Version = 2;
const unsigned int SaveMagic = Magic + Version;

int screen_width = 320;
int screen_height = 200;
int screen_color_depth = 32;

IAGSEngine *engine;

bool enabled = false;


typedef struct {
	int x;
	int y;
	int slot;
	int speed;
} sprite_t;

#define MAX_SPEED 1000
#define MAX_SPRITES 100
sprite_t sprites[MAX_SPRITES];

// workaround to fix this error:
//   psp-fixup-imports ags_parallax.elf
//   Error, no .lib.stub section found
void dummy() {
	void *tmp = new int;
}

static size_t engineFileRead(void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = engine->FRead(ptr, size * count, fileHandle);
	return totalBytes / size;
}

static size_t engineFileWrite(const void *ptr, size_t size, size_t count, long fileHandle) {
	auto totalBytes = engine->FWrite(const_cast<void *>(ptr), size * count, fileHandle);
	return totalBytes / size;
}

void RestoreGame(long fileHandle) {
	unsigned int SaveVersion = 0;
	engineFileRead(&SaveVersion, sizeof(SaveVersion), 1, fileHandle);

	if (SaveVersion != SaveMagic) {
		engine->AbortGame("ags_parallax: bad save.");
	}

	engineFileRead(sprites, sizeof(sprite_t), MAX_SPRITES, fileHandle);
	engineFileRead(&enabled, sizeof(bool), 1, fileHandle);
}

void SaveGame(long file) {
	engineFileWrite(&SaveMagic, sizeof(SaveMagic), 1, file);
	engineFileWrite(sprites, sizeof(sprite_t), MAX_SPRITES, file);
	engineFileWrite(&enabled, sizeof(bool), 1, file);
}


void Initialize() {
	memset(sprites, 0, sizeof(sprite_t) * MAX_SPRITES);

	int i;
	for (i = 0; i < MAX_SPRITES; i++)
		sprites[i].slot = -1;

	enabled = false;
}


void Draw(bool foreground) {
	if (!enabled)
		return;

	BITMAP *bmp;
	int i;

	int offsetX = 0;
	int offsetY = 0;
	engine->ViewportToRoom(&offsetX, &offsetY);

	for (i = 0; i < MAX_SPRITES; i++) {
		if (sprites[i].slot > -1) {
			if (foreground) {
				if (sprites[i].speed > 0) {
					bmp = engine->GetSpriteGraphic(sprites[i].slot);
					if (bmp)
						engine->BlitBitmap(sprites[i].x - offsetX - (sprites[i].speed * offsetX / 100), sprites[i].y, bmp, 1);
				}
			} else {
				if (sprites[i].speed <= 0) {
					bmp = engine->GetSpriteGraphic(sprites[i].slot);
					if (bmp)
						engine->BlitBitmap(sprites[i].x - offsetX - (sprites[i].speed * offsetX / 1000), sprites[i].y, bmp, 1);
				}
			}
		}
	}
}





// ********************************************
// ************  AGS Interface  ***************
// ********************************************

void pxDrawSprite(int id, int x, int y, int slot, int speed) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%s %d %d %d %d %d\n", "pxDrawSprite", id, x, y, slot, speed);
	engine->PrintDebugConsole(buffer);
#endif

	if ((id < 0) || (id >= MAX_SPRITES))
		return;

	if ((speed < -MAX_SPEED) || (speed > MAX_SPEED))
		speed = 0;

	sprites[id].x = x;
	sprites[id].y = y;
	sprites[id].slot = slot;
	sprites[id].speed = speed;

	engine->RoomToViewport(&sprites[id].x, &sprites[id].y);

	enabled = true;
}


void pxDeleteSprite(int id) {
#ifdef DEBUG
	char buffer[200];
	sprintf(buffer, "%s %d\n", "pxDeleteSprite", id);
	engine->PrintDebugConsole(buffer);
#endif

	if ((id < 0) || (id >= MAX_SPRITES))
		return;

	sprites[id].slot = -1;
}

void AGS_EngineStartup(IAGSEngine *lpEngine) {
	engine = lpEngine;

	if (engine->version < 13)
		engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	engine->RegisterScriptFunction("pxDrawSprite", (void *)&pxDrawSprite);
	engine->RegisterScriptFunction("pxDeleteSprite", (void *)&pxDeleteSprite);

	engine->RequestEventHook(AGSE_PREGUIDRAW);
	engine->RequestEventHook(AGSE_PRESCREENDRAW);
	engine->RequestEventHook(AGSE_ENTERROOM);
	engine->RequestEventHook(AGSE_SAVEGAME);
	engine->RequestEventHook(AGSE_RESTOREGAME);

	Initialize();
}

void AGS_EngineShutdown() {
}

int AGS_EngineOnEvent(int event, int data) {
	if (event == AGSE_PREGUIDRAW) {
		Draw(true);
	} else if (event == AGSE_PRESCREENDRAW) {
		Draw(false);
	} else if (event == AGSE_ENTERROOM) {
		// Reset all sprites
		Initialize();
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size once here
		engine->GetScreenDimensions(&screen_width, &screen_height, &screen_color_depth);
		engine->UnrequestEventHook(AGSE_PRESCREENDRAW);
	} else if (event == AGSE_RESTOREGAME) {
		RestoreGame(data);
	} else if (event == AGSE_SAVEGAME) {
		SaveGame(data);
	}

	return 0;
}

int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved) {
	return 0;
}

void AGS_EngineInitGfx(const char *driverID, void *data) {
}



#if AGS_PLATFORM_OS_WINDOWS && !defined(BUILTIN_PLUGINS)

// ********************************************
// ***********  Editor Interface  *************
// ********************************************

const char *scriptHeader =
    "import void pxDrawSprite(int ID, int X, int Y, int SlotNum, int Speed);\r\n"
    "import void pxDeleteSprite(int ID);\r\n";

IAGSEditor *editor;


LPCSTR AGS_GetPluginName(void) {
	// Return the plugin description
	return "Parallax plugin recreation";
}

int  AGS_EditorStartup(IAGSEditor *lpEditor) {
	// User has checked the plugin to use it in their game

	// If it's an earlier version than what we need, abort.
	if (lpEditor->version < 1)
		return -1;

	editor = lpEditor;
	editor->RegisterScriptHeader(scriptHeader);

	// Return 0 to indicate success
	return 0;
}

void AGS_EditorShutdown() {
	// User has un-checked the plugin from their game
	editor->UnregisterScriptHeader(scriptHeader);
}

void AGS_EditorProperties(HWND parent) {
	// User has chosen to view the Properties of the plugin
	// We could load up an options dialog or something here instead
	MessageBoxA(parent, "Parallax plugin recreation by JJS", "About", MB_OK | MB_ICONINFORMATION);
}

int AGS_EditorSaveGame(char *buffer, int bufsize) {
	// We don't want to save any persistent data
	return 0;
}

void AGS_EditorLoadGame(char *buffer, int bufsize) {
	// Nothing to load for this plugin
}

#endif


#if defined(BUILTIN_PLUGINS)
} // namespace ags_parallax
#endif
