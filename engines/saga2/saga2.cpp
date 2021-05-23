/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "saga2/saga2.h"

#include "saga2/std.h"
#include "saga2/loadmsg.h"

namespace Saga2 {

void main_saga2();

Saga2Engine::Saga2Engine(OSystem *syst)
	: Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("saga2");

	debug("Saga2Engine::Saga2Engine");
}

Saga2Engine::~Saga2Engine() {
	debug("Saga2Engine::~Saga2Engine");

	freeExeResources();

	// Dispose your resources here
	delete _rnd;
}

Common::Error Saga2Engine::run() {
	// Initialize graphics using following:
	initGraphics(640, 480);

	// Additional setup.
	debug("Saga2Engine::init");

	// Your main even loop should be (invoked from) here.
	debug("Saga2Engine::go: Hello, World!");

	loadExeResources();

	loadingScreen();

	// Simple main event loop
	Common::Event evt;
	while (!shouldQuit()) {
		g_system->getEventManager()->pollEvent(evt);

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	main_saga2();

	return Common::kNoError;
}

bool Saga2Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error Saga2Engine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	return Common::kNoError;
}

Common::Error Saga2Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void Saga2Engine::syncGameStream(Common::Serializer &s) {
	// Use methods of Serializer to save/load fields
	int dummy = 0;
	s.syncAsUint16LE(dummy);
}

int32 clamp(int32 a, int32 val, int32 c) {
	if (val < a) return a;
	if (val > c) return c;
	return val;
}

gFont Onyx10Font;
gFont Plate18Font;
gFont Helv11Font;
gFont Amber13Font;
gFont ThinFix8Font;
gFont Script10Font;

uint32 loadingWindowWidth = 640;
uint32 loadingWindowHeight = 480;

uint8 *loadingWindowPalette;
uint8 *loadingWindowData;


static void loadFont(Common::File &file, gFont *font, uint32 offset) {
	file.seek(offset);

	font->height = file.readUint16LE();
	font->baseLine = file.readUint16LE();
	font->rowMod = file.readUint16LE();

	for (int i = 0; i < 256; i++)
		font->charXOffset[i] = file.readUint16LE();

	file.read(font->charWidth, 256);
	file.read(font->charKern, 256);
	file.read(font->charSpace, 256);

	uint size = font->height * font->rowMod;

	font->fontdata = (byte *)malloc(size);
	file.read(font->fontdata, size);
}

void Saga2Engine::loadExeResources() {
	Common::File exe;
	const uint32 offset = 0x4F6D90 - 0xF4990;

	if (!(exe.open("win/fta2win.exe") || exe.open("fta2win.exe")))
		error("FTA2WIN.EXE file is missing");

	if (exe.size() != 1093120)
		error("Incorrect FTA2WIN.EXE file size. Expected is 1093120");

	loadFont(exe, &Onyx10Font, 0x004F7258 - offset);
	loadFont(exe, &Plate18Font, 0x004F7EE0 - offset);
	loadFont(exe, &Helv11Font, 0x004F9F30 - offset);
	loadFont(exe, &Amber13Font, 0x004FAC60 - offset);
	loadFont(exe, &ThinFix8Font, 0x004FC210 - offset);
	loadFont(exe, &Script10Font, 0x004FCD18 - offset);

	loadingWindowPalette = (uint8 *)malloc(1024);
	exe.seek(0x004A2600 - offset);
	exe.read(loadingWindowPalette, 1024);

	loadingWindowData = (uint8 *)malloc(307200);
	exe.seek(0x004A2A00 - offset);
	exe.read(loadingWindowData, 307200);

	exe.close();
}

void Saga2Engine::freeExeResources() {
	free(Onyx10Font.fontdata);
	free(Plate18Font.fontdata);
	free(Helv11Font.fontdata);
	free(Amber13Font.fontdata);
	free(ThinFix8Font.fontdata);
	free(Script10Font.fontdata);

	free(loadingWindowPalette);
	free(loadingWindowData);
}

} // End of namespace Saga2
