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

#ifndef TITANIC_MUSIC_WAVE_H
#define TITANIC_MUSIC_WAVE_H

#include "common/array.h"
#include "titanic/support/string.h"

namespace Titanic {

class CProjectItem;
class CSoundManager;
class CWaveFile;

class CMusicWave {
	struct CMusicWaveFile {
		CWaveFile *_waveFile;
		int _value;
		CMusicWaveFile() : _waveFile(nullptr), _value(0) {}
	};
private:
	CProjectItem *_project;
	CSoundManager *_soundManager;
	Common::Array<CMusicWaveFile> _items;
private:
	/**
	 * Loads the specified wave file, and returns a CWaveFile instance for it
	 */
	CWaveFile *createWaveFile(const CString &name);
public:
	CMusicWave(CProjectItem *project, CSoundManager *soundManager, int index);

	/**
	 * Sets the maximum number of allowed files that be defined
	 */
	void setSize(uint count);

	/**
	 * Loads a new file into the list of available entries
	 */
	void load(int index, const CString &filename, int v3);

	/**
	 * Stops the music
	 */
	void stop();
};

} // End of namespace Titanic

#endif /* TITANIC_MUSIC_WAVE_H */
