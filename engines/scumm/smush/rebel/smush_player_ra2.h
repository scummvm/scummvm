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

#if !defined(SCUMM_SMUSH_PLAYER_RA2_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_SMUSH_PLAYER_RA2_H

#include "scumm/smush/smush_player.h"

namespace Scumm {

class SmushPlayerRebel2 : public SmushPlayer {
public:
	SmushPlayerRebel2(ScummEngine_v7 *scumm, IMuseDigital *imuseDigital, Insane *insane);
	~SmushPlayerRebel2() override;
	bool ra2PromoteCurrentFrameToHiRes(int scrollX, int scrollY);

protected:
	void initGamePlayerFields() override;
	void destroyGamePlayerFields() override;
	void initGameVideoState() override;
	void releaseGameVideoState() override;
	bool shouldPreserveFrameBuffer() const override { return true; }
	// Override to keep the FOBJ parm2 field (codec 23 marker) the base reader discards.
	void handleFrameObject(int32 subSize, Common::SeekableReadStream &b) override;
	bool handleGameFetch(int32 subSize, Common::SeekableReadStream &b) override;
	bool handleGameTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &b) override;
	bool handleGameTextRendering(const char *str, int fontId, int color, int pos_x, int pos_y, int left, int top, int width, int height, TextStyleFlags flg) override;
	bool shouldAlwaysShowSubtitles() const override { return true; }
	SmushFont *getGameFont(int font) override;
	void adjustGamePalette() override;
	bool shouldLoadAnimHeaderPalette() const override;
	bool handleGameAnimHeader(byte *headerContent) override;
	bool handleGameSetupStrings() override;
	void handleGameParseNextFrame() override;
	bool shouldRouteAllIACTs() const override { return true; }
	bool handleGameFrameBufferSelect(int codec, int width, int height) override;
	bool handleGameDimensionOverride(int codec, int width, int height) override;
	int handleGameFrameObjectPitch(int pitch) override;
	bool handleGameAdjustCoords(int codec, int &left, int &top, int &width, int &height, int pitch, int *srcSkipY) override;
	bool handleGameCodecDecode(int codec, const uint8 *src, int left, int top, int width, int height, int pitch, int dataSize, uint8 param = 0, uint16 parm2 = 0) override;
	bool handleGameStoreFrame() override;
	void handleGameFrameObjectPre(int codec, int left, int top, int width, int height, int dataSize) override;
	void handleGameFrameObjectPost(int codec, const byte *data, int32 dataSize, int left, int top, int width, int height) override;
	void handleGameFrameStart() override;
	bool handleGameSkipChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b) override;
	void handleGameGost(int32 subSize, Common::SeekableReadStream &b) override;
	void handleGameProcessAudio(int16 feedSize) override;
	bool isInsaneGame() const override { return true; }
	void handleGameLoad(int32 subSize, Common::SeekableReadStream &b) override;

private:
	void handleLoad(int32 subSize, Common::SeekableReadStream &b);
	void ra2HandleTextResource(const char *str, int fontId, int color,
							   int pos_x, int pos_y, int left, int top,
							   int width, int height, TextStyleFlags flg);
	void ra2PrepareFrameObjectSurface(int left, int top, int width, int height);
	bool ra2SelectFrameBuffer(int codec, int width, int height);
	bool ra2EnsureLowResVideoBuffer();
	void ra2ClearCurrentTarget();
	bool ra2IsHighResMode() const;
	bool ra2DecodePlacedDeltaCodec(int codec, const uint8 *src, int left, int top,
								   int width, int height, int pitch, int dataSize);
	bool ra2DecodeCodec(int codec, const uint8 *src, int left, int top,
						int width, int height, int pitch, int dataSize);
	void ra2HandleDeltaPalette(int32 subSize, Common::SeekableReadStream &b);
	void ra2StoreFobjData(int codec, const byte *data, int32 dataSize,
						  int left, int top, int width, int height);
	void ra2HandleGost(int32 subSize, Common::SeekableReadStream &b);
	void ra2InitAudioTrackSizes();
	void ra2HandleFrameAudioChunk(uint32 subType, int32 subSize, Common::SeekableReadStream &b);
	void ra2FeedAudio(uint8 *srcBuf, int groupId, int volume, int pan, int16 flags);

	// LOAD chunk streaming buffer (embedded resource data)
	byte *_loadBuffer;
	int32 _loadBufferSize;
	int32 _loadBufferOffset;
	int32 _loadReadOffset;
	int16 _lastLoadChunkIdx;
	int16 _loadStreamId;
	int _ra2FrameSourceSkipX;
	int _ra2FrameSourceSkipY;
	int _ra2FrameObjectOriginalWidth;
	int _ra2FrameObjectOriginalHeight;
	int _ra2FrameObjectSurfaceWidth;
	int _ra2FrameObjectSurfaceHeight;
	int _ra2DeltaBlocksWidth;
	int _ra2DeltaBlocksHeight;
	int _ra2DeltaGlyphsWidth;
	int _ra2DeltaGlyphsHeight;
	byte *_ra2LowResVideoBuffer;
	int _ra2LowResVideoBufferSize;
	bool _ra2NativeFrameNeedsClear;
	bool _ra2UsingGameplaySurface;
	bool _ra2PendingAnimHeaderPalette;
	byte _ra2Codec45Palette[0x300];
	byte _ra2Codec45Lookup[0x8000];
	byte _ra2SkipRemapTable[256]; // codec 23 translucency remap table
	bool _ra2SkipRemapValid;
};

} // End of namespace Scumm

#endif
