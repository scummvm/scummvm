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

#ifndef DIRECTOR_LINGO_XTRAS_B_BEATNIK_H
#define DIRECTOR_LINGO_XTRAS_B_BEATNIK_H

namespace Director {

class BeatnikXtraObject : public Object<BeatnikXtraObject> {
public:
	BeatnikXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BeatnikXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_setPreferredSoundChannel(int nargs);
void m_getSampleLibrary(int nargs);
void m_setSampleLibrary(int nargs);
void m_getSampleRate(int nargs);
void m_setSampleRate(int nargs);
void m_setSliceSize(int nargs);
void m_setReady(int nargs);
void m_closeBeatnik(int nargs);
void m_isReady(int nargs);
void m_onMetaEvent(int nargs);
void m_onStop(int nargs);
void m_play(int nargs);
void m_playGroovoid(int nargs);
void m_stop(int nargs);
void m_pause(int nargs);
void m_setLoop(int nargs);
void m_setAutoStart(int nargs);
void m_setGlobalMute(int nargs);
void m_fadeTo(int nargs);
void m_fadeFromTo(int nargs);
void m_playNote(int nargs);
void m_noteOn(int nargs);
void m_noteOff(int nargs);
void m_getNoteName(int nargs);
void m_getNoteNumber(int nargs);
void m_setVolume(int nargs);
void m_setEnableTranspose(int nargs);
void m_setTranspose(int nargs);
void m_setReverbType(int nargs);
void m_setTempo(int nargs);
void m_setController(int nargs);
void m_setProgram(int nargs);
void m_setTrackMute(int nargs);
void m_setChannelMute(int nargs);
void m_setTrackSolo(int nargs);
void m_setChannelSolo(int nargs);
void m_setPosition(int nargs);
void m_setStartTime(int nargs);
void m_setEndTime(int nargs);
void m_getLoop(int nargs);
void m_getAutoStart(int nargs);
void m_getGlobalMute(int nargs);
void m_getReverbType(int nargs);
void m_getTempo(int nargs);
void m_getEnableTranspose(int nargs);
void m_getTranspose(int nargs);
void m_getController(int nargs);
void m_getProgram(int nargs);
void m_getVolume(int nargs);
void m_getTrackMute(int nargs);
void m_getChannelMute(int nargs);
void m_getTrackSolo(int nargs);
void m_getChannelSolo(int nargs);
void m_getPosition(int nargs);
void m_getPlayLength(int nargs);
void m_isPaused(int nargs);
void m_isPlaying(int nargs);
void m_getInfo(int nargs);
void m_getErrorString(int nargs);
void m_getVersion(int nargs);
void m_register(int nargs);
void m_"(int nargs);

} // End of namespace BeatnikXtra

} // End of namespace Director

#endif
