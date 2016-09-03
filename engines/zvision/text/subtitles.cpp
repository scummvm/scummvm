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

#include "zvision/graphics/render_manager.h"
#include "zvision/text/subtitles.h"
#include "zvision/file/search_manager.h"
#include "zvision/text/text.h"

namespace ZVision {

Subtitle::Subtitle(ZVision *engine, const Common::String &subname, bool upscaleToHires) :
	_engine(engine),
	_areaId(-1),
	_subId(-1) {
	Common::File file;
	if (_engine->getSearchManager()->openFile(file, subname)) {
		while (!file.eos()) {
			Common::String str = file.readLine();
			if (str.lastChar() == '~')
				str.deleteLastChar();

			if (str.matchString("*Initialization*", true)) {
				// Not used
			} else if (str.matchString("*Rectangle*", true)) {
				int32 x1, y1, x2, y2;
				sscanf(str.c_str(), "%*[^:]:%d %d %d %d", &x1, &y1, &x2, &y2);
				Common::Rect rct = Common::Rect(x1, y1, x2, y2);
				if (upscaleToHires)
					_engine->getRenderManager()->upscaleRect(rct);
				_areaId = _engine->getRenderManager()->createSubArea(rct);
			} else if (str.matchString("*TextFile*", true)) {
				char filename[64];
				sscanf(str.c_str(), "%*[^:]:%s", filename);
				Common::File txt;
				if (_engine->getSearchManager()->openFile(txt, filename)) {
					while (!txt.eos()) {
						Common::String txtline = readWideLine(txt);
						sub curSubtitle;
						curSubtitle.start = -1;
						curSubtitle.stop = -1;
						curSubtitle.subStr = txtline;

						_subs.push_back(curSubtitle);
					}
					txt.close();
				}
			} else {
				int32 st;
				int32 en;
				int32 sb;
				if (sscanf(str.c_str(), "%*[^:]:(%d,%d)=%d", &st, &en, &sb) == 3) {
					if (sb <= (int32)_subs.size()) {
						if (upscaleToHires) {
							// Convert from 15FPS (AVI) to 29.97FPS (VOB)
							st = st * 2997 / 1500;
							en = en * 2997 / 1500;
						}
						_subs[sb].start = st;
						_subs[sb].stop = en;
					}
				}
			}
		}
	}
}

Subtitle::~Subtitle() {
	if (_areaId != -1)
		_engine->getRenderManager()->deleteSubArea(_areaId);

	_subs.clear();
}

void Subtitle::process(int32 time) {
	int16 j = -1;
	for (uint16 i = 0; i < _subs.size(); i++)
		if (time >= _subs[i].start && time <= _subs[i].stop) {
			j = i;
			break;
		}

	if (j == -1 && _subId != -1) {
		if (_areaId != -1)
			_engine->getRenderManager()->updateSubArea(_areaId, "");
		_subId = -1;
	}

	if (j != -1 && j != _subId) {
		if (_subs[j].subStr.size())
			if (_areaId != -1)
				_engine->getRenderManager()->updateSubArea(_areaId, _subs[j].subStr);
		_subId = j;
	}
}

} // End of namespace ZVision
