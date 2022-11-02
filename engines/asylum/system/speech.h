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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_SYSTEM_SPEECH_H
#define ASYLUM_SYSTEM_SPEECH_H

#include "common/scummsys.h"

#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

class Speech {
public:
	Speech(AsylumEngine *engine);
	~Speech() {};

	/**
	 * Play speech
	 *
	 * @param soundResourceId The sound resource id.
	 * @param textResourceId  The text resource id.
	 */
	ResourceId play(ResourceId soundResourceId, ResourceId textResourceId);

	/**
	 *  Prepare speech to play
	 *   - Process sound speech and draws dialog in screen
	 */
	void prepareSpeech();

	/**
	 * Play speech by index offset.
	 *
	 * @param index The index offset.
	 *
	 * @return the ResourceId for the sound played
	 */
	ResourceId playIndexed(int32 index);

	/**
	 * Play scene speech.
	 *
	 * @param type   The type of speech to play.
	 * @param index  The index offset.
	 *
	 * @return the ResourceId for the sound played
	 */
	ResourceId playScene(int32 type, int32 index);

	/**
	 * Play speech by index offset (based on player type)
	 *
	 * @param index The index offset.
	 *
	 * @return the ResourceId for the sound played
	 */
	ResourceId playPlayer(int32 index);

	/**
	 * Resets the resource identifiers.
	 */
	void resetResourceIds();

	/**
	 * Resets text data
	 */
	void resetTextData();

	/**
	 * Gets the sound resource identifier.
	 *
	 * @return The sound resource identifier.
	 */
	ResourceId getSoundResourceId() const { return _soundResourceId; }

	/**
	 * Sets the tick.
	 *
	 * @param val The value.
	 */
	void setTick(int32 val) { _tick = val;}

	/**
	 * Gets the tick.
	 *
	 * @return The tick.
	 */
	uint32 getTick() { return _tick; }

	/**
	 * Sets the sound resource identifier
	 *
	 * @param id The sound resource identifier.
	 */
	void setSoundResourceId(ResourceId id) { _soundResourceId = id; }

	/**
	 * Sets the text resource identifier
	 *
	 * @param id The text resource identifier.
	 */
	void setTextResourceId(ResourceId id) { _textResourceId = id; }

	/**
	 * Gets the text resource identifier.
	 *
	 * @return The text resource identifier.
	 */
	ResourceId getTextResourceId() { return _textResourceId; }

	/**
	 * Sets text data.
	 *
	 * @param text The text.
	 */
	void setTextData(char *text) { _textData = text; }

	/**
	 * Get text data
	 *
	 * @return the text data
	 */
	char *getTextData() { return _textData; }

	/**
	 * Sets text data position.
	 *
	 * @param text The text.
	 */
	void setTextDataPos(char *text) { _textDataPos = text; }

	/**
	 * Get text data position
	 *
	 * @return the text data position
	 */
	char *getTextDataPos() { return _textDataPos; }

private:
	AsylumEngine *_vm;

	int32  _tick;
	char  *_textData;
	char  *_textDataPos;

	ResourceId _soundResourceId;
	ResourceId _textResourceId;

	/**
	 * Process speech and prepare for display
	 */
	void process();

};

} // end of namespace Asylum

#endif // ASYLUM_SYSTEM_SPEECH_H
