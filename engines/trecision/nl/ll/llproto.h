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

namespace Trecision {

int Compare(const void *p1, const void *p2);
char AnimFileInit(const char *fname);
void AnimFileFinish();
int AnimFileOpen(const char *name);
int FmvFileOpen(const char *name);
char SpeechFileInit(const char *fname);
void SpeechFileFinish();
int SpeechFileLen(const char *name);
int SpeechFileRead(const char *name, uint8 *buf);
void BCopy(uint32 Sco, uint8 *Src, uint32 Len);
void DCopy(uint32 Sco, uint8 *Src, uint32 Len);
void StartSmackAnim(uint16 num);
void StopSmackAnim(uint16 num);
void StopAllSmackAnims();
void CallSmackVolumePan(int buf, int track, int vol);
void CallSmackGoto(int buf, int num);
void CallSmackSoundOnOff(int pos, int on);
void RegenAnim(int box);
void RegenSmackAnim(int num);
void RegenSmackIcon(int StartIcon, int num);
void RedrawRoom();
void *smkmalloc(unsigned int size);
void smkfree(void *ptr);
void decompress(const uint8 *src, unsigned int src_len, uint8 *dst, unsigned int dst_len);
uint32 DecXPK(const char *ArcName, const char *FileName, uint8 *DestArea, uint8 *DecArea);
void setbiosisr();
void setrawisr();
void installlowisr();
char waitKey();
char ReadKey();
void FreeKey();
char *GetNextSent();
void SaveAll();
void LoadAll();
uint16 vr(int16 x, int16 y);
void VPix(int16 x, int16 y, uint16 col);
void VMouseOFF();
void VMouseON();
void VMouseRestore();
void VMouseCopy();
void Mouse(uint8 opt);
uint16 TextLength(const char *sign, uint16 num);
uint16 CheckDText(struct SDText t);
void DText(struct SDText t);
void IntroText(int16 x, int16 y, const char *sign, uint8 *buf);
void IconSnapShot();
int16 DataSave();
int16 DataLoad();
int16 QuitGame();
void DemoOver();
void CheckFileInCD(const char *name);
void openSys();
void OpenVideo();
uint32 ReadActor(const char *filename, uint8 *Area);
void CloseSys(const char *str);
void ReadLoc();
void TendIn();
void ReadObj();
void ReadExtraObj2C();
void ReadExtraObj41D();
void ReadAnims();
void ReadSounds();
void RegenRoom();
void PaintRegenRoom();
void DrawObj(struct SDObj d);
void VisualFrame(uint16 CurAnim);
void KillFrame(uint16 CurAnim);
void RegenInventory(uint8 StartIcon, uint8 StartLine);
void RGBColor(uint16 a, uint8 *r, uint8 *g, uint8 *b);
uint16 RGB2Color(uint8 r, uint8 g, uint8 b);
uint16 ULaw2Linear(uint8 ulawbyte);
void GrabVideo();
void SaveAction();
void InitScrollPage(int flag);
void VisualScrollPage(int dx);
void ScrollPage(int dx);
void AlternativeScrolling(int dx);
void SetHeadTexture(uint8 num);
void SetTexture(int flag);
void StartSoundSystem();
void StopSoundSystem();
void LoadSound(int16 num, uint8 *Buffer);
int16 LoadAudioWav(int num, uint8 *Buffer, int len);
void NLPlaySound(int num);
void NLStopSound(int num);
void SoundFadOut();
void SoundFadIn(int num);
void WaitSoundFadEnd();
void SoundPasso(int midx, int midz, int act, int frame, uint16 *list);
void SetBackgroundVolume(int val);
void serve_sample();
void ContinueTalk();
int32 Talk(const char *name);
void StopTalk();
void DetectVesaMode(unsigned int ModeNumber);
void SetVesaMode(unsigned int ModeNumber);
void SetVesaLineLength(unsigned int Length);
void SetVesaStartVisualPage(unsigned int StartX, unsigned int StartY);
void ResetZB(int x1, int y1, int x2, int y2);
void SetVesaPage(unsigned int PageNumber);
void VCopy(uint32 Sco, uint16 *Src, uint32 Len);
void MCopy(uint16 *Dest, uint16 *Src, uint32 Len);
void ShowScreen(int, int, int, int);
void UpdatePixelFormat(uint16 *p, uint32 len);
uint16 UnUpdatePixelFormat(uint16  t);
void ClearScreen();
uint32 ReadTime();
void NlDelay(uint32 val);
void NlDissolve(int val);
uint32 TrecRnd(uint32 t);
int16 CheckMask(uint16 MX, uint16 my);
uint8 CheckMaskBox(uint16 mx, uint16 my);
void WaitCop(uint8 num);
void LockVideo();
void UnlockVideo();
void StartFullMotion(const char *name);
void StopFullMotion();
void RegenFullMotion();
char FastFileInit(const char *fname);
void FastFileFinish();
struct SFileHandle *FastFileOpen(const char *name);
char FastFileClose(struct SFileHandle *pfh);
int FastFileRead(struct SFileHandle *pfh, void *ptr, int size);
char FastFileSeek(struct SFileHandle *pfh, int off, int how);
int32 FastFileTell(struct SFileHandle *pfh);
int32 FastFileLen(struct SFileHandle *pfh);
uint16 PalTo16bit(uint8 r, uint8 g, uint8 b);
void wordcopy(void *dest, void *src, uint32 len);
void longcopy(void *dest, void *src, uint32 len);
void wordset(void *dest, uint16 value, uint32 len);
void longset(void *dest, uint32 value, uint32 len);
void byte2word(void *dest, void *src, void *data, uint32 len);
void byte2long(void *dest, void *src, void *data, uint32 len);
void byte2wordm(void *dest, void *src, void *data, uint32 len);
void byte2wordn(void *dest, void *src, void *data, uint32 len);
void byte2longm(void *dest, void *src, void *data, uint32 len);

} // End of namespace Trecision
