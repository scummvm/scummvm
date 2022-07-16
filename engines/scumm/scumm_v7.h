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

#ifndef SCUMM_SCRIPT_V7_H
#define SCUMM_SCRIPT_V7_H

#ifdef ENABLE_SCUMM_7_8

#include "scumm/scumm_v6.h"
#include "scumm/charset_v7.h"

namespace Scumm {

class Insane;
class SmushMixer;
class SmushPlayer;
class TextRenderer_v7;

class ScummEngine_v7 : public ScummEngine_v6 {
	friend class SmushPlayer;
	friend class Insane;
public:
	ScummEngine_v7(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v7() override;


protected:
	int _smushFrameRate;

	/**
	 * Flag which signals that the SMUSH video playback should end now
	 * (e.g. because it was aborted by the user or it's simply finished).
	 */
	bool _smushVideoShouldFinish;

	bool _smushActive;

	Insane *_insane;

public:
	SmushMixer *_smixer;
	SmushPlayer *_splayer;


	struct LangIndexNode {
		char tag[12+1];
		int32 offset;
	};

	struct InternalGUIControl {
		int relativeCenterX;
		int relativeCenterY;
		int xPos;
		int yPos;
		int primaryFillColor;
		int topLineColor;
		int bottomLineColor;
		int leftLineColor;
		int rightLineColor;
		int primaryLineColor;
		int secondaryLineColor;
		int secondaryFillColor;
		bool centerText;
		char *label;
	};

protected:
	TextRenderer_v7 *_textV7;
	Common::Rect _defaultTextClipRect;
	Common::Rect _wrappedTextClipRect;
	bool _newTextRenderStyle;
	int _blastTextRectsQueue = 0;

	int _verbLineSpacing;
	bool _existLanguageFile;
	char *_languageBuffer;
	LangIndexNode *_languageIndex;
	int _languageIndexSize;
	char _lastStringTag[12+1];

	struct SubtitleText : TextObject {
		void clear() {
			TextObject::clear();
			actorSpeechMsg = center = wrap = false;
		}
		bool actorSpeechMsg;
		bool center;
		bool wrap;
	};

	friend void syncWithSerializer(Common::Serializer &, SubtitleText &);

	int _subtitleQueuePos;
	SubtitleText _subtitleQueue[20];

	int32 _bannerColors[50]; // Colors for the original GUI
	byte *_bannerMem = nullptr;
	uint32 _bannerMemSize = 0;
	InternalGUIControl _internalGUIControls[30];
	char _emptyMsg[1] = "";

public:
	void processSubtitleQueue();
	void addSubtitleToQueue(const byte *text, const Common::Point &pos, byte color, byte charset, bool center, bool wrap);
	void clearSubtitleQueue();
	void CHARSET_1() override;
	bool isSmushActive() { return _smushActive; }
	void removeBlastTexts() override;
	void restoreBlastTextsRects();

protected:

	void scummLoop_handleSound() override;
	void scummLoop_handleDrawing() override;
	void processInput() override;
	void processKeyboard(Common::KeyState lastKeyHit) override;

	void setupScumm(const Common::String &macResourceFile) override;
	void resetScumm() override;

	void setupScummVars() override;
	void resetScummVars() override;

	void akos_processQueue() override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void readMAXS(int blockSize) override;
	void readGlobalObjects() override;
	void readIndexBlock(uint32 blocktype, uint32 itemsize) override;

	void setCameraAt(int pos_x, int pos_y) override;
	void setCameraFollows(Actor *a, bool setCamera = false) override;
	void moveCamera() override;
	void panCameraTo(int x, int y) override;

	int getObjectIdFromOBIM(const byte *obim) override;

	void createTextRenderer(GlyphRenderer_v7 *gr) override;
	void enqueueText(const byte *text, int x, int y, byte color, byte charset, TextStyleFlags flags);
	void drawTextImmediately(const byte *text, int x, int y, byte color, byte charset, TextStyleFlags flags);
	void drawBlastTexts() override;
	void showMessageDialog(const byte *msg) override;

	void actorTalk(const byte *msg) override;
	void translateText(const byte *text, byte *trans_buff) override;
	void loadLanguageBundle() override;
	void playSpeech(const byte *ptr);

	void initBanners();
	Common::KeyState showBannerAndPause(int bannerId, int32 waitTime, const char *msg, ...);
	void waitForBannerInput(int32 waitTime, Common::KeyState &ks, bool &leftBtnClicked, bool &rightBtnClicked);
	void clearBanner();
	void setBannerColors(int bannerId, byte r, byte g, byte b);
	int getBannerColor(int bannerId);
	void setUpInternalGUIControl(int id, int primaryFillColor, int primaryLineColor,
								 int topLineColor, int bottomLineColor, int leftLineColor, int rightLineColor,
								 int secondaryLineColor, int secondaryFillColor,
								 int anchorPointX, int anchorPointY, int x, int y, char *label, bool centerFlag);
	void drawInternalGUIControl(int id, bool useSecondaryColor);
	int getInternalGUIControlFromCoordinates(int x, int y);
	void confirmExitDialog() override;

	void setDefaultCursor() override;

	void drawVerb(int verb, int mode) override;

	void pauseEngineIntern(bool pause) override;

	void o6_kernelSetFunctions() override;

	struct BlastText : TextObject {
		Common::Rect rect;
		TextStyleFlags flags;

		void clear() {
			this->TextObject::clear();
			rect = Common::Rect();
		}
	};

	int _blastTextQueuePos;
	BlastText _blastTextQueue[50];
};


} // End of namespace Scumm

#endif // ENABLE_SCUMM_7_8

#endif
