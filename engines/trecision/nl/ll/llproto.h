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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TRECISION_LLPROTO_H
#define TRECISION_LLPROTO_H

#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Trecision {

int Compare(const void *p1, const void *p2);
void RedrawRoom();
void decompress(const uint8 *src, unsigned int src_len, uint8 *dst, unsigned int dst_len);
char waitKey();
void FreeKey();
char *GetNextSent();
void Mouse(uint8 opt);
uint16 TextLength(const char *sign, uint16 num);
void IconSnapShot();
bool DataSave();
bool DataLoad();
bool QuitGame();
void DemoOver();
void openSys();
void ReadLoc();
void TendIn();
void ReadObj();
void ReadExtraObj2C();
void ReadExtraObj41D();
void ReadSounds();
void RegenRoom();
void PaintRegenRoom();
void DrawObj(SDObj d);
void StopSoundSystem();
void LoadAudioWav(int num, Common::String fileName);
void NLPlaySound(int num);
void NLStopSound(int num);
void SoundFadOut();
void SoundFadIn(int num);
void SoundStopAll();
void WaitSoundFadEnd();
void SoundPasso(int midx, int midz, int act, int frame, uint16 *list);
int32 Talk(const char *name);
void StopTalk();
void resetZBuffer(int x1, int y1, int x2, int y2);
uint32 ReadTime();
void NlDelay(uint32 val);
void NlDissolve(int val);
bool CheckMask(uint16 mx, uint16 my);
void byte2word(void *dest, void *src, void *data, uint32 len);
void byte2wordm(void *dest, void *src, void *data, uint32 len);
void byte2wordn(void *dest, void *src, void *data, uint32 len);

} // End of namespace Trecision

#endif
