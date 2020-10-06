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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef ENABLE_EOB

#ifndef KYRA_SEQPLAYER_SEGACD_H
#define KYRA_SEQPLAYER_SEGACD_H

namespace Common {
	class SeekableReadStream;
	class SeekableReadStreamEndian;
}

namespace Kyra {
class EoBEngine;
class Screen_EoB;
class SegaRenderer;
class SegaCDResource;
class SegaSequencePlayer {
public:
	SegaSequencePlayer(EoBEngine *vm, Screen_EoB *screen, SegaCDResource *res);
	~SegaSequencePlayer();

	bool play(int id);
	void pause(bool pause);

private:
	void run(const uint8 *data);

	void animateWaterdeepScene();
	void updateSpeechAnimations();
	void updateSpeechAnimGraphics(int animDrawOp);

	struct TileSet {
		const uint16 *data;
		uint16 width;
		uint16 height;
	};

	TileSet *_tileSets;

	struct DrawObject {
		uint16 agg;
		const uint16 *tileData;
		uint16 width;
		uint16 height;
		uint16 nTblVal;
		uint16 x;
		uint16 y;
		uint16 addr;
	};

	uint16 _waterdeepScene;
	uint16 _playSpeechAnimation;
	uint16 _speechAnimType;
	uint16 _speechAnimDrawOps[14];
	uint32 _frameTimer;
	uint32 _pauseStart;
	bool _fastForward;

	int _waterdeepSceneTimer, _speechAnimTimer;
	uint16 _speechAnimNo, _speechAnimFrame;
	int _playingID;
	int _newTrack;

	uint8 *_scaleSrcBuffer;
	uint8 *_scaleOutBuffer;
	uint16 *_scaleStampMap;
	uint16 *_scaleTraceVectors;

	uint32 _debugResyncCnt;

	DrawObject *_drawObjects;

	EoBEngine *_vm;
	Screen_EoB *_screen;
	SegaRenderer *_renderer;
	SegaAnimator *_animator;
	SegaCDResource *_res;
	ScrollManager *_scrollManager;

private:
	class SQOpcode : public Common::Functor1Mem<const uint8*, void, SegaSequencePlayer> {
	public:
		typedef Common::Functor1Mem<const uint8*, void, SegaSequencePlayer> SQFunc;
		SQOpcode(SegaSequencePlayer *sq, const SQFunc::FuncType &func, const char *dbgName) : SQFunc(sq, func), _msg(Common::String::format("    %s()", dbgName)) {}
		~SQOpcode() override {}
		void run(const uint8 *arg) {
			assert(arg);
			debugC(7, kDebugLevelSequence, "%s", _msg.c_str());
			if (SQFunc::isValid())
				SQFunc::operator()(arg);
		}
	private:
		Common::String _msg;
	};

	Common::Array<SQOpcode*> _opcodes;

	void s_initDrawObject(const uint8 *pos);
	void s_drawTileSet(const uint8 *pos);
	void s_loadTileDataSingle(const uint8 *pos);
	void s_drawTileSetCustom(const uint8 *pos);
	void s_drawTileSetCustomTopToBottom(const uint8 *pos);
	void s_fillRect(const uint8 *pos);
	void s_void(const uint8*) {}
	void s_initSprite(const uint8 *pos);
	void s_removeSprite(const uint8 *pos);
	void s_displayTextJp(const uint8 *pos);
	void s_fadeToNeutral(const uint8 *pos);
	void s_fadeToBlack(const uint8 *pos);
	void s_fadeToNeutral2(const uint8 *pos);
	void s_fadeToWhite(const uint8 *pos);
	void s_setPalette(const uint8 *pos);
	void s_vScroll(const uint8 *pos);
	void s_hScroll(const uint8 *pos);
	void s_paletteOps(const uint8 *pos);
	void s_initSpriteCustomCoords(const uint8 *pos);
	void s_fillRectWithPattern(const uint8 *pos);
	void s_loadTileDataSeries(const uint8 *pos);
	void s_drawTileSetSeries(const uint8 *pos);
	void s_initSpriteSeries(const uint8 *pos);
	void s_initSpriteCustom(const uint8 *pos);
	void s_drawTileSetCustomCoords(const uint8 *pos);
	void s_waitForPaletteFade(const uint8*);
	void s_clearSprites(const uint8*);
	void s_moveSprites2(const uint8 *pos);
	void s_moveSprites(const uint8 *pos);
	void s_moveMorphSprite(const uint8 *pos);
	void s_unpauseCD(const uint8 *pos);
	void s_toggleWaterDeepAnimations(const uint8 *pos);
	void s_assignSpeechAnimGraphics(const uint8 *pos);
	void s_toggleSpeechAnimation(const uint8 *pos);
	void s_orbZoomEffect(const uint8*);
	void s_stopCD(const uint8*);
	void s_playCD(const uint8 *pos);
	void s_displayTextEn(const uint8 *pos);
	void s_loadCustomPalettes(const uint8 *pos);
	void s_playSoundEffect(const uint8 *pos);

private:
	const uint8 *_cdaTracks;
	const uint8 *_wdAnimSprites;
	const uint8 *_speechAnimData;
	const uint16 *_wdDsX;
	const uint8 *_wdDsY;
	const uint16 *_patternTables[6];
};

} // End of namespace Kyra

#endif
#endif // ENABLE_EOB
