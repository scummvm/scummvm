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

#include "hollywood/hollywood.h"
#include "hollywood/font.h"
#include "hollywood/scenes/intro/scene9000.h"
#include "hollywood/scenes/intro/scene9010.h"
#include "hollywood/scenes/intro/scene9100.h"
#include "hollywood/resource.h"

#include "common/debug.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"

namespace Hollywood {

HollywoodEngine::HollywoodEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst),
		_gameDescription(gameDesc),
		_resources(new ResourceManager()),
		_font(new HollywoodFont()) {
}

HollywoodEngine::~HollywoodEngine() {
	delete _font;
	delete _resources;
}

Common::Error HollywoodEngine::run() {
	initGraphics(kScreenWidth, kScreenHeight);

	_font->load();

	debugC(1, kDebugGeneral, "Hollywood Monsters engine initialized");
	Scene9000 scene9000(this);
	if (!scene9000.play())
		return Common::kReadingFailed;

	Scene9010 scene9010(this);
	if (!scene9010.play())
		return Common::kReadingFailed;

	Scene9100 scene9100(this);
	if (!scene9100.play())
		return Common::kReadingFailed;

	debugC(1, kDebugGeneral, "Intro presentation completed");
	return Common::kNoError;
}

bool HollywoodEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher || f == kSupportsSubtitleOptions;
}

const char *HollywoodEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Language HollywoodEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform HollywoodEngine::getPlatform() const {
	return _gameDescription->platform;
}

} // End of namespace Hollywood
