/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"

#include "rect.h"
#include "chunk.h"
#include "palette.h"
#include "codec1.h"
#include "codec37.h"
#include "codec44.h"
#include "codec47.h"

class FontRenderer;
class Mixer;
class Renderer;
class StringResource;

/*!	@brief the SMUSH player class

	This class is the player itself. 
*/
class SmushPlayer {
private:
	char * _fname;			//!< the name of the animation file being played
	int _version;			//!< the version of the animation file being played
	int _secondaryVersion;	//!< the secondary version number of the animation file being played
	int _soundFrequency;		//!< the sound frequency of the animation file being played
	int _nbframes;			//!< the number of frames in the animation file
	Mixer * _mixer;			//!< the sound mixer
	Palette _pal;			//!< the current palette
	short _deltaPal[768];		//!< the delta palette information set by an xpal
	Renderer * _renderer;		//!< pointer to the ::renderer
	StringResource * _strings;	//!< pointer to the string resources associated with the animation
	FontRenderer * _fr[4];		//!< pointers to the fonts for the animation
	Codec1Decoder _codec1;	//!< the ::decoder for codec 1 and 3
	Codec37Decoder _codec37;	//!< the ::decoder for codec 37
	Codec44Decoder _codec44;	//!< the ::decoder for codec 21 and 44
	DumpDecoder _codecd;	//!< the ::decoder for codec 21 and 44
	Point _frameSize;		//!< the current frame size of the animation
	int _frame;				//!< the current frame number of the animation
	bool _outputSound;		//!< should we handle sound ?
	bool _wait;				//!< should we synchronise the player ?
	bool _alreadyInit;		//!< has the player already been initialized for the current frame
	bool _codec37Called;		//!< has the codec 37 already been called once for this animation
	bool _skipNext;			//!< should the player skip the next frame object ?
	bool _subtitles;			//!< should the player handle subtitles ?
	bool _bgmusic;			//!< should the player output the background music ?
	bool _voices;			//!< should the player output the voice ?
	bool _skips[37];			//!< mapping of frame object identifier to show or hide
	char * _curBuffer;		//!< pointer to the current frame
public:
	SmushPlayer(Renderer *, bool wait = true, bool output_sound = true);
	virtual ~SmushPlayer();
	bool play(const char *);
	void updatePalette(void);
	void show(const char *);
	void hide(const char *);
protected:
	bool readString(const char * file, bool &);
	void clean();
	void checkBlock(const Chunk &, Chunk::type, unsigned int = 0);
	void handleAnimHeader(Chunk &);
	void handleFrame(Chunk &);
	void handleNewPalette(Chunk &);
	void handleFrameObject(Chunk &);
	void handleSoundBuffer(int, int, int, int, int, int, Chunk &, int);
	void handleImuseBuffer(int, int, int, int, int, int, Chunk &, int);
	void handleSoundFrame(Chunk &);
	void handleSkip(Chunk &);
	void handleStore(Chunk &);
	void handleFetch(Chunk &);
	void handleImuseAction8(Chunk &, int flags, int unknown, int track_id);
	void handleImuseAction(Chunk &);
	void handleTextResource(Chunk &);
	void handleDeltaPalette(Chunk &);
	void decodeCodec(Chunk &, const Rect &, Decoder &);
	void readPalette(Palette &, Chunk &);
	void initSize(const Rect &, bool, bool);
};

#endif
