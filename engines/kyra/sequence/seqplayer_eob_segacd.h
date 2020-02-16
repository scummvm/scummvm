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
	void setWaitFlag(bool enable);

private:
	void run(const uint8 *data);
	void setVScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB);
	void setHScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB);
	void updateScrollTimers();
	void animateWaterdeepScene();
	void update2();
	void update3();

	struct TileSet {
		const uint16 *data;
		uint16 width;
		uint16 height;
	};

	TileSet *_tileSets;

	struct ScrollTimer {
		ScrollTimer() : _offsCur(0), _offsDest(0), _incr(0), _delay(0), _timer(0) {}
		uint16 _offsCur;
		uint16 _offsDest;
		int16 _incr;
		int16 _delay;
		int16 _timer;
	};

	ScrollTimer *_vScrollTimers;
	ScrollTimer *_hScrollTimers;

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

	bool _var1;
	uint16 _waterdeepScene;
	uint16 _update2;
	uint16 _varUnkX1;
	uint16 _varUnkX2[12];

	bool _waitFlag;
	int _waterdeepSceneTimer, _unkSEQ2;
	uint8 *_tempBuffer;

	uint32 _debugResyncCnt;

	DrawObject *_drawObjects;

	EoBEngine *_vm;
	Screen_EoB *_screen;
	SegaRenderer *_renderer;
	SegaAnimator *_animator;
	SegaCDResource *_res;

	const uint16 *_wdDsX;
	const uint8 *_wdDsY;

private:
	class SQOpcode : public Common::Functor1Mem<const uint8*, void, SegaSequencePlayer> {
	public:
		typedef Common::Functor1Mem<const uint8*, void, SegaSequencePlayer> SQFunc;
		SQOpcode(SegaSequencePlayer *sq, const typename SQFunc::FuncType &func, const char *dbgName) : SQFunc(sq, func), _msg(Common::String::format("    %s()", dbgName)) {}
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
	void s_3(const uint8 *pos);
	void s_4(const uint8 *pos);
	void s_fillRect(const uint8 *pos);
	void s_6(const uint8 *pos);
	void s_7(const uint8 *pos);
	void s_8(const uint8 *pos);
	void s_9_dispText(const uint8 *pos);
	void s_fadeToNeutral(const uint8 *pos);
	void s_fadeToBlack(const uint8 *pos);
	void s_fadeToNeutral2(const uint8 *pos);
	void s_fadeToWhite(const uint8 *pos);
	void s_setPalette(const uint8 *pos);
	void s_vScroll(const uint8 *pos);
	void s_hScroll(const uint8 *pos);
	void s_paletteOps(const uint8 *pos);
	void s_initSprite(const uint8 *pos);
	void s_fillRectWithPattern(const uint8 *pos);
	void s_loadTileDataMult(const uint8 *pos);
	void s_21(const uint8 *pos);
	void s_22(const uint8 *pos);
	void s_initSprite2(const uint8 *pos);
	void s_drawTileSetCustom(const uint8 *pos);
	void s_waitForPaletteFade(const uint8*);
	void s_clearSprites(const uint8*);
	void s_27(const uint8 *pos);
	void s_moveSprites(const uint8 *pos);
	void s_moveMorphSprite(const uint8 *pos);
	void s_unpauseCD(const uint8 *pos);
	void s_enableWaterDeepAnimations(const uint8 *pos);
	void s_32(const uint8 *pos);
	void s_setUpdate2(const uint8 *pos);
	void s_orbEffect(const uint8*);
	void s_stopCD(const uint8*);
	void s_playCD(const uint8 *pos);
	void s_displayText(const uint8 *pos);
	void s_loadCustomPalettes(const uint8 *pos);
	void s_playSoundEffect(const uint8 *pos);

private:
	static const uint8 _cdaTracks[60];
};

} // End of namespace Kyra

#endif
