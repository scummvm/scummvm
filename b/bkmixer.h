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

#ifndef DIRECTOR_LINGO_XTRAS_B_BKMIXER_H
#define DIRECTOR_LINGO_XTRAS_B_BKMIXER_H

namespace Director {

class BkmixerXtraObject : public Object<BkmixerXtraObject> {
public:
	BkmixerXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BkmixerXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_bkMixerVersion(int nargs);
void m_bkGetMasterVolume(int nargs);
void m_bkGetCDVolume(int nargs);
void m_bkGetWaveVolume(int nargs);
void m_bkGetMidiVolume(int nargs);
void m_bkGetLineInVolume(int nargs);
void m_bkGetMicVolume(int nargs);
void m_bkGetSpeakerVolume(int nargs);
void m_bkSetMasterVolume(int nargs);
void m_bkSetCDVolume(int nargs);
void m_bkSetWaveVolume(int nargs);
void m_bkSetMidiVolume(int nargs);
void m_bkSetLineInVolume(int nargs);
void m_bkSetMicVolume(int nargs);
void m_bkSetSpeakerVolume(int nargs);
void m_bkIsMasterMute(int nargs);
void m_bkIsCDMute(int nargs);
void m_bkIsWaveMute(int nargs);
void m_bkIsMidiMute(int nargs);
void m_bkIsLineInMute(int nargs);
void m_bkIsMicMute(int nargs);
void m_bkIsSpeakerMute(int nargs);
void m_bkMuteMaster(int nargs);
void m_bkMuteCD(int nargs);
void m_bkMuteWave(int nargs);
void m_bkMuteMidi(int nargs);
void m_bkMuteLineIn(int nargs);
void m_bkMuteMic(int nargs);
void m_bkMuteSpeaker(int nargs);
void m_bkDetectWaveOut(int nargs);
void m_bkGetWaveOutName(int nargs);
void m_bkDetectMixer(int nargs);
void m_bkGetMixerName(int nargs);
void m_bkGetMasterVolumeEx(int nargs);
void m_bkGetCDVolumeEx(int nargs);
void m_bkGetWaveVolumeEx(int nargs);
void m_bkGetMidiVolumeEx(int nargs);
void m_bkGetLineInVolumeEx(int nargs);
void m_bkGetMicVolumeEx(int nargs);
void m_bkGetSpeakerVolumeEx(int nargs);
void m_bkSetMasterVolumeEx(int nargs);
void m_bkSetCDVolumeEx(int nargs);
void m_bkSetWaveVolumeEx(int nargs);
void m_bkSetMidiVolumeEx(int nargs);
void m_bkSetLineInVolumeEx(int nargs);
void m_bkSetMicVolumeEx(int nargs);
void m_bkSetSpeakerVolumeEx(int nargs);
void m_"(int nargs);

} // End of namespace BkmixerXtra

} // End of namespace Director

#endif
