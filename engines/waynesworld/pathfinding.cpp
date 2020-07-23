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

namespace WaynesWorld {

bool WaynesWorldEngine::walkIsPixelWalkable(int x, int y) {
	if (x < 0 || y < 0 || x >= 320 || y >= 150)
		return false;
	return (_walkMap[(y * 40) + (x / 8)] & (0x80 >> (x % 8))) != 0;
}

bool WaynesWorldEngine::walkAdjustDestPoint(int &x, int &y) {
    if (walkIsPixelWalkable(x, y))
        return true;
    for (int incr = 1; incr < 200; incr++) {
        for (int xc = 0; xc <= incr; xc++) {
            if (y + incr < 150) {
                if (walkIsPixelWalkable(x + xc, y + incr)) {
                    x += xc;
                    y += incr;
                    return true;
                }
                if (walkIsPixelWalkable(x - xc, y + incr)) {
                    x -= xc;
                    y += incr;
                    return true;
                }
            } else if (y - incr >= 0) {
                if (walkIsPixelWalkable(x + xc, y - incr)) {
                    x += xc;
                    y -= incr;
                    return true;
                }
                if (walkIsPixelWalkable(x - xc, y - incr)) {
                    x -= xc;
                    y -= incr;
                    return true;
                }
            }
        }
        for (int yc = 0; yc <= incr - 1; yc++) {
            if (x + incr < 320) {
                if (walkIsPixelWalkable(x + incr, y + yc)) {
                    x += incr;
                    y += yc;
                    return true;
                }
                if (walkIsPixelWalkable(x + incr, y - yc)) {
                    x += incr;
                    y -= yc;
                    return true;
                }
            } else if (x + incr >= 0) {
                if (walkIsPixelWalkable(x - incr, y + yc)) {
                    x -= incr;
                    y += yc;
                    return true;
                }
                if (walkIsPixelWalkable(x - incr, y - yc)) {
                    x -= incr;
                    y -= yc;
                    return true;
                }
            }
        }
    }
    return false;
}

void WaynesWorldEngine::walkCalcOtherActorDest(int flag, int &x, int &y) {
    int direction;
    if (flag == 1) {
        direction = walkCalcDirection(x - _garthSpriteX, y - _garthSpriteY);
    } else {
        direction = walkCalcDirection(x - _wayneSpriteX, y - _wayneSpriteY);
    }
    int actorScale = getActorScaleFromY(y);
    int actorDistance = (actorScale * 20) / 100;
    switch (direction) {
    case 0:
        y += actorDistance;
    case 1:
        x += actorDistance;
        break;
    case 2:
        x += actorDistance;
    case 3:
        y -= actorDistance;
        break;
    case 4:
        y -= actorDistance;
    case 5:
        x -= actorDistance;
        break;
    case 6:
        x -= actorDistance;
    case 7:
        y += actorDistance;
        break;
    }
    walkAdjustDestPoint(x, y);
}

int WaynesWorldEngine::walkCalcPath(int flag, int sourceX, int sourceY, int destX, int destY, int pointsCount) {
    if (pointsCount >= 300)
        return 299;

    if (walkIsLineWalkable(sourceX, sourceY, destX, destY))
        return walkAddWalkLine(flag, sourceX, sourceY, destX, destY, pointsCount);

    int nextSourceX, nextSourceY;

    walkGetNextPoint(sourceX, sourceY, destX, destY, nextSourceX, nextSourceY);

    bool walkable = walkFindPoint(flag, sourceX, sourceY, nextSourceX, nextSourceY, destX, destY, pointsCount);
    while (!walkable) {
        walkGetNextPoint(sourceX, sourceY, nextSourceX, nextSourceY, nextSourceX, nextSourceY);
        if (ABS(sourceX - nextSourceX) < 3 && ABS(sourceY - nextSourceY) < 3) {
            return pointsCount;
        }
        walkable = walkFindPoint(flag, sourceX, sourceY, nextSourceX, nextSourceY, destX, destY, pointsCount);
    }

    pointsCount = walkAddWalkLine(flag, sourceX, sourceY, nextSourceX, nextSourceY, pointsCount);

    sourceX = nextSourceX;
    sourceY = nextSourceY;
    pointsCount = walkCalcPath(flag, sourceX, sourceY, destX, destY, pointsCount);

    return pointsCount;
}

bool WaynesWorldEngine::walkFindPoint(int flag, int &sourceX, int &sourceY, int &nextSourceX, int &nextSourceY, int destX, int destY, int pointsCount) {
    bool walkable = false;

    if (walkIsLineWalkable(sourceX, sourceY, nextSourceX, nextSourceY))
        return true;

    float slopeX;

    if (sourceX == nextSourceX) {
        slopeX = 0.0f;
    } else if (sourceY == nextSourceY) {
        slopeX = 100.0f;
    } else {
        slopeX = -(float(sourceX - nextSourceX) / float(sourceY - nextSourceY));
    }

    int scan1X = sourceX, scan1Y = sourceY;
    int scan2X = nextSourceX, scan2Y = nextSourceY;
    bool scan1Done = false;
    int scan1Incr = 0;

    while (!walkable && !scan1Done) {
        scan1Done = true;
        int scan1Sign = -1;
        while (scan1Sign < 2 && !walkable) {
            if (ABS(slopeX) >= 1.0f) {
                float slopeY = 1.0f / slopeX;
                scan1Y = sourceY + scan1Incr * scan1Sign;
                scan1X = sourceX + (scan1Y - sourceY) * slopeY;
            } else {
                scan1X = sourceX + scan1Incr * scan1Sign;
                scan1Y = sourceY + (scan1X - sourceX) * slopeX;
            }
            if (scan1X >= 0 && scan1X < 320 && scan1Y >= 0 && scan1Y < 150) {
                scan1Done = false;
                bool scan2Done = false;
                int scan2Incr = 0;
                while (!walkable && !scan2Done) {
                    scan2Done = true;
                    int scan2Sign = -1;
                    while (scan2Sign < 2 && !walkable) {
                        if (ABS(slopeX) >= 1.0f) {
                            float slopeY = 1.0f / slopeX;
                            scan2Y = nextSourceY + scan2Incr * scan2Sign;
                            scan2X = nextSourceX + (scan2Y - nextSourceY) * slopeY;
                        } else {
                            scan2X = nextSourceX + scan2Incr * scan2Sign;
                            scan2Y = nextSourceY + (scan2X - nextSourceX) * slopeX;
                        }
                        if (scan2X >= 0 && scan2X < 320 && scan2Y >= 0 && scan2Y < 150) {
                            scan2Done = false;
                            walkable = walkIsLineWalkable(scan1X, scan1Y, scan2X, scan2Y) && walkTestPoint(scan1X, scan1Y, scan2X, scan2Y, destX, destY);
                        }
                        scan2Sign += 2;
                    }
                    scan2Incr += 4;
                }
            }
            scan1Sign += 2;
        }
        scan1Incr += 4;
    }

    if (scan1Done)
        return false;

    if (sourceX != scan1X || sourceY != scan1Y) {
        if (!walkTestPoint(sourceX, sourceY, scan1X, scan1Y, destX, destY)) {
            return false;
		}
        pointsCount = walkAddWalkLine(flag, sourceX, sourceY, scan1X, scan1Y, pointsCount);
        sourceX = scan1X;
        sourceY = scan1Y;
    }

    nextSourceX = scan2X;
    nextSourceY = scan2Y;

    return true;
}

bool WaynesWorldEngine::walkTestPoint(int sourceX, int sourceY, int nextSourceX, int nextSourceY, int destX, int destY) {
    return ABS(destY - nextSourceY) < ABS(destY - sourceY) || ABS(destX - nextSourceX) < ABS(destX - sourceX);
}

bool WaynesWorldEngine::walkIsLineWalkable(int sourceX, int sourceY, int destX, int destY) {
    bool walkable = true;
    if (sourceX == destX) {
        const int incr = sourceY > destY ? -4 : 4;
        for (int yc = sourceY; yc * incr < destY * incr && walkable; yc += incr) {
            if (!walkIsPixelWalkable(sourceX, yc)) {
                walkable = false;
            }
        }
    } else if (ABS(sourceY - destY) >= ABS(sourceX - destX)) {
        const float slopeY = float(sourceX - destX) / float(sourceY - destY);
        const int incr = sourceY > destY ? -4 : 4;
        for (int yc = sourceY; yc * incr < destY * incr && walkable; yc += incr) {
            if (!walkIsPixelWalkable(sourceX + (yc - sourceY) * slopeY, yc)) {
                walkable = false;
            }
        }
    } else {
        const float slopeX = float(sourceY - destY) / float(sourceX - destX);
        const int incr = sourceX > destX ? -4 : 4;
        for (int xc = sourceX; xc * incr < destX * incr && walkable; xc += incr) {
            if (!walkIsPixelWalkable(xc, sourceY + (xc - sourceX) * slopeX)) {
                walkable = false;
            }
        }
    }
    if (!walkIsPixelWalkable(destX, destY)) {
        walkable = false;
    }
    return walkable;
}

void WaynesWorldEngine::walkGetNextPoint(int sourceX, int sourceY, int destX, int destY, int &nextX, int &nextY) {
    nextX = (sourceX + destX) / 2;
    nextY = (sourceY + destY) / 2;
}

int WaynesWorldEngine::walkCalcDirection(int deltaX, int deltaY) {
    int direction = 0;
    if (deltaY < 0) {
        if (deltaX < 0) {
            if (deltaX * 2 > deltaY) {
                direction = 0;
            } else if (deltaY * 2 > deltaX) {
                direction = 2;
            } else {
                direction = 1;
            }
        } else {
            if (deltaX * -2 > deltaY) {
                direction = 0;
            } else if (deltaY * -2 < deltaX) {
                direction = 6;
            } else {
                direction = 7;
            }
        }
    } else {
        if (deltaX < 0) {
            if (deltaX * -2 < deltaY) {
                direction = 4;
            } else if (deltaY * -2 > deltaX) {
                direction = 2;
            } else {
                direction = 3;
            }
        } else {
            if (deltaX * 2 < deltaY) {
                direction = 4;
            } else if (deltaY * 2 < deltaX) {
                direction = 6;
            } else {
                direction = 5;
            }
        }
    }
    return direction;
}

int WaynesWorldEngine::walkAddWalkLine(int flag, int x1, int y1, int x2, int y2, int pointsCount) {
    WalkPoint *walkPoints = flag == 0 ? _wayneWalkPoints : _garthWalkPoints;
    int newDirection = walkCalcDirection(x1 - x2, y1 - y2);
	// debug("walkAddWalkLine() %d, %d, %d, %d", x1, y1, x2, y2);

    if (pointsCount == 0 || walkPoints[pointsCount - 1].x != x1 || walkPoints[pointsCount - 1].y != y1) {
        walkPoints[pointsCount].x = x1;
        walkPoints[pointsCount].y = y1;
        walkPoints[pointsCount].direction = newDirection;
        pointsCount++;
    }

    if (x1 == x2) {
        const int incr = y1 > y2 ? -4 : 4;
        for (int yc = y1; yc * incr < y2 * incr; yc += incr) {
            walkPoints[pointsCount].x = x1;
            walkPoints[pointsCount].y = yc;
            walkPoints[pointsCount].direction = newDirection;
            pointsCount++;
        }
    } else if (ABS(y1 - y2) >= ABS(x1 - x2)) {
        const float slopeY = float(x1 - x2) / float(y1 - y2);
        const int incr = y1 > y2 ? -4 : 4;
        for (int yc = y1; yc * incr < y2 * incr; yc += incr) {
            walkPoints[pointsCount].x = x1 + (yc - y1) * slopeY;
            walkPoints[pointsCount].y = yc;
            walkPoints[pointsCount].direction = newDirection;
            pointsCount++;
        }
    } else {
        const float slopeX = float(y1 - y2) / float(x1 - x2);
        const int incr = x1 > x2 ? -4 : 4;
        for (int xc = x1; xc * incr < x2 * incr; xc += incr) {
            walkPoints[pointsCount].x = xc;
            walkPoints[pointsCount].y = y1 + (xc - x1) * slopeX;
            walkPoints[pointsCount].direction = newDirection;
            pointsCount++;
        }
    }

    walkPoints[pointsCount].x = x2;
    walkPoints[pointsCount].y = y2;
    walkPoints[pointsCount].direction = newDirection;
    pointsCount++;

    return pointsCount;
}

bool WaynesWorldEngine::walkTo(int actor1_destX, int actor1_destY, int direction, int actor2_destX, int actor2_destY) {
    WalkPoint *actor1Points = 0, *actor2Points = 0;
    int flag1, flag2;
    int actor1X, actor1Y, actor2X, actor2Y;
    int actor2WalkDestX, actor2WalkDestY;

	debug("walkTo(%d, %d, %d, %d, %d)", actor1_destX, actor1_destY, direction, actor2_destX, actor2_destY);

    _isTextVisible = false;

    if (_currentActorNum != 0) {
        actor1Points = _wayneWalkPoints;
        actor2Points = _garthWalkPoints;
        flag1 = 0;
        flag2 = 1;
        actor1X = _wayneSpriteX;
        actor1Y = _wayneSpriteY;
        actor2X = _garthSpriteX;
        actor2Y = _garthSpriteY;
    } else {
        actor1Points = _garthWalkPoints;
        actor2Points = _wayneWalkPoints;
        flag1 = 1;
        flag2 = 0;
        actor1X = _garthSpriteX;
        actor1Y = _garthSpriteY;
        actor2X = _wayneSpriteX;
        actor2Y = _wayneSpriteY;
    }

    if (!walkAdjustDestPoint(actor1_destX, actor1_destY))
        return false;

    if ((actor1X == actor1_destX && actor1Y == actor1_destY) || actor1X == -1) {
        refreshActors();
        return true;
    }

    if (actor2X == -1) {
        // Other actor is not visible
        actor2WalkDestX = actor2X;
        actor2WalkDestY = actor2Y;
    } else if (actor2_destX == -1) {
        // Other actor follows first actor
        actor2WalkDestX = actor1_destX;
        actor2WalkDestY = actor1_destY;
        walkCalcOtherActorDest(flag2, actor2WalkDestX, actor2WalkDestY);
    } else {
        // Other actor has coordinates specified
        actor2WalkDestX = actor2_destX;
        actor2WalkDestY = actor2_destY;
        walkAdjustDestPoint(actor2WalkDestX, actor2WalkDestY);
    }

    int actor1PointsCount, actor2PointsCount;

    actor1PointsCount = walkCalcPath(flag1, actor1X, actor1Y, actor1_destX, actor1_destY, 0);

    if (actor2X != actor2WalkDestX || actor2Y != actor2WalkDestY) {
        actor2PointsCount = walkCalcPath(flag2, actor2X, actor2Y, actor2WalkDestX, actor2WalkDestY, 0);
    } else {
        actor2PointsCount = 0;
    }

    if (actor1PointsCount + 20 < actor2PointsCount) {
        actor2PointsCount = actor1PointsCount + 20;
    }

    for (int pointIndex = 0, walkIncr = 0; pointIndex < actor1PointsCount || pointIndex < actor2PointsCount; pointIndex += 2, walkIncr += 2) {
        int scale = 0;
        if (_word_306DD > 0) {
            // TODO scrollRoom(false);
        }
        if (pointIndex < actor2PointsCount && pointIndex < actor1PointsCount) {
            scale = drawActors(actor1Points[pointIndex].direction, 0, 0, (walkIncr % 8) / 2, _wayneWalkPoints[pointIndex].x - _from_x1, _wayneWalkPoints[pointIndex].y, _garthWalkPoints[pointIndex].x - _from_x1, _garthWalkPoints[pointIndex].y);
        } else if (pointIndex < actor1PointsCount) {
            if (flag1 == 0) {
                scale = drawActors(actor1Points[pointIndex].direction, 0, 1, (walkIncr % 8) / 2, _wayneWalkPoints[pointIndex].x - _from_x1, _wayneWalkPoints[pointIndex].y, _garthSpriteX, _garthSpriteY);
            } else {
                scale = drawActors(actor1Points[pointIndex].direction, 1, 0, (walkIncr % 8) / 2, _wayneSpriteX, _wayneSpriteY, _garthWalkPoints[pointIndex].x - _from_x1, _garthWalkPoints[pointIndex].y);            
            }
        } else {
            if (flag2 == 0) {
                scale = drawActors(actor2Points[pointIndex].direction, 0, 1, (walkIncr % 8) / 2, _wayneWalkPoints[pointIndex].x - _from_x1, _wayneWalkPoints[pointIndex].y, _garthSpriteX, _garthSpriteY);
            } else {
                scale = drawActors(actor2Points[pointIndex].direction, 1, 0, (walkIncr % 8) / 2, _wayneSpriteX, _wayneSpriteY, _garthWalkPoints[pointIndex].x - _from_x1, _garthWalkPoints[pointIndex].y);            
            }
        }
        if (_inventoryItemsCount == 0 && _from_x1 == 0) {
            waitMillis(10000 / (scale * scale) * 10);
        }
        if (_word_306DD == 0) {
            // TOOD
            // if (updateGame()) {
            //     return true;
            // }
        }
    }

    if (direction == -1) {
        if (actor1PointsCount > 0) {
            direction = actor1Points[actor1PointsCount - 1].direction;
        } else {
            direction = 0;
        }
    }

    if (_currentActorNum != 0) {
        drawActors(direction, 1, 1, 0, actor1_destX - _from_x1, actor1_destY, actor2WalkDestX - _from_x1, actor2WalkDestY);
    } else {
        drawActors(direction, 1, 1, 0, actor2WalkDestX - _from_x1, actor2WalkDestY, actor1_destX - _from_x1, actor1_destY);
    }

    // TODO
    // Finish background scrolling
    // while (_word_306DD > 0) {
    //     scrollRoom(true);
    // }

    return true;
}

} // End of namespace WaynesWorld
