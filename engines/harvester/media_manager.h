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

#ifndef HARVESTER_MEDIA_MANAGER_H
#define HARVESTER_MEDIA_MANAGER_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/str.h"

namespace Graphics {
class Screen;
}

namespace Harvester {

class Art;
class ResourceManager;
class EntityManager;
struct AudioCommand;
class Text;

class MediaManager {
public:
	explicit MediaManager(ResourceManager &resources);
	~MediaManager();

	EntityManager *getRuntimeEntities() const { return _entityManager; }
	Graphics::Screen *getScreen() const { return _screen; }
	int getDisplayWidth() const { return _displayWidth; }
	int getDisplayHeight() const { return _displayHeight; }
	Art *getArt() const { return _art; }
	Text *getText() const { return _text; }
	const Common::String &getMusicPath() const { return _musicPath; }

	static float mapGammaLevelToBrightnessScale(int level);

	void resetScreen(int width, int height);
	bool loadArt();
	bool loadText();
	bool loadQuickTipsResources();
	void drawWaitFrame() const;
	void applyMixerLevels(int fxLevel, int musicLevel);
	bool isMusicPlaying() const;
	bool playMusic(const Common::String &path);
	void pauseMusic(bool paused);
	void stopMusic();
	bool executeAudioCommand(const AudioCommand &command);
	bool playSound(const Common::String &path);
	bool playSingleSound(const Common::String &path);
	void stopSingleSound();
	bool isSingleSoundPlaying() const;
	bool playSpeech(const Common::String &path);
	void stopSpeech();
	bool isSpeechPlaying() const;
	bool loadSound(int slot, const Common::String &path);
	bool playLoadedSound(int slot);
	bool deleteLoadedSound(int slot);
	void stopSound();

private:
	void stopSoundHandle(Audio::SoundHandle &handle);
	bool validateLoadedSoundSlot(int slot) const;

	ResourceManager &_resources;
	EntityManager *_entityManager = nullptr;
	Graphics::Screen *_screen = nullptr;
	int _displayWidth = 0;
	int _displayHeight = 0;
	Art *_art = nullptr;
	Text *_text = nullptr;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _singleSoundHandle;
	Audio::SoundHandle _speechHandle;
	Common::String _musicPath;
	int _soundSlotIndex = -1;
	Common::String _soundPaths[8];
	Audio::SoundHandle _soundHandles[8];
	Common::String _loadedSoundPaths[4];
	Common::Array<byte> _loadedSoundData[4];
	Audio::SoundHandle _loadedSoundHandles[4];
};

} // End of namespace Harvester

#endif // HARVESTER_MEDIA_MANAGER_H
