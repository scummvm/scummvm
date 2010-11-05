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

class Scene;

class Speech {
public:
	Speech(Scene *scene);
	virtual ~Speech();

	ResourceId _soundResourceId;
	ResourceId _textResourceId;

	/** .text:00414810
	 *  Play player speech
	 *  @param speechIdx speech index (used for sound and text resources)
	 *  @return correct resourceId
	 */
	int32 play(ResourceId speechResourceId);
	/** .text:004146D0
	 *  Set speech for different player type
	 *  @param soundResourceId sound resource id
	 *  @param textResourceId text resource id
	 */
	void setPlayerSpeech(ResourceId soundResourceId, ResourceId textResourceId);
	/** .text:004144C0
	 *  Prepare speech to play
	 *   - Process sound speech and draws dialogue in screen
	 */
	void prepareSpeech();

private:
	Scene *_scene;

	int32  _tick;
	char  *_textData;
	char  *_textDataPos;

	/** .text:00414580
	 * TODO add description
	 */
	void processSpeech();

	// This function was cutoff since it doesn't make any sense using it. Its here for address information only
	/** .text:00414630 void playSpeech(ResourceId textResourceId, ResourceId fontResourceId); */

}; // end of class Speech

} // end of namespace Asylum

#endif
