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

#ifndef DIRECTOR_LINGO_XTRAS_B_BASS_H
#define DIRECTOR_LINGO_XTRAS_B_BASS_H

namespace Director {

class BassXtraObject : public Object<BassXtraObject> {
public:
	BassXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace BassXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_xaBASS_GetConfig(int nargs);
void m_xaBASS_SetConfig(int nargs);
void m_xaBASS_ErrorGetCode(int nargs);
void m_xaBASS_Free(int nargs);
void m_xaBASS_GetCPU(int nargs);
void m_xaBASS_GetDevice(int nargs);
void m_xaBASS_GetDeviceDescription(int nargs);
void m_xaBASS_GetInfo(int nargs);
void m_xaBASS_GetVersion(int nargs);
void m_xaBASS_GetVolume(int nargs);
void m_xaBASS_Init(int nargs);
void m_xaBASS_Pause(int nargs);
void m_xaBASS_SetDevice(int nargs);
void m_xaBASS_SetVolume(int nargs);
void m_xaBASS_Start(int nargs);
void m_xaBASS_Stop(int nargs);
void m_xaBASS_Update(int nargs);
void m_xaBASS_Apply3D(int nargs);
void m_xaBASS_Get3DFactors(int nargs);
void m_xaBASS_Get3DPosition(int nargs);
void m_xaBASS_GetEAXParameters(int nargs);
void m_xaBASS_Set3DFactors(int nargs);
void m_xaBASS_Set3DPosition(int nargs);
void m_xaBASS_SetEAXParameters(int nargs);
void m_xaBASS_SampleFree	(int nargs);
void m_xaBASS_SampleGetInfo	object(int nargs);
void m_xaBASS_SampleLoad	object(int nargs);
void m_xaBASS_SamplePlay	object(int nargs);
void m_xaBASS_SamplePlay3D	object(int nargs);
void m_xaBASS_SamplePlay3DEx	object(int nargs);
void m_xaBASS_SamplePlayEx	object(int nargs);
void m_xaBASS_SampleSetInfo	object(int nargs);
void m_xaBASS_SampleStop	object(int nargs);
void m_xaBASS_StreamCreate(int nargs);
void m_xaBASS_StreamCreateFile(int nargs);
void m_xaBASS_StreamCreateURL(int nargs);
void m_xaBASS_StreamFree(int nargs);
void m_xaBASS_StreamGetFilePosition(int nargs);
void m_xaBASS_StreamGetLength(int nargs);
void m_xaBASS_StreamGetTags(int nargs);
void m_xaBASS_StreamPlay(int nargs);
void m_xaBASS_StreamPreBuf(int nargs);
void m_xaBASS_MusicFree	object(int nargs);
void m_xaBASS_MusicGetLength	object(int nargs);
void m_xaBASS_MusicGetName	object(int nargs);
void m_xaBASS_MusicGetVolume	object(int nargs);
void m_xaBASS_MusicLoad	object(int nargs);
void m_xaBASS_MusicPlay	object(int nargs);
void m_xaBASS_MusicPlayEx	object(int nargs);
void m_xaBASS_MusicPreBuf	object(int nargs);
void m_xaBASS_MusicSetAmplify	object(int nargs);
void m_xaBASS_MusicSetPanSep	object(int nargs);
void m_xaBASS_MusicSetPositionScaler	object(int nargs);
void m_xaBASS_MusicSetVolume	object(int nargs);
void m_xaBASS_RecordFree	object(int nargs);
void m_xaBASS_RecordGetDevice	object(int nargs);
void m_xaBASS_RecordGetDeviceDescription	object(int nargs);
void m_xaBASS_RecordGetInfo	object(int nargs);
void m_xaBASS_RecordGetInput	object(int nargs);
void m_xaBASS_RecordGetInputName	object(int nargs);
void m_xaBASS_RecordInit	object(int nargs);
void m_xaBASS_RecordSetDevice	object(int nargs);
void m_xaBASS_RecordSetInput	object(int nargs);
void m_xaBASS_RecordStart	object(int nargs);
void m_xaBASS_RecordStop(int nargs);
void m_xaBASS_RecordPlay(int nargs);
void m_xaBASS_RecordWrite(int nargs);
void m_xaBASS_ChannelBytes2Seconds(int nargs);
void m_xaBASS_ChannelGet3DAttributes(int nargs);
void m_xaBASS_ChannelGet3DPosition(int nargs);
void m_xaBASS_ChannelGetAttributes(int nargs);
void m_xaBASS_ChannelGetData(int nargs);
void m_xaBASS_ChannelGetDevice(int nargs);
void m_xaBASS_ChannelGetEAXMix(int nargs);
void m_xaBASS_ChannelGetInfo(int nargs);
void m_xaBASS_ChannelGetLevel(int nargs);
void m_xaBASS_ChannelGetPosition(int nargs);
void m_xaBASS_ChannelIsActive(int nargs);
void m_xaBASS_ChannelIsSliding(int nargs);
void m_xaBASS_ChannelPause(int nargs);
void m_xaBASS_ChannelRemoveFX(int nargs);
void m_xaBASS_ChannelRemoveLink(int nargs);
void m_xaBASS_ChannelResume(int nargs);
void m_xaBASS_ChannelSeconds2Bytes(int nargs);
void m_xaBASS_ChannelSet3DAttributes(int nargs);
void m_xaBASS_ChannelSet3DPosition(int nargs);
void m_xaBASS_ChannelSetAttributes(int nargs);
void m_xaBASS_ChannelSetEAXMix(int nargs);
void m_xaBASS_ChannelSetFX(int nargs);
void m_xaBASS_ChannelSlideAttributes(int nargs);
void m_xaBASS_ChannelSetLink(int nargs);
void m_xaBASS_ChannelSetMusicPosition(int nargs);
void m_xaBASS_ChannelSetPosition(int nargs);
void m_xaBASS_ChannelStop(int nargs);
void m_xaBASS_FXGetParameters(int nargs);
void m_xaBASS_FXSetParameters(int nargs);
void m_xaClose(int nargs);
void m_xaRegister(int nargs);
void m_"(int nargs);

} // End of namespace BassXtra

} // End of namespace Director

#endif
