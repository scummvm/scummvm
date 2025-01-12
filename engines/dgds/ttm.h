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

#ifndef DGDS_TTM_H
#define DGDS_TTM_H

#include "dgds/scripts.h"
#include "dgds/font.h"

namespace Dgds {

class SoundRaw;

class GetPutRegion {
public:
	Common::Rect _area;
	Common::SharedPtr<Graphics::ManagedSurface> _surf;

	void reset();
};

class TTMEnviro : public ScriptParserData {
public:
	TTMEnviro() : _totalFrames(330), _enviro(0), _creditScrollMeasure(0),
			_creditScrollYOffset(0), _xOff(0), _yOff(0), _xScroll(0), _yScroll(0),
			ScriptParserData() {
		ARRAYCLEAR(_scriptPals);
	}

	Common::Error syncState(Common::Serializer &s);

	uint16 _enviro;
	uint16 _totalFrames;
	Common::Array<int> _frameOffsets;
	Common::SharedPtr<Image> _scriptShapes[6];
	Common::Array<GetPutRegion> _getPuts;
	int _scriptPals[6];
	Common::String _strings[10];
	Common::Array<FontManager::FontType> _fonts;
	int16 _creditScrollMeasure;
	int16 _creditScrollYOffset;
	// The below are all globals in the original, but never get access from
	// multiple environments so cleaner to keep here?
	int16 _xOff;
	int16 _yOff;
	Common::SharedPtr<Image> _scrollShape;
	int16 _xScroll;
	int16 _yScroll;
	Common::SharedPtr<SoundRaw> _soundRaw;
};

enum TTMRunType {
	kRunTypeStopped = 0,
	kRunTypeKeepGoing = 1,
	kRunTypeMulti = 2,
	kRunTypeTimeLimited = 3,
	kRunTypeFinished = 4,
	kRunTypePaused = 5,
};


// Note: this object needs to be safely copy-able - ADS opcodes 0x4000 and 0x4010 require it.
struct TTMSeq {
	TTMSeq() : _enviro(0), _seqNum(0), _startFrame(0), _lastFrame(0), _timeCut(0),
		_currentBmpId(0), _currentGetPutId(0) {
		// Other members are initialized in the reset function.
		reset();
	}

	void reset();
	Common::Error syncState(Common::Serializer &s);

	int16 _enviro;
	int16 _seqNum;
	int16 _startFrame;	// first frame in this sequence
	int16 _gotoFrame;   // frame to GOTO (or -1 if not currently set)
	int16 _currentFrame; // currently executing frame
	int16 _lastFrame;	// previous frame processed (-1 if none)
	bool _selfLoop;		// does the script frame loop back on itself
	bool _executed;		// has the current frame already been run
	uint32 _timeNext;	// time the next frame should be run
	uint32 _timeCut;	// time to finish execution
	Common::Rect _drawWin;
	// these current ids are called "slot"s in the original
	int16 _currentFontId; 	// aka slot 0
	int16 _currentPalId;	// aka slot 1
	int16 _currentSongId;	// aka slot 3
	int16 _currentBmpId;	// aka slot 4
	int16 _currentGetPutId;	// aka slot 5
	int16 _brushNum;
	byte _drawColFG;
	byte _drawColBG;
	int16 _runPlayed;		// number of times the sequence has been started from ADS
	int16 _runCount;		// number of times to play the sequence before stopping
	int16 _timeInterval;	// interval between frames
	TTMRunType _runFlag;
	int16 _scriptFlag;
};

class TTMInterpreter {
public:
	TTMInterpreter(DgdsEngine *vm);
	virtual ~TTMInterpreter() {};

	bool load(const Common::String &filename, TTMEnviro &env);
	void unload();
	bool run(TTMEnviro &env, TTMSeq &seq);
	void findAndAddSequences(TTMEnviro &scriptData, Common::Array<Common::SharedPtr<TTMSeq>> &seqArray);

	static Common::String readTTMStringVal(Common::SeekableReadStream *scr);
	int32 findGOTOTarget(const TTMEnviro &env, const TTMSeq &seq, int16 frame);

protected:
	virtual void handleOperation(TTMEnviro &env, TTMSeq &seq, uint16 op, byte count, const int16 *ivals, const Common::String &sval, const Common::Array<Common::Point> &pts);
	void doWipeOp(uint16 code, const TTMEnviro &env, const TTMSeq &seq, const Common::Rect &r);
	int16 doInitCreditScrollOp(const Image *img);
	bool doCreditsScrollOp(const Image *img, int16 ygap, int16 ymax, int16 xoff, int16 measuredWidth, const Common::Rect &clipRect);
	void doDrawDialogForStrings(const TTMEnviro &env, const TTMSeq &seq, int16 x, int16 y, int16 width, int16 height);
	void doDrawSpriteOp(const TTMEnviro &env, const TTMSeq &seq, uint16 op, byte count, const int16 *ivals, int16 xoff = 0, int16 yoff = 0);
	void doFadeOutOp(int16 colorno, int16 ncolors, int16 targetcol, int16 speed);
	void doFadeInOp(int16 colorno, int16 ncolors, int16 targetcol, int16 speed);

	static const char *ttmOpName(uint16 op);

	DgdsEngine *_vm;
	int _stackDepth;
};

} // end namespace Dgds

#endif // DGDS_TTM_H
