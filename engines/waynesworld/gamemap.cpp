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

#include "waynesworld/waynesworld.h"
#include "waynesworld/graphics.h"

namespace WaynesWorld {

struct MapItem9 {
    int roomNumber;
    int x0, y0;
    int x1, y1;
};

struct MapItem13 {
    int roomNumber;
    const char *name;
    int animX, animY;
    int tagX, tagY;
};

struct MapItem17 {
    const char *name;
    int tagX, tagY;
    int roomNumber;
    int wayneX, wayneY;
    int garthX, garthY;
};

static const MapItem9 kMapItem9s[] = {
    { 102, 40, 94, 112, 89 },
    { 102, 40, 94, 69, 47 },
    { 102, 40, 94, 157, 66 },
    { 101, 84, 28, 84, 44 },
    { 101, 84, 28, 154, 54 },
    { 101, 84, 28, 85, 72 },
    { 103, 230, 35, 188, 41 },
    { 103, 230, 35, 231, 64 },
    { 104, 224, 80, 223, 70 },
    { 104, 224, 80, 230, 97 },
    { 105, 139, 113, 86, 80 },
    { 105, 139, 113, 126, 118 },
    { 105, 139, 113, 180, 129 },
    { 106, 256, 130, 177, 90 },
    { 106, 256, 130, 265, 98 }
};

static const MapItem13 kMapItem13s[] = {
    { 101, "na", 53, 6, 84, 49 },
    { 102, "wa", 34, 32, 56, 87 },
    { 103, "but", 127, 11, 247, 38 },
    { 104, "stn", 173, 13, 173, 90 },
    { 105, "dt", 70, 69, 125, 135 },
    { 106, "sea", 139, 57, 218, 142 }
};

static const MapItem17 kMapItem17s[] = {
    { "cab", 61, 18, 3, 58, 108, 44, 102 },
    { "ad", 141, 31, 4, 51, 70, 40, 67 },
    { "cas", 69, 80, 5, 291, 105, 312, 102 },
    { "inv", 49, 52, 1, 23, 111, 15, 102 },
    { "way", 73, 86, 0, 168, 92, 184, 95 },
    { "don", 120, 47, 2, 126, 90, 117, 88 },
    { "cin", 147, 27, 6, 185, 137, 151, 139 },
    { "c35", 208, 72, 7, 157, 122, 180, 122 },
    { "may", 193, 40, 8, 243, 103, 239, 95 },
    { "eug", 241, 82, 9, 250, 96, 263, 93 },
    { "jun", 78, 87, 10, -1, -1, -1, -1 },
    { "cih", 134, 102, 11, 130, 140, 160, 131 },
    { "uno", 142, 137, 12, 289, 114, 294, 110 },
    { "pep", 151, 101, 13, 61, 77, 65, 75 },
    { "gil", 224, 106, 14, 195, 141, 229, 138 }
};

void WaynesWorldEngine::gameMapOpen() {
    _gameMapRoomNumber = _currentRoomNumber;
    _gameMapWayneSpriteX = _wayneSpriteX;
    _gameMapGarthSpriteX = _garthSpriteX;
    unloadStaticRoomObjects();
    _currentRoomNumber = 100;
    _wayneSpriteX = -1;
    _garthSpriteX = -1;
    _currentMapItemIndex = -1;
    _isTextVisible = false;
    /* TODO
    if (_byte_3070D & 0x08) {
        setWaynePosition(243, 103);
        setGarthPosition(239, 95);
        changeRoom(8);
        return;
    }
    */
    // sysMouseDriver(2);
    paletteFadeOut(0, 256, 16);
    /* TODO
    if ((_byte_306F4 & 0x04) && !(_byte_306F4 & 0x08)) {
        // TODO sub_305E0();
        _gameState = 0;
        // sysMouseDriver(1);
        drawInterface(_verbNumber);
        paletteFadeIn(0, 256, 4);
        return;
    }
    */
    _musicIndex = 2;
    changeMusic();
    _screen->clear(0);
    drawImageToScreen("m02/main_map", 0, 0);
    _screen->fillSquare(kMapItem9s[_gameMapRoomNumber].x0, kMapItem9s[_gameMapRoomNumber].y0, 3, 109);
    paletteFadeIn(0, 256, 16);
    // TODO gameMapPaletteHandlerStart();
    // sysMouseDriver(1);
}

void WaynesWorldEngine::gameMapFinish() {
    _gameState = 0;
    // TODO gameMapPaletteHandlerStop();
    paletteFadeOut(0, 256, 16);
    _musicIndex = 0;
    changeMusic();
    _screen->clear(0);
    drawInterface(_verbNumber);
    changeRoom(_gameMapDestinationRoomNum);
    paletteFadeIn(0, 256, 16);
}

void WaynesWorldEngine::gameMapHandleMouseMove(int objectNumber) {
    if (_hoverObjectNumber == objectNumber)
        return;
    _hoverObjectNumber = objectNumber;

    Common::String tagFilename;
    int tagX = -1, tagY = -1;

    if (_hoverObjectNumber == -1) {
        if (_currentMapItemIndex == 6) {
            // TODO gameMapPaletteHandlerStop();
            // TODO paletteFadeColor(36, 4, 21, 2, 64);
            // TODO gameMapPaletteHandlerStart();
            _currentMapItemIndex = -1;
        } else if (_currentMapItemIndex >= 0 && _currentMapItemIndex <= 5) {
            const MapItem13 &item = kMapItem13s[_currentMapItemIndex];
            tagFilename = Common::String::format("m02/%s_xtag", item.name);
            tagX = item.tagX;
            tagY = item.tagY;
        } else if (_currentMapItemIndex >= 7 && _currentMapItemIndex <= 21) {
            const MapItem17 &item = kMapItem17s[_currentMapItemIndex - 7];
            tagFilename = Common::String::format("m02/%s_xtag", item.name);
            tagX = item.tagX;
            tagY = item.tagY;
        }
    } else if (_hoverObjectNumber >= 0 && _hoverObjectNumber <= 5) {
        const MapItem13 &item = kMapItem13s[_hoverObjectNumber];
        tagFilename = Common::String::format("m02/%s_tag", item.name);
        tagX = item.tagX;
        tagY = item.tagY;
    } else if (_hoverObjectNumber >= 7 && _hoverObjectNumber <= 21) {
        const MapItem17 &item = kMapItem17s[_hoverObjectNumber - 7];
        tagFilename = Common::String::format("m02/%s_tag", item.name);
        tagX = item.tagX;
        tagY = item.tagY;
    } else if (_hoverObjectNumber == 6 || (_hoverObjectNumber >= 22 && _hoverObjectNumber <= 27)) {
        // TODO gameMapPaletteHandlerStop();
        // TODO paletteFadeColor(36, 63, 0, 0, 64);
        // TODO gameMapPaletteHandlerStart();
        _currentMapItemIndex = 6;
    }

    if (tagX != -1 && tagY != -1) {
        _currentMapItemIndex = _hoverObjectNumber;
        drawImageToScreen(tagFilename.c_str(), tagX, tagY);
    }

}

void WaynesWorldEngine::gameMapHandleMouseClick() {
    bool isDone = false;

    if (_hoverObjectNumber == 6) {
        _gameMapDestinationRoomNum = _gameMapRoomNumber;
        _wayneSpriteX = _gameMapWayneSpriteX;
        _garthSpriteX = _gameMapGarthSpriteX;
        isDone = true;
    } else if (_hoverObjectNumber >= 22 && _hoverObjectNumber <= 27) {
        _gameMapFlag = false;
        _currentRoomNumber = 100;
        drawImageToScreen("m02/main_map", 0, 0);
        _screen->fillSquare(kMapItem9s[_gameMapRoomNumber].x0, kMapItem9s[_gameMapRoomNumber].y0, 3, 109);
    } else if (_hoverObjectNumber >= 0 && _hoverObjectNumber <= 5) {
        const MapItem13 &item = kMapItem13s[_hoverObjectNumber];
        _currentRoomNumber = item.roomNumber;
        _currentMapItemIndex = -1;
        gameMapSelectItem(item.name, item.animX, item.animY);
    } else if (_hoverObjectNumber >= 7 && _hoverObjectNumber <= 21) {
        const MapItem17 &item = kMapItem17s[_hoverObjectNumber - 7];
        _gameMapDestinationRoomNum = item.roomNumber;
        _wayneSpriteX = item.wayneX;
        _wayneSpriteY = item.wayneY;
        _garthSpriteX = item.garthX;
        _garthSpriteY = item.garthY;
        isDone = true;
    }

    if (isDone) {
        gameMapFinish();
        loadPalette("m01/wstand0");
    }

}

void WaynesWorldEngine::gameMapSelectItem(const char *prefix, int animX, int animY) {
    // sysMouseDriver(2);
    for (int frameNum = 0; frameNum < 12; frameNum++) {
        Common::String filename = Common::String::format("m02/%s_zm%d", prefix, frameNum);
        drawImageToScreen(filename.c_str(), animX, animY);
        waitMillis(50);
    }
    drawImageToScreen("m02/zmbtn", 125, 188);
    if (kMapItem9s[_gameMapRoomNumber].roomNumber == _currentRoomNumber) {
        _screen->fillSquare(kMapItem9s[_gameMapRoomNumber].x1, kMapItem9s[_gameMapRoomNumber].y1, 3, 109);
    }
    // sysMouseDriver(1);
    _gameMapFlag = true;
}

} // End of namespace WaynesWorld
