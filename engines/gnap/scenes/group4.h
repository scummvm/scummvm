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

#ifndef GNAP_GROUP4_H
#define GNAP_GROUP4_H

#include "gnap/debugger.h"

namespace Gnap {

enum {
	kHS40Platypus			= 0,
	kHS40ExitCave			= 1,
	kHS40ExitToyStand		= 2,
	kHS40ExitBBQ			= 3,
	kHS40ExitUfo			= 4,
	kHS40ExitKissinBooth	= 5,
	kHS40ExitDancefloor		= 6,
	kHS40ExitShoe			= 7,
	kHS40Device				= 8
};

enum {
	kHS41Platypus	= 0,
	kHS41ExitCave	= 1,
	kHS41Exit		= 2,
	kHS41ExitBBQ	= 3,
	kHS41ToyVendor	= 4,
	kHS41Kid		= 5,
	kHS41ToyUfo		= 6,
	kHS41Device		= 7,
	kHS41WalkArea1	= 8
};

enum {
	kHS41UfoExitLeft	= 1,
	kHS41UfoExitRight	= 2,
	kHS41UfoDevice		= 3,
	kHS41UfoWalkArea1	= 4
};

enum {
	kHS42Platypus		= 0,
	kHS42ExitUfoParty	= 1,
	kHS42ExitToyStand	= 2,
	kHS42ExitUfo		= 3,
	kHS42BBQVendor		= 4,
	kHS42ChickenLeg		= 5,
	kHS42Device			= 6,
	kHS42WalkArea1		= 7,
	kHS42WalkArea2		= 8
};

enum {
	kHS42UfoExitLeft	= 1,
	kHS42UfoExitRight	= 2,
	kHS42UfoHotSauce	= 3,
	kHS42UfoDevice		= 4
};

enum {
	kHS43Platypus			= 0,
	kHS43Device				= 1,
	kHS43ExitUfoParty		= 2,
	kHS43ExitBBQ			= 3,
	kHS43ExitKissinBooth	= 4,
	kHS43TwoHeadedGuy		= 5,
	kHS43Key				= 6,
	kHS43Ufo				= 7,
	kHS43WalkArea1			= 8,
	kHS43WalkArea2			= 9
};

enum {
	kHS43UfoExitLeft	= 1,
	kHS43UfoExitRight	= 2,
	kHS43UfoKey			= 3,
	kHS43UfoBucket		= 4,
	kHS43UfoDevice		= 5
};

enum {
	kHS44Platypus		= 0,
	kHS44ExitUfoParty	= 1,
	kHS44ExitUfo		= 2,
	kHS44ExitShow		= 3,
	kHS44KissingLady	= 4,
	kHS44Spring			= 5,
	kHS44SpringGuy		= 6,
	kHS44Device			= 7,
	kHS44WalkArea1		= 8,
	kHS44WalkArea2		= 9
};

enum {
	kHS44UfoExitLeft	= 1,
	kHS44UfoExitRight	= 2,
	kHS44UfoDevice		= 3
};

enum {
	kHS45Platypus		= 0,
	kHS45ExitUfoParty	= 1,
	kHS45ExitShoe		= 2,
	kHS45ExitRight		= 3,
	kHS45ExitDiscoBall	= 4,
	kHS45DiscoBall		= 5,
	kHS45Device			= 6,
	kHS45WalkArea1		= 7
};

enum {
	kHS45UfoExitLeft	= 1,
	kHS45UfoExitRight	= 2,
	kHS45UfoDevice		= 3
};

enum {
	kHS46Platypus			= 0,
	kHS46ExitUfoParty		= 1,
	kHS46ExitKissinBooth	= 2,
	kHS46ExitDisco			= 3,
	kHS46SackGuy			= 4,
	kHS46ItchyGuy			= 5,
	kHS46Device				= 6,
	kHS46WalkArea1			= 7
};

enum {
	kHS46UfoExitLeft	= 1,
	kHS46UfoExitRight	= 2,
	kHS46UfoDevice		= 3
};

enum {
	kAS41LeaveScene					= 0,
	kAS41UseQuarterWithToyVendor	= 1,
	kAS41TalkToyVendor				= 2,
	kAS41UseGumWithToyUfo			= 3,
	kAS41UseChickenBucketWithKid	= 4,
	kAS41GrabKid					= 5,
	kAS41GiveBackToyUfo				= 6,
	kAS41ToyUfoLeaveScene			= 7,
	kAS41ToyUfoRefresh				= 8,
	kAS41UfoGumAttached				= 9
};

enum {
	kAS42LeaveScene						= 0,
	kAS42TalkBBQVendor					= 1,
	kAS42UseQuarterWithBBQVendor		= 2,
	kAS42UseQuarterWithBBQVendorDone	= 3,
	kAS42GrabChickenLeg					= 4,
	kAS42ToyUfoLeaveScene				= 5,
	kAS42ToyUfoRefresh					= 6,
	kAS42ToyUfoPickUpHotSauce			= 7
};

/*****************************************************************************/

class GnapEngine;
class CutScene;

class Scene40: public Scene {
public:
	Scene40(GnapEngine *vm);
	~Scene40() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};
};

class Scene41: public Scene {
public:
	Scene41(GnapEngine *vm);
	~Scene41() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currKidSequenceId;
	int _nextKidSequenceId;
	int _currToyVendorSequenceId;
	int _nextToyVendorSequenceId;
};

class Scene42: public Scene {
public:
	Scene42(GnapEngine *vm);
	~Scene42() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currBBQVendorSequenceId;
	int _nextBBQVendorSequenceId;
};

class Scene43: public Scene {
public:
	Scene43(GnapEngine *vm);
	~Scene43() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currTwoHeadedGuySequenceId;
	int _nextTwoHeadedGuySequenceId;
};

class Scene44: public Scene {
public:
	Scene44(GnapEngine *vm);
	~Scene44() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _nextSpringGuySequenceId;
	int _nextKissingLadySequenceId;
	int _currSpringGuySequenceId;
	int _currKissingLadySequenceId;
};

class Scene45: public Scene {
public:
	Scene45(GnapEngine *vm);
	~Scene45() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currDancerSequenceId;
};

class Scene46: public Scene {
public:
	Scene46(GnapEngine *vm);
	~Scene46() override {}

	int init() override;
	void updateHotspots() override;
	void run() override;
	void updateAnimations() override;
	void updateAnimationsCb() override {};

private:
	int _currSackGuySequenceId;
	int _nextItchyGuySequenceId;
	int _nextSackGuySequenceId;
	int _currItchyGuySequenceId;
};

} // End of namespace Gnap

#endif // GNAP_GROUP4_H
