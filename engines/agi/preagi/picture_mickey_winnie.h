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

#ifndef AGI_PREAGI_PICTURE_MICKEY_WINNIE_H
#define AGI_PREAGI_PICTURE_MICKEY_WINNIE_H

namespace Agi {

class PictureMgr_Mickey_Winnie : public PictureMgr {
public:
	PictureMgr_Mickey_Winnie(AgiBase *agi, GfxMgr *gfx);

	void drawPicture() override;
	void drawPicture_DOS_Amiga();
	void drawPicture_A2_C64_CoCo();

	void plotPattern(byte x, byte y) override;
	void plotBrush() override;

	void draw_Fill(int16 x, int16 y) override;

	bool getNextXCoordinate(byte &x) override;
	bool getNextYCoordinate(byte &y) override;
	
	bool getGraphicsCoordinates(int16 &x, int16 &y) override;
	
	void setOffset(int xOffset, int yOffset) {
		_xOffset = xOffset;
		_yOffset = yOffset;
	}

	void setMaxStep(int maxStep) { _maxStep = maxStep; }
	int getMaxStep() const { return _maxStep; }

private:
	bool _isDosOrAmiga;
	int16 _xOffset;
	int16 _yOffset;
	int _maxStep; // Max opcodes to draw, zero for all. Used by Mickey
};

} // End of namespace Agi

#endif
