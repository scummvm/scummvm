/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ_H
#define AGS_PLUGINS_AGSCREDITZ_AGSCREDITZ_H

#include "ags/plugins/dll.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCreditz {

struct Credit {
	Common::String _text;
	int _x = 0;
	int _y = 0;
	int _fontSlot = 0;
	int _color = 0;
	int _center = 0;
	bool _isSet = false;
	bool _image = false;
	bool _outline = false;
};

struct SequenceSettings {
	int startpoint = 0;
	int endpoint = 0;
	int speed = 0;
	bool finished = false;
	int automatic = 0;
	int endwait = 0;
	int topmask = 0;
	int bottommask = 0;
};

struct StCredit {
	Common::String credit;
	Common::String title;
	int x = 0;
	int y = 0;
	int font = 0;
	int color = 0;
	int title_x = 0;
	int title_y = 0;
	int title_font = 0;
	int title_color = 0;
	int pause = 0;
	bool image = false;
	int image_slot = 0;
	int image_time = 0;
	bool outline = false;
	bool title_outline = false;
};

struct StSequenceSettings {
	int speed = 0;
	bool finished = false;
};

struct SingleStatic {
	int id = 0;
	int time = 0;
	int style = 0;
	int settings = 01;
	int settings2 = 0;
	bool bool_ = false;
};

typedef Common::Array<Credit> CreditArray;
typedef Common::Array<StCredit> StCreditArray;

struct State {
	CreditArray _credits[10];
	StCreditArray _stCredits[10];
	bool _creditsRunning = 0, _paused = 0, _staticCredits = 0;
	int _creditSequence = 0, _yPos = 0, _sequenceHeight = 0, _speedPoint = 0;
	int _calculatedSequenceHeight = 0, _timer = 0, _currentStatic = 0;
	int _numChars = 0, _timer2 = 0;
	int _emptyLineHeight = 10;
	int _strCredit[10];
	SequenceSettings _seqSettings[10];
	StSequenceSettings _stSeqSettings[10];
	SingleStatic _singleStatic;
	void *_maskScreen = nullptr;
	void *_maski = nullptr;
	void *_creditScreen = nullptr;

	// Version 1.1 specific
	bool _resolutionFlag = false;
	int _screenWidth = 0, _screenHeight = 0, _screenColorDepth = 0;
};

class AGSCreditz : public DLL {
protected:
	enum Version {
		VERSION_11 = 11, VERSION_20 = 20
	};

	static Version _version;
	static State *_state;
	static IAGSEngine *_engine;
protected:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	// Shared Script methods
	static int IsCreditScrollingFinished(const ScriptMethodParams &params);
	static void SetCreditImage(const ScriptMethodParams &params);
	static void PauseScroll(const ScriptMethodParams &params);
	static void ScrollReset(const ScriptMethodParams &params);
	static void SetEmptyLineHeight(const ScriptMethodParams &params);
	static int GetEmptyLineHeight(const ScriptMethodParams &params);
	static void SetStaticCredit(const ScriptMethodParams &params);
	static string GetStaticCredit(const ScriptMethodParams &params);
	static void StartEndStaticCredits(const ScriptMethodParams &params);
	static int GetCurrentStaticCredit(const ScriptMethodParams &params);
	static void SetDefaultStaticDelay(const ScriptMethodParams &params);
	static void SetStaticPause(const ScriptMethodParams &params);
	static void SetStaticCreditTitle(const ScriptMethodParams &params);
	static void ShowStaticCredit(const ScriptMethodParams &params);
	static void StaticReset(const ScriptMethodParams &params);
	static string GetStaticCreditTitle(const ScriptMethodParams &params);
	static void SetStaticCreditImage(const ScriptMethodParams &params);
	static int IsStaticCreditsFinished(const ScriptMethodParams &params);
public:
	AGSCreditz();
	~AGSCreditz();
};

class AGSCreditz11 : public AGSCreditz {
private:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void SetCredit(const ScriptMethodParams &params);
	static void ScrollCredits(const ScriptMethodParams &params);
	static string GetCredit(const ScriptMethodParams &params);
public:
	AGSCreditz11();
};

class AGSCreditz20 : public AGSCreditz {
private:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void SetCredit(const ScriptMethodParams &params);
public:
	AGSCreditz20();
};

} // namespace AGSCreditz
} // namespace Plugins
} // namespace AGS3

#endif
