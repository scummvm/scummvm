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

#include "gamos/gamos.h"

namespace Gamos {

void GamosEngine::storeToGameScreen(int id) {
    GameScreen &gs = _gameScreens[id];
    gs._savedObjects.clear();

    gs._savedStates = _states;

    int objCount = 0;
    for (int i = 0; i < _objects.size(); i++) {
        const Object &obj = _objects[i];
        if ((obj.flags & 3) == 3 || (obj.flags & 7) == 1)
            objCount++;
    }

    int idx = 0;
    gs._savedObjects.resize(objCount);
    for (int i = 0; i < _objects.size(); i++) {
        Object &obj = _objects[i];

        if ( (obj.flags & 3) == 3 ) {
            const int refObjIdx = idx;
            if (obj.x == -1) {
                gs._savedObjects[idx] = obj;
                gs._savedObjects[idx].index = idx;
                obj.flags = 0;
                idx++;
            } else {
                Object &drawObj = _objects[ obj.x ];
                gs._savedObjects[idx] = obj;
                gs._savedObjects[idx].index = idx;
                gs._savedObjects[idx].x = idx + 1;
                gs._savedObjects[idx].y = idx + 1;
                gs._savedObjects[idx + 1] = drawObj;
                gs._savedObjects[idx + 1].index = idx + 1;
                gs._savedObjects[idx + 1].pos = idx & 0xff;
                gs._savedObjects[idx + 1].blk = (idx >> 8) & 0xff;
                obj.flags = 0;
                drawObj.flags = 0;
                idx += 2;
            }

            for (int j = 0; j < _objects.size(); j++) {
                Object &lobj = _objects[ j ];
                if ((lobj.flags & 7) == 1 && ((lobj.blk << 8) | lobj.pos) == obj.index) {
                    gs._savedObjects[idx] = lobj;
                    gs._savedObjects[idx].index = idx;
                    gs._savedObjects[idx].pos = refObjIdx & 0xff;
                    gs._savedObjects[idx].blk = (refObjIdx >> 8) & 0xff;
                    lobj.flags = 0;
                    idx++;
                }
            }
        } else if ( (obj.flags & 7) == 1 && obj.pos == 0xff && obj.blk == 0xff ) {
            gs._savedObjects[idx] = obj;
            gs._savedObjects[idx].index = idx;
            obj.flags = 0;
            idx++;
        }
    }

    _objects.clear();
}


bool GamosEngine::switchToGameScreen(int id, bool doNotStore) {
    if (_currentGameScreen != -1 && doNotStore == false)
        storeToGameScreen(_currentGameScreen);

    _currentGameScreen = id;
    GameScreen &gs = _gameScreens[id];

    addDirtyRect(Common::Rect(Common::Point(), _bkgUpdateSizes ));

    _states = gs._savedStates;

    for(const Object &obj : gs._savedObjects) {
        Object *nobj = getFreeObject();
        if (nobj->index != obj.index) {
            warning("Error!  nobj->index != obj.index");
            return false;
        }

        *nobj = obj;
    }

    gs._savedObjects.clear();
    gs._savedStates.clear();

    flushDirtyRects(false);

    if (doNotStore == false && !setPaletteCurrentGS())
        return false;

    return true;
}

}
