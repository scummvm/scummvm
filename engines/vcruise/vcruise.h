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

#ifndef VCRUISE_H
#define VCRUISE_H

#include "engines/engine.h"
#include "common/rect.h"

#include "vcruise/runtime.h"
#include "vcruise/detection.h"

/**
 * This is the namespace of the V-Cruise engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Reah: Face the Unknown
 * - Schizm: Mysterious Journey
 */
namespace VCruise {

class VCruiseEngine : public ::Engine {
protected:
	// Engine APIs
	Common::Error run() override;

public:
	VCruiseEngine(OSystem *syst, const VCruiseGameDescription *gameDesc);
	~VCruiseEngine() override;

	bool hasFeature(EngineFeature f) const override;
	void syncSoundSettings() override;

	const VCruiseGameDescription *_gameDescription;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	bool canSaveAutosaveCurrently() override;
	bool canSaveGameStateCurrently() override;
	bool canLoadGameStateCurrently() override;

	void initializePath(const Common::FSNode &gamePath) override;

	bool hasDefaultSave();
	bool hasAnySave();

protected:
	void pauseEngineIntern(bool pause) override;

private:
	void handleEvents();

	Common::Rect _videoRect;
	Common::Rect _menuBarRect;
	Common::Rect _trayRect;

	Common::FSNode _rootFSNode;

	Common::SharedPtr<Runtime> _runtime;
};

} // End of namespace VCruise

#endif /* VCRUISE_H */
