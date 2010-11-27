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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SPEECH_H
#define ASYLUM_SPEECH_H

#include "asylum/shared.h"

#include "common/scummsys.h"

namespace Asylum {

class AsylumEngine;

class Speech {
public:
	Speech(AsylumEngine *engine);
	virtual ~Speech();

	/**
	 * Play speech
	 *
	 * @param soundResourceId The sound resource id.
	 * @param textResourceId  The text resource id.
	 */
	ResourceId play(ResourceId soundResourceId, ResourceId textResourceId);

	/**
	 *  Prepare speech to play
	 *   - Process sound speech and draws dialogue in screen
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
	int32 getTick() { return _tick; }

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

}; // end of class Speech

} // end of namespace Asylum

#endif
