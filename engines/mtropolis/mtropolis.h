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

#ifndef MTROPOLIS_MTROPOLIS_H
#define MTROPOLIS_MTROPOLIS_H

#include "engines/engine.h"

#include "common/random.h"

#include "mtropolis/detection.h"
#include "mtropolis/saveload.h"

/**
 * This is the namespace of the mTropolis engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Obsidian
 */
namespace MTropolis {

class Runtime;
class RuntimeObject;

class MTropolisEngine : public ::Engine, public ISaveUIProvider, public ILoadUIProvider, public IAutoSaveProvider {
protected:
	// Engine APIs
	Common::Error run() override;

public:
	MTropolisEngine(OSystem *syst, const MTropolisGameDescription *gameDesc);
	~MTropolisEngine() override;

	bool hasFeature(EngineFeature f) const override;
	//void syncSoundSettings() override;

	const MTropolisGameDescription *_gameDescription;
	uint32 getGameID() const;
	Common::Platform getPlatform() const;

	bool promptSave(ISaveWriter *writer, const Graphics::Surface *screenshotOverride) override;
	bool autoSave(ISaveWriter *writer) override;
	bool promptLoad(ISaveReader *reader) override;

	const Graphics::Surface *getSavegameScreenshot() const;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave) override;
	bool canSaveAutosaveCurrently() override;
	bool canSaveGameStateCurrently() override;	

public:
	void handleEvents();

protected:
	void pauseEngineIntern(bool pause) override;

private:
	static const uint kCurrentSaveFileVersion = 1;
	static const uint kSavegameSignature = 0x6d545356;	// mTSV

	ISaveWriter *_saveWriter;
	bool _isTriggeredAutosave;

	Common::ScopedPtr<Runtime> _runtime;
};

} // End of namespace MTropolis

#endif /* MTROPOLIS_MTROPOLIS_H */
