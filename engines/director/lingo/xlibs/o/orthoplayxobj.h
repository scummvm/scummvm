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

#ifndef DIRECTOR_LINGO_XLIBS_ORTHOPLAYXOBJ_H
#define DIRECTOR_LINGO_XLIBS_ORTHOPLAYXOBJ_H

namespace Director {

class OrthoPlayXObject : public Object<OrthoPlayXObject> {
public:
	OrthoPlayXObject(ObjectType objType);
};

namespace OrthoPlayXObj {

extern const char *const xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_setSerialPort(int nargs);
void m_setInitViaDlog(int nargs);
void m_getInitInfo(int nargs);
void m_setInitInfo(int nargs);
void m_getMaxDevices(int nargs);
void m_getDeviceTitle(int nargs);
void m_setDevice(int nargs);
void m_selectDevice(int nargs);
void m_getDevice(int nargs);
void m_service(int nargs);
void m_getValue(int nargs);
void m_cancel(int nargs);
void m_explain(int nargs);
void m_idle(int nargs);
void m_readStatus(int nargs);
void m_readPos(int nargs);
void m_searchTo(int nargs);
void m_play(int nargs);
void m_still(int nargs);
void m_stop(int nargs);
void m_scanForward(int nargs);
void m_scanReverse(int nargs);
void m_playReverse(int nargs);
void m_fastForward(int nargs);
void m_rewind(int nargs);
void m_stepForward(int nargs);
void m_stepReverse(int nargs);
void m_shuttle(int nargs);
void m_record(int nargs);
void m_eject(int nargs);
void m_prepareMedium(int nargs);
void m_getFirstTrack(int nargs);
void m_getLastTrack(int nargs);
void m_getFirstFrame(int nargs);
void m_getLastFrame(int nargs);
void m_getTrack(int nargs);
void m_resetCounter(int nargs);
void m_audioEnable(int nargs);
void m_audioMute(int nargs);
void m_videoEnable(int nargs);
void m_showFrame(int nargs);
void m_getFrameResolution(int nargs);
void m_setFrameResolution(int nargs);
void m_hasDropFrames(int nargs);
void m_sendRaw(int nargs);
void m_readRaw(int nargs);
void m_setInPoint(int nargs);
void m_setOutPoint(int nargs);
void m_setDuration(int nargs);
void m_getMinDuration(int nargs);
void m_setPreroll(int nargs);
void m_getPreroll(int nargs);
void m_setPostroll(int nargs);
void m_getPostroll(int nargs);
void m_setFieldDominance(int nargs);
void m_playCue(int nargs);
void m_playSegment(int nargs);
void m_recordCue(int nargs);
void m_recordSegment(int nargs);
void m_recordVideoEnable(int nargs);
void m_recordAudioEnable(int nargs);
void m_assembleRecord(int nargs);
void m_previewRecord(int nargs);
void m_gotoInPoint(int nargs);
void m_gotoOutPoint(int nargs);
void m_gotoPrerollPoint(int nargs);
void m_gotoPostrollPoint(int nargs);

} // End of namespace OrthoPlayXObj

} // End of namespace Director

#endif
