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

#ifndef MADS_MESSAGES_H
#define MADS_MESSAGES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/action.h"
#include "mads/font.h"
#include "mads/msurface.h"

namespace MADS {

#define KERNEL_MESSAGES_SIZE 10
#define INDEFINITE_TIMEOUT 9999999
#define TEXT_DISPLAY_SIZE 40
#define RANDOM_MESSAGE_SIZE 4

enum KernelMessageFlags {
	KMSG_QUOTED = 1, KMSG_PLAYER_TIMEOUT = 2, KMSG_SEQ_ENTRY = 4, KMSG_SCROLL = 8,
	KMSG_RIGHT_ALIGN = 0x10, KMSG_CENTER_ALIGN = 0x20, KMSG_EXPIRE = 0x40,
	KMSG_ACTIVE = 0x80, KMSG_ANIM = 0x100
};

class MADSEngine;

class KernelMessage {
public:
	uint16 _flags;
	int _sequenceIndex;
	int _color1;
	int _color2;
	Common::Point _position;
	int _textDisplayIndex;
	uint32 _msgOffset;
	int _numTicks;
	uint32 _frameTimer2;
	uint32 _frameTimer;
	int32 _timeout;
	int _trigger;
	TriggerMode _abortMode;
	ActionDetails _actionDetails;
	Common::String _msg;

	KernelMessage();
};

struct RandomEntry {
	int _handle;
	int _quoteId;

	RandomEntry() { _handle = _quoteId = -1; }
};

class RandomMessages : public Common::Array<RandomEntry> {
public:
	Common::Rect _bounds;
	int _randomSpacing;
	int _color;
	int _duration;
	int _scrollRate;
public:
	RandomMessages();

	void reset();
};

class KernelMessages {
private:
	MADSEngine *_vm;

	Common::Array<int> _randomQuotes;
	RandomMessages _randomMessages;
public:
	Common::Array<KernelMessage> _entries;
	Font *_talkFont;
public:
	KernelMessages(MADSEngine *vm);
	~KernelMessages();

	void clear();
	int add(const Common::Point &pt, uint fontColor, uint8 flags, int endTrigger,
		uint32 timeout, const Common::String &msg);
	int addQuote(int quoteId, int endTrigger, uint32 timeout);
	void scrollMessage(int msgIndex, int numTicks, bool quoted);
	void setSeqIndex(int msgIndex, int seqIndex);
	void setAnim(int msgId, int seqId, int val3);
	void remove(int msgIndex);
	void reset();
	void update();
	void processText(int msgIndex);
	void delay(uint32 priorFrameTime, uint32 currentTime);
	void setQuoted(int msgIndex, int numTicks, bool quoted);

	void initRandomMessages(int maxSimultaneousMessages,
		const Common::Rect &bounds, int minYSpacing, int scrollRate,
		int color, int duration, int quoteId, ...);

	/**
	 * Handles expiring any active random messages as necessary
	 */
	void randomServer();

	/**
	 * Return the number of currently active random messages
	 */
	int checkRandom();

	/**
	 * Handles generating new random messages
	 */
	bool generateRandom(int major, int minor);
};

class TextDisplay {
public:
	bool _active;
	int _expire;
	int _spacing;
	Common::Rect _bounds;
	uint8 _color1;
	uint8 _color2;
	Font *_font;
	Common::String _msg;

	TextDisplay();
};

#define TEXT_DISPLAY_SIZE 40

class TextDisplayList : public Common::Array<TextDisplay> {
private:
	MADSEngine *_vm;
public:
	TextDisplayList(MADSEngine *vm);

	/**
	 * Expire a given text display entry
	 */
	void expire(int idx);

	int add(int xp, int yp, uint fontColor, int charSpacing, const Common::String &, Font *font);

	/**
	 * Reset all of the text display entries in the list to inactive
	 */
	void reset();

	/**
	 * Draw any text in the list to the specified surface
	 * @param surface	Surface
	 */
	void draw(BaseSurface *s);

	/**
	 * Determine dirty areas for active text areas
	 */
	void setDirtyAreas();

	/**
	* Secondary setup dirty areas for the text areas
	*/
	void setDirtyAreas2();

	/**
	* Deactivates any text display entries that are finished
	*/
	void cleanUp();
};

} // End of namespace MADS

#endif /* MADS_MESSAGES_H */
