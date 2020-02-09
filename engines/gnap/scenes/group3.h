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

#ifndef GNAP_GROUP3_H
#define GNAP_GROUP3_H

#include "gnap/debugger.h"

namespace Gnap {

enum {
	kHS30Platypus		= 0,
	kHS30PillMachine	= 1,
	kHS30Device			= 2,
	kHS30ExitCircus		= 3,
	kHS30WalkArea1		= 4
};

enum {
	kHS31Platypus			= 0,
	kHS31MeasuringClown		= 1,
	kHS31BeerBarrel			= 2,
	kHS31Device				= 3,
	kHS31ExitCircus			= 4,
	kHS31ExitOutsideClown	= 5,
	kHS31WalkArea1			= 6
};

enum {
	kHS32Platypus	= 0,
	kHS32ExitTruck	= 1,
	kHS32Device		= 2,
	kHS32WalkArea1	= 3,
	kHS32WalkArea2	= 4,
	kHS32WalkArea3	= 5,
	kHS32WalkArea4	= 6,
	kHS32WalkArea5	= 7,
	kHS32WalkArea6	= 8,
	kHS32WalkArea7	= 9,
	kHS32WalkArea8	= 10
};

enum {
	kHS33Platypus	= 0,
	kHS33Chicken	= 1,
	kHS33Device		= 2,
	kHS33ExitHouse	= 3,
	kHS33ExitBarn	= 4,
	kHS33ExitCreek	= 5,
	kHS33ExitPigpen	= 6,
	kHS33WalkArea1	= 7,
	kHS33WalkArea2	= 8
};

enum {
	kHS38Platypus		= 0,
	kHS38ExitHouse		= 1,
	kHS38ExitCave		= 2,
	kHS38TrapDoorLid1	= 3,
	kHS38TrapDoorLid2	= 4,
	kHS38HuntingTrophy	= 5,
	kHS38WalkArea1		= 6,
	kHS38Device			= 7,
	kHS38WalkArea2		= 8,
	kHS38WalkArea3		= 9,
	kHS38WalkArea4		= 10,
	kHS38WalkArea5		= 11,
	kHS38WalkArea6		= 12
};

enum {
	kHS39Platypus			= 0,
	kHS39ExitInsideHouse	= 1,
	kHS39ExitUfoParty		= 2,
	kHS39Sign				= 3,
	kHS39Device				= 4,
	kHS39WalkArea1			= 5,
	kHS39WalkArea2			= 6
};

enum {
	kAS30LeaveScene			= 0,
	kAS30UsePillMachine		= 1,
	kAS30UsePillMachine2	= 2,
	kAS30LookPillMachine	= 3,
	kAS30UsePillMachine3	= 4,
	kAS30UsePillMachine4	= 5
};

enum {
	kAS31UseBeerBarrel					= 1,
	kAS31FillEmptyBucketWithBeer		= 2,
	kAS31FillEmptyBucketWithBeerDone	= 3,
	kAS31PlatMeasuringClown				= 4,
	kAS31UseMeasuringClown				= 5,
	kAS31LeaveScene						= 6
};

enum {
	kAS32LeaveScene	= 0
};

enum {
	kAS33LeaveScene		= 0,
	kAS33TalkChicken	= 1,
	kAS33UseChicken		= 2,
	kAS33UseChickenDone	= 3
};

enum {
	kAS38LeaveScene					= 0,
	kAS38ExitCave					= 1,
	kAS38UseHuntingTrophy			= 2,
	kAS38HoldingHuntingTrophy		= 3,
	kAS38ReleaseHuntingTrophy		= 4,
	kAS38UsePlatypusWithTrapDoor	= 5,
	kAS38PlatypusHoldingTrapDoor	= 6
};

enum {
	kAS39LeaveScene	= 0
};

/*****************************************************************************/

class GnapEngine;
class CutScene;

class Scene30: public Scene {
public:
	Scene30(GnapEngine *vm);
	~Scene30() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _kidSequenceId;
};

class Scene31: public Scene {
public:
	Scene31(GnapEngine *vm);
	~Scene31() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	bool _beerGuyDistracted;
	int _currClerkSequenceId;
	int _nextClerkSequenceId;
	int _clerkMeasureCtr;
	int _clerkMeasureMaxCtr;
};

class Scene32: public Scene {
public:
	Scene32(GnapEngine *vm);
	~Scene32() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};
};

class Scene33: public Scene {
public:
	Scene33(GnapEngine *vm);
	~Scene33() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currChickenSequenceId;
	int _nextChickenSequenceId;
};

class Scene38: public Scene {
public:
	Scene38(GnapEngine *vm);
	~Scene38() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};
};

class Scene39: public Scene {
public:
	Scene39(GnapEngine *vm);
	~Scene39() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currGuySequenceId;
	int _nextGuySequenceId;
};

} // End of namespace Gnap

#endif // GNAP_GROUP3_H
