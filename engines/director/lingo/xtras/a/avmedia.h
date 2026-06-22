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

#ifndef DIRECTOR_LINGO_XTRAS_A_AVMEDIA_H
#define DIRECTOR_LINGO_XTRAS_A_AVMEDIA_H

namespace Director {

class AvmediaXtraObject : public Object<AvmediaXtraObject> {
public:
	AvmediaXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace AvmediaXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_audioOpen(int nargs);
void m_audioClose(int nargs);
void m_audioPlay(int nargs);
void m_audioPause(int nargs);
void m_audioGetDuration(int nargs);
void m_audioGetCurrentTime(int nargs);
void m_audioSetCurrentTime(int nargs);
void m_audioGetPan(int nargs);
void m_audioSetPan(int nargs);
void m_audioGetRate(int nargs);
void m_audioSetRate(int nargs);
void m_audioGetVolume(int nargs);
void m_audioSetVolume(int nargs);
void m_audioFileConvert(int nargs);
void m_videoOpen(int nargs);
void m_videoClose(int nargs);
void m_videoGetCurrentTime(int nargs);
void m_videoGetDuration(int nargs);
void m_videoGetFPS(int nargs);
void m_videoGetImage(int nargs);
void m_videoGetNaturalSize(int nargs);
void m_videoGetRate(int nargs);
void m_videoGetReadyForDisplay(int nargs);
void m_videoGetRect(int nargs);
void m_videoGetTimeScale(int nargs);
void m_videoGetTrackList(int nargs);
void m_videoGetVolume(int nargs);
void m_videoSetCurrentTime(int nargs);
void m_videoSetLoop(int nargs);
void m_videoSetOverlay(int nargs);
void m_videoSetOverlayVisibility(int nargs);
void m_videoSetParentWindow(int nargs);
void m_videoSetRate(int nargs);
void m_videoSetRect(int nargs);
void m_videoSetTrackEnabled(int nargs);
void m_videoSetVolume(int nargs);
void m_videoSetWindowOrder(int nargs);
void m_videoStepByCount(int nargs);
void m_videoSyncToAudio(int nargs);
void m_videoToggleFullScreen(int nargs);
void m_videoSetDoubleClickCallback(int nargs);
void m_videoSetMouseDownCallback(int nargs);
void m_videoSetReadyStatusChangeCallback(int nargs);
void m_videoSetWindowClosedCallback(int nargs);
void m_videoFileConvert(int nargs);
void m_videoSetFileConvertCallback(int nargs);

} // End of namespace AvmediaXtra

} // End of namespace Director

#endif
