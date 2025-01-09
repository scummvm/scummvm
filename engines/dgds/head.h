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

#ifndef DGDS_HEAD_H
#define DGDS_HEAD_H

#include "common/str.h"
#include "common/types.h"
#include "common/ptr.h"
#include "common/array.h"

#include "graphics/managed_surface.h"

#include "dgds/dgds_rect.h"
#include "dgds/ttm.h"

// Classes related to talking heads and conversations.

namespace Dgds {

class Image;
class SoundRaw;
class Dialog;

class TalkDataHeadFrame {
public:
	TalkDataHeadFrame() : _xoff(0), _yoff(0), _frameNo(0), _flipFlags(0) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _frameNo;
	int16 _xoff;
	int16 _yoff;
	uint16 _flipFlags;
};

enum HeadFlags {
	kHeadFlagNone = 0,
	kHeadFlag1 = 1,
	kHeadFlag2 = 2,
	kHeadFlag4 = 4,
	kHeadFlag8 = 8,
	kHeadFlag10 = 0x10,
	kHeadFlagVisible = 0x20,
	kHeadFlag40 = 0x40,
	kHeadFlag80 = 0x80,
};

class TalkData;

class TalkDataHead {
public:
	TalkDataHead() : _num(0), _drawType(0), _drawCol(0), _flags(kHeadFlagNone) {}
	Common::String dump(const Common::String &indent) const;

	void updateHead();

	void drawHead(Graphics::ManagedSurface *dst, const TalkData &data) const;
	void drawHeadType1(Graphics::ManagedSurface *dst, const Image &img) const;
	void drawHeadType2(Graphics::ManagedSurface *dst, const Image &img) const;
	void drawHeadType3(Graphics::ManagedSurface *dst, const Image &img) const;
	void drawHeadType3Beamish(Graphics::ManagedSurface *dst, const TalkData &data) const;

	uint16 _num;
	uint16 _drawType;
	uint16 _drawCol;
	DgdsRect _rect;
	Common::Array<TalkDataHeadFrame> _headFrames;
	Common::String _bmpFile;
	HeadFlags _flags;
	Common::SharedPtr<Image> _shape;
};

/** TDS talking head data from HOC+ */
class TalkData {
public:
	TalkData() : _num(0), _val(0) {}
	Common::String dump(const Common::String &indent) const;

	uint16 _num;
	Common::SharedPtr<Image> _shape;
	Common::Array<TalkDataHead> _heads;
	uint16 _val;
	Common::String _bmpFile;

	void updateVisibleHeads();
	void drawVisibleHeads(Graphics::ManagedSurface *dst) const;
	bool hasVisibleHead() const;
};


/**
 * A TTM interpreter with customized opcode handling for the TTM bit of
 * CDS files (Willy Beamish CD version conversation data)
 */
class CDSTTMInterpreter : public TTMInterpreter {
public:
	CDSTTMInterpreter(DgdsEngine *vm);

	Graphics::ManagedSurface &getStoredAreaBuffer() { return _storedAreaBuffer; }

protected:
	virtual void handleOperation(TTMEnviro &env, TTMSeq &seq, uint16 op, byte count, const int16 *ivals, const Common::String &sval, const Common::Array<Common::Point> &pts) override;

	Graphics::ManagedSurface _storedAreaBuffer;
};

class CDSTTMEnviro : public TTMEnviro {
public:
	CDSTTMEnviro() : _cdsPlayedSound(false), _cdsFrame(-1), _cdsJumped(false), _cdsDelay(0),
	_cdsDidStoreArea(false), TTMEnviro()
	{}

	bool _cdsPlayedSound;
	int16 _cdsFrame; // The GOTO target to use in the CDS script (Willy Beamish talkie)
	int16 _cdsDelay;
	bool _cdsJumped;
	bool _cdsDidStoreArea;
	DgdsRect _storedAreaRect;
};

/** CDS data from Willy Beamish CD version talkie */
class Conversation {
public:
	Conversation() : _nextExecMs(0), _runTempFrame(0), _tempFrameNum(0), _stopScript(false), _loadState(0), _dlgNum(-1), _dlgFileNum(-1), _subNum(-1), _finished(false), _haveHeadData(false) {}
	~Conversation();

	void unloadData();
	void runScript();
	void loadData(uint16 num, uint16 num2, int16 sub, bool haveHeadData);
	bool isForDlg(const Dialog *dlg) const;
	bool isFinished() const { return _finished; }
	void clear();

	DgdsRect _drawRect;

private:
	Common::SharedPtr<CDSTTMInterpreter> _ttmScript;
	Common::Array<Common::SharedPtr<TTMSeq>> _ttmSeqs;
	CDSTTMEnviro _ttmEnv;
	uint32 _nextExecMs;

	bool runScriptFrame(int16 frameNum);
	void checkAndRunScript();
	void incrementFrame();
	bool isScriptRunning();
	void pumpMessages();

	int16 _runTempFrame;
	int16 _tempFrameNum;

	uint32 _thisFrameMs;
	bool _stopScript;
	int16 _loadState;
	int16 _dlgNum;
	int16 _dlgFileNum;
	int16 _subNum;
	bool _finished;
	bool _haveHeadData;
};


} // end namespace Dgds

#endif // DGDS_HEAD_H
