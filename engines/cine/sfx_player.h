/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CINE_SFXPLAYER_H_
#define CINE_SFXPLAYER_H_

namespace Cine {

class SoundDriver;

class SfxPlayer {
public:

	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};
	
	SfxPlayer(SoundDriver *driver);
	~SfxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void fadeOut();

	static void updateCallback(void *ref);

private:

	void update();
	void handleEvents();
	void handlePattern(int channel, const uint8 *patternData);
	void unload();

	bool _playing;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	uint8 *_sfxData;
	uint8 *_instrumentsData[NUM_INSTRUMENTS];
	SoundDriver *_driver;
};

extern SfxPlayer *g_sfxPlayer; // TEMP

} // End of namespace Cine

#endif				/* _SFXPLAYER_H_ */
