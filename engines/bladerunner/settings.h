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

#ifndef BLADERUNNER_SETTINGS_H
#define BLADERUNNER_SETTINGS_H

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;

class Settings {
	static const int kAmmoTypesCount = 3;

	BladeRunnerEngine *_vm;

	int   _chapter;
	int   _scene;
	int   _set;
	int   _unk0;
	float _gamma;

	bool  _chapterChanged;
	int   _newChapter;
	int   _newScene;
	int   _newSet;

	bool  _startingGame;
	bool  _loadingGame;

	// int   _unk1;

	int   _fullHDFrames;
	int   _mst3k;

	int   _difficulty;
	int   _playerAgenda;

	int   _ammoType;
	int   _ammoAmounts[kAmmoTypesCount];

	bool  _learyMode;

public:
	Settings(BladeRunnerEngine *vm);

	void reset();

	void setGamma(float gamma) {
		_gamma = gamma;
	}

	void setNewSetAndScene(int set, int scene) {
		_newSet = set;
		_newScene = scene;
	}

	void clearNewSetAndScene() {
		_newSet = -1;
		_newScene = -1;
	}

	int getNewScene() const {
		return _newScene;
	}

	int getNewSet() const {
		return _newSet;
	}

	int getScene() const {
		return _scene;
	}

	int getSet() const {
		return _set;
	}

	int getChapter() const {
		return _chapter;
	}

	void setChapter(int newChapter) {
		_chapterChanged = true;
		_newChapter = newChapter;
	}

	void setLoadingGame() {
		_loadingGame = true;
	}

	bool isLoadingGame() const {
		return _loadingGame;
	}

	void setStartingGame() {
		_startingGame = true;
	}

	bool openNewScene();

	static int getAmmoTypesCount();
	int getAmmoType() const;
	void setAmmoType(int ammoType);
	int getAmmo(int ammoType) const;
	void addAmmo(int ammoType, int ammo);
	void decreaseAmmo();

	int getDifficulty() const;
	void setDifficulty(int difficulty);

	int getPlayerAgenda() const;
	void setPlayerAgenda(int agenda);

	bool getLearyMode() const;
	void setLearyMode(bool learyMode);

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
