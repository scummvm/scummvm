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

#ifndef BBVS_MINIGAMES_BBAIRGUITAR_H
#define BBVS_MINIGAMES_BBAIRGUITAR_H

#include "bbvs/minigames/minigame.h"

namespace Bbvs {

class MinigameBbAirGuitar : public Minigame {
public:
	MinigameBbAirGuitar(BbvsEngine *vm) : Minigame(vm) {};
	bool run(bool fromMainGame) override;
public:

	struct Obj {
		int kind;
		int x, y;
		int xIncr, yIncr;
		const ObjAnimation *anim;
		int frameIndex;
		int ticks;
		int status;
		int16 frameIndexAdd;
		int16 unk2;
	};

	enum {
		kMaxObjectsCount = 256,
		kMaxTracks = 2048
	};

	struct PianoKeyInfo {
		int x, y;
		int frameIndex;
	};

	struct TrackEvt {
		int8 noteNum;
		int16 ticks;
	};

	Obj _objects[kMaxObjectsCount];

	int _playerMode;

	bool _modified;

	TrackEvt _track[kMaxTracks];
	int _trackIndex, _trackCount;

	int _noteStartTime;

	int _vuMeterLeft1, _vuMeterLeft2;
	int _vuMeterRight1, _vuMeterRight2;

	bool _resetAnims;
	bool _rockTunePlaying;

	int _currButtonNum;
	int _buttonClickTicks;

	int *_currFrameIndex;
	int _btn3KindToggle;

	const BBPolygon *_currPianoKeyArea;
	const Rect *_currPlayerButtonRect;

	bool _movingTrackBar;
	int _trackBarMouseX;
	int _trackBarX;
	Rect _trackBarThumbRect;

	int _currTrackPos, _totalTrackLength;
	int _ticksDelta;

	int _actionStartTrackPos, _actionTrackPos;
	int _actionStartTime;

	int _currNoteNum;
	int _currPatchNum;

	const ObjAnimation *getAnimation(int animIndex);
	bool ptInRect(const Rect *r, int x, int y);
	bool ptInPoly(const BBPolygon *poly, int x, int y);

	void buildDrawList(DrawList &drawList);
	void buildDrawList0(DrawList &drawList);
	void buildDrawList1(DrawList &drawList);

	void drawSprites();

	void initObjs();
	Obj *getFreeObject();

	void initObjects();
	void initObjects0();
	void initObjects1();

	bool updateStatus(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus0(int mouseX, int mouseY, uint mouseButtons);
	bool updateStatus1(int mouseX, int mouseY, uint mouseButtons);

	void updateObjs();

	void update();

	void play();
	void record();
	void setPlayerMode3();
	void stop();
	void changePatch(int patchNum);
	void afterButtonReleased();
	void calcTotalTicks2();
	void calcTotalTicks1();
	void noteOn(int noteNum);
	void noteOff(int noteNum);
	void resetObjs();

	void loadSounds();
	void playNote(int noteNum);
	void stopNote(int noteNum);

	bool getLoadFilename(Common::String &filename);
	bool getSaveFilename(Common::String &filename);
	bool querySaveModifiedDialog();
	bool querySaveModifiedTracks();
	bool loadTracks();
	bool saveTracks();
	bool loadFromStream(Common::ReadStream *stream);
	void saveToStream(Common::WriteStream *stream);

};

} // End of namespace Bbvs

#endif // BBVS_MINIGAMES_BBAIRGUITAR_H
