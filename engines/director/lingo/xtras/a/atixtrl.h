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

#ifndef DIRECTOR_LINGO_XTRAS_A_ATIXTRL_H
#define DIRECTOR_LINGO_XTRAS_A_ATIXTRL_H

namespace Director {

class AtixtrlXtraObject : public Object<AtixtrlXtraObject> {
public:
	AtixtrlXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace AtixtrlXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_atixLoadDriver(int nargs);
void m_atixUnloadDriver(int nargs);
void m_atixGetSource(int nargs);
void m_atixSetSource(int nargs);
void m_atixLowestChannel(int nargs);
void m_atixHighestChannel(int nargs);
void m_atixTuneToChannel(int nargs);
void m_atixTunerLocked(int nargs);
void m_atixSetBrightness(int nargs);
void m_atixSetContrast(int nargs);
void m_atixSetColor(int nargs);
void m_atixSetTint(int nargs);
void m_atixSetHAdjust(int nargs);
void m_atixSetVAdjust(int nargs);
void m_atixSetTracking(int nargs);
void m_atixSetSharpness(int nargs);
void m_atixSetColorTemp(int nargs);
void m_atixSetQuality(int nargs);
void m_atixPreparePreview(int nargs);
void m_atixPrepareOverlay(int nargs);
void m_atixPrepareCapture(int nargs);
void m_atixSetCropping(int nargs);
void m_atixSetPreviewRect(int nargs);
void m_atixUnprepare(int nargs);
void m_atixCaptureFrame(int nargs);
void m_atixConfigureDialog(int nargs);
void m_atixDriverInfoDialog(int nargs);
void m_atixSetVolumes(int nargs);
void m_atixGetLeftVolume(int nargs);
void m_atixGetRightVolume(int nargs);
void m_atixCanUseStereo(int nargs);
void m_atixSetSoundMix(int nargs);
void m_atixCanMixSound(int nargs);
void m_atixSetSoundSwitch(int nargs);
void m_atixCanSwitchSound(int nargs);
void m_atixSetMute(int nargs);
void m_atixGetAudioDrivers(int nargs);
void m_atixUseAudioDriver(int nargs);
void m_atixGetAudioConnectors(int nargs);
void m_atixUseAudioConnector(int nargs);
void m_atixGetConnectorRecordVolume(int nargs);
void m_atixSetConnectorRecordVolume(int nargs);
void m_"(int nargs);

} // End of namespace AtixtrlXtra

} // End of namespace Director

#endif
