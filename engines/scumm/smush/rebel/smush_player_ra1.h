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

#if !defined(SCUMM_SMUSH_PLAYER_RA1_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_SMUSH_PLAYER_RA1_H

#include "scumm/smush/smush_player.h"

namespace Scumm {

class SmushPlayerRebel1 : public SmushPlayer {
	friend class InsaneRebel1;
public:
	SmushPlayerRebel1(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane);
	~SmushPlayerRebel1() override;

protected:
	void initGamePlayerFields() override;
	void destroyGamePlayerFields() override;
	void resetGameVideoState() override;
	void initGameVideoState() override;
	void releaseGameVideoState() override;
	bool handleGameFetch(int32 subSize, Common::SeekableReadStream &b) override;
	bool handleGameTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &b) override;
	SmushFont *getGameFont(int font) override;
	void adjustGamePalette() override;
	bool handleGameAnimHeader(byte *headerContent) override;
	void handleGameParseNextFrame() override;
	bool handleGameFrameBufferSelect(int codec, int width, int height) override;
	bool handleGameDimensionOverride(int codec, int width, int height) override;
	bool handleGameAdjustCoords(int codec, int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) override;
	bool handleGameCodecDecode(int codec, const uint8 *src, int left, int top, int width, int height, int pitch, int dataSize, uint8 param = 0, uint16 parm2 = 0) override;
	bool handleGameStoreFrame() override;
	void handleGameFrameObjectPre(int codec, int left, int top, int width, int height, int dataSize) override;
	void handleGameFrameObjectPost(int codec, const byte *data, int32 dataSize, int left, int top, int width, int height) override;
	void handleGameFrameStart() override;
	void handleGameGost(int32 subSize, Common::SeekableReadStream &b) override;
	void handleGameProcessAudio(int16 feedSize) override;
	bool isInsaneGame() const override { return true; }
	void handleFrameObject(int32 subSize, Common::SeekableReadStream &b) override;
	void handleFrame(int32 frameSize, Common::SeekableReadStream &b) override;
	void handleGameUpdateScreen(const byte *src, int srcPitch, int width, int height) override;

private:
	void ra1HandleGost(int32 subSize, Common::SeekableReadStream &b);
	void ra1HandleDeltaPalette(int32 subSize, Common::SeekableReadStream &b);
	void ra1HandleFade(int32 subSize, Common::SeekableReadStream &b);
	SmushFont *ra1GetFont(int font);
	void ra1HandleText(int32 subSize, Common::SeekableReadStream &b);
	void ra1HandleFrameAudioChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b);
	void ra1FeedAudio(uint32 subType, uint8 *srcBuf, int groupId, int volume, int pan, int16 flags);
	void ra1HandleGameFrameChunk(int32 subSize, Common::SeekableReadStream &b, bool fastForwarding);
	void ra1HandleObjOverlayFrameChunk(int32 objDataSize, Common::SeekableReadStream &b, bool fastForwarding);
	bool ra1HandleUnknownFrameChunk(uint32 subType, int32 subSize);
	bool ra1DispatchFrameChunk(uint32 subType, int32 subSize, int32 &frameSize,
		Common::SeekableReadStream &b, bool fastForwarding);
	void ra1InitAudioTrackSizes();

	// RA1 clean frame buffer for delta source restoration
	byte *_ra1CleanFrame;
	int32 _ra1CleanFrameSize;
	bool _ra1HasCleanFrame;

	// RA1 interactive movies present a 312x192 viewport inside a black 320x200 frame.
	byte *_ra1PresentationBuffer;
	int32 _ra1PresentationBufferSize;

	// RA1 OBJ overlay FOBJ — cockpit drawn once frame 0, re-rendered every frame
	byte *_ra1ObjOverlayData;
	int32 _ra1ObjOverlayDataSize;
	int _ra1ObjOverlayCodec;
	int _ra1ObjOverlayLeft;
	int _ra1ObjOverlayTop;
	int _ra1ObjOverlayWidth;
	int _ra1ObjOverlayHeight;

	// RA1 viewport scroll offset for interactive gameplay
	int _ra1ViewportOffsetX;
	int _ra1ViewportOffsetY;
	int _ra1FrameSourceSkipY;
	bool _ra1LastFrameObjectVisible;

	// RA1 FADE chunks update the visible 320x200 screen through a sparse
	// copy mask, separate from the decoded frame buffer.
	byte *_ra1FadeFrame;
	int32 _ra1FadeFrameSize;
	int _ra1FadeFrameWidth;
	int _ra1FadeFrameHeight;
	bool _ra1FadeFrameValid;
	bool _ra1UseFadeFrame;
};

} // End of namespace Scumm

#endif
