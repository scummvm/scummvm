#include <PalmOS.h>
#include "b_globals.h"

#include "sw1_staticres.h"

#define	MAX_ROOMS_PER_FX	7

#define FX_SPOT 1
#define FX_LOOP 2
#define FX_RANDOM 3

typedef struct  {
	int32 roomNo, leftVol, rightVol;
} RoomVol;

typedef struct {
	uint32 sampleId, type, delay;
	RoomVol roomVolList[MAX_ROOMS_PER_FX];
} FxDef;

static void addStaticres_fxList() {
	FxDef _fxList[312] = {
			// 0
		{
			0,						// sampleId
			0,						// type 			(FX_LOOP, FX_RANDOM or FX_SPOT)
			0,						// delay 			(random chance for FX_RANDOM sound fx)
			{							// roomVolList
				{0,0,0},		// {roomNo,leftVol,rightVol}
			},
		},
		//------------------------
		// 1 Newton's cradle. Anim=NEWTON.
		{
			FX_NEWTON,		// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance)
			{							// roomVolList
				{45,4,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 2
		{
			FX_TRAFFIC2,	// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{1,12,12},	// {roomNo,leftVol,rightVol}
				{2,1,1},
				{3,1,1},
				{4,13,13},
				{5,1,1},
				{8,7,7},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 3
		{
			FX_HORN1,			// sampleId
			FX_RANDOM,		// type
			1200,					// delay (or random chance)
			{							// roomVolList
				{1,3,3},		// {roomNo,leftVol,rightVol}
				{3,1,1},
				{4,1,1},
				{5,2,2},
				{8,4,4},
				{18,2,3},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 4
		{
			FX_HORN2,			// sampleId
			FX_RANDOM,		// type
			1200,					// delay (or random chance)
			{							// roomVolList
				{1,4,4},		// {roomNo,leftVol,rightVol}
				{3,2,2},
				{4,3,3},
				{5,2,2},
				{8,4,4},
				{18,1,1},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 5
		{
			FX_HORN3,			// sampleId
			FX_RANDOM,		// type
			1200,					// delay (or random chance)
			{							// roomVolList
				{1,4,4},		// {roomNo,leftVol,rightVol}
				{2,4,4},
				{3,2,2},
				{4,3,3},
				{5,2,2},
				{8,4,4},
				{18,1,1},
			},
		},
		//------------------------
		// 6
		{
			FX_CAMERA1,		// sampleId
			FX_SPOT,			// type
			25,						// delay (or random chance)
			{							// roomVolList
				{1,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 7
		{
			FX_CAMERA2,		// sampleId
			FX_SPOT,			// type
			25,						// delay (or random chance)
			{							// roomVolList
				{1,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 8
		{
			FX_CAMERA3,		// sampleId
			FX_SPOT,			// type
			25,						// delay (or random chance)
			{							// roomVolList
				{1,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 9
		{
			FX_SWATER1,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{7,12,12},	// {roomNo,leftVol,rightVol}
				{6,12,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 10 Mad dogs in Spain, triggered by George going around the corner in the villa hall.
		// In 56 and 57, the dogs will continue barking after George has either been ejected or sneaked up stairs
		// for a couple of loops before stopping.
		{
			FX_DOGS56,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{60,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0}			// NULL-TERMINATOR
			},
		},
		//------------------------
		// 11
		{
			FX_DRIP1,			// sampleId
			FX_RANDOM,		// type
			20,						// delay (or random chance)
			{							// roomVolList
				{7,15,15},	// {roomNo,leftVol,rightVol}
				{6,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 12
		{
			FX_DRIP2,			// sampleId
			FX_RANDOM,		// type
			30,						// delay (or random chance)
			{							// roomVolList
				{7,15,15},	// {roomNo,leftVol,rightVol}
				{6,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 13
		{
			FX_DRIP3,			// sampleId
			FX_RANDOM,		// type
			40,						// delay (or random chance)
			{							// roomVolList
				{7,15,15},	// {roomNo,leftVol,rightVol}
				{6,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 14
		{
			FX_TWEET1,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{1,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 15
		{
			FX_TWEET2,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{1,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 16
		{
			FX_TWEET3,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{1,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 17
		{
			FX_TWEET4,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{1,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 18
		{
			FX_TWEET5,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{1,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 19 Tied to large bird flying up screen anim 
		{
			FX_CAW1,			// sampleId
			FX_SPOT,			// type
			20,						// delay (or random chance)
			{							// roomVolList
				{1,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 20 George picking the canopy up: GEOCAN 
		{
			FX_CANUP,			// sampleId
			FX_SPOT,			// type
			5,						// delay (or random chance) *
			{							// roomVolList
				{1,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 21 George dropping the canopy: GEOCAN 
		{
			FX_CANDO,			// sampleId
			FX_SPOT,			// type
			52,						// delay (or random chance) *
			{							// roomVolList
				{1,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 22 George dusts himself down: GEOCAN 
		{
			FX_DUST,			// sampleId
			FX_SPOT,			// type
			58,						// delay (or random chance) *
			{							// roomVolList
				{1,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 23 George picks up the paper and opens it: GEOPAP
		{
			FX_PAP1,			// sampleId
			FX_SPOT,			// type
			23,						// delay (or random chance) *
			{							// roomVolList
				{1,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 24 George puts the paper away: GEOPAP2
		{
			FX_PAP2,			// sampleId
			FX_SPOT,			// type
			3,						// delay (or random chance) *
			{							// roomVolList
				{1,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 25 George gives the paper away: GEOWRK8
		{
			FX_PAP3,			// sampleId
			FX_SPOT,			// type
			13,						// delay (or random chance) *
			{							// roomVolList
				{4,14,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 26 Workman examines paper: WRKOPN - it's now just WRKPPR
		{
			FX_PAP4,			// sampleId
			FX_SPOT,			// type
			15,						// delay (or random chance) *
			{							// roomVolList
				{4,14,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 27 Workman puts paper down: WRKOPN (REVERSED) - now just WRKCLM
		{
			FX_PAP5,			// sampleId
			FX_SPOT,			// type
			2,						// delay (or random chance)*
			{							// roomVolList
				{4,14,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 28 Pickaxe sound 1:, Screen 4 - WRKDIG
		{
			FX_PICK1,			// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{4,10,10},
				{0,0,0}			// NULL-TERMINATOR
			},
		},
		//------------------------
		// 29 Pickaxe sound 2:, Screen 4 - WRKDIG 
		{
			FX_PICK2,			// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{4,10,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 30 Pickaxe sound 3:, Screen 4 - WRKDIG 
		{
			FX_PICK3,			// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{4,10,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 31 Pickaxe sound 4:, Screen 4 - WRKDIG 
		{
			FX_PICK4,			// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{4,10,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 32 Shorting light: FLICKER
		{
			FX_LIGHT,			// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{3,15,15},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 33 Cat leaps out of bin and runs: CATJMP!
		{
			FX_CAT,				// sampleId
			FX_SPOT,			// type
			20,						// delay (or random chance) *
			{							// roomVolList
				{2,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 34 George rocks plastic crate: GEOCRT
		{
			FX_CRATE,			// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance) *
			{							// roomVolList
				{2,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 35 George tries to climb drainpipe: GEOCLM02
		{
			FX_DRAIN,			// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{2,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 36 George removes manhole cover: GEOMAN8
		{
			FX_HOLE,			// sampleId
			FX_SPOT,			// type
			19,						// delay (or random chance) ?
			{							// roomVolList
				{2,12,11},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 37 Brandy bottle put down: CHNDRN
		{
			FX_BOTDN,			// sampleId
			FX_SPOT,			// type
			43,						// delay (or random chance) *
			{							// roomVolList
				{3,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 38 Brandy bottle picked up: GEOBOT3
		{
			FX_BOTUP,			// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{3,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 39 Chantelle gulps on brandy: CHNDRN
		{
			FX_GULP,			// sampleId
			FX_SPOT,			// type
			23,						// delay (or random chance) *
			{							// roomVolList
				{3,4,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 40 Chantelle picked up off the floor: GEOCHN
		{
			FX_PIKUP,			// sampleId
			FX_SPOT,			// type
			28,						// delay (or random chance) *
			{							// roomVolList
				{3,11,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 41 George searches Plantard's body: GEOCPS
		{
			FX_BODY,			// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance) *
			{							// roomVolList
				{3,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 42 Moue cocks handgun. MOUENT
		{
			FX_PISTOL,		// sampleId
			FX_SPOT,			// type
			23,						// delay (or random chance) *
			{							// roomVolList
				{4,4,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 43 George rummages in toolbox: GEOTBX
		{
			FX_TBOX,			// sampleId
			FX_SPOT,			// type
			12,						// delay (or random chance) *
			{							// roomVolList
				{4,12,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 44 rat squeak 1
		{
			FX_RAT1,			// sampleId
			FX_RANDOM,		// type
			193,					// delay (or random chance)
			{							// roomVolList
				{6,5,7},		// {roomNo,leftVol,rightVol}
				{7,5,3},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 45 rat squeak 2
		{
			FX_RAT2,			// sampleId
			FX_RANDOM,		// type
			201,					// delay (or random chance)
			{							// roomVolList
				{6,3,5},		// {roomNo,leftVol,rightVol}
				{7,4,6},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 46 George climbs down ladder:
		{
			FX_LADD1,			// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{6,10,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 47 Rushing water loop
		{
			FX_SWATER3,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{6,10,11},	// {roomNo,leftVol,rightVol}
				{7,12,11},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 48 Left hand bin being opened: GEOCAT?
		{
			FX_BIN3,			// sampleId
			FX_SPOT,			// type
			12,						// delay (or random chance)
			{							// roomVolList
				{2,12,11},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 49 Middle bin being opened: GEOBIN
		{
			FX_BIN2,			// sampleId
			FX_SPOT,			// type
			12,						// delay (or random chance)
			{							// roomVolList
				{2,11,11},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 50 Right hand bin being opened: GEOLID?
		{
			FX_BIN1,			// sampleId
			FX_SPOT,			// type
			12,						// delay (or random chance)
			{							// roomVolList
				{2,10,11},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 51 Passing car sound
		{
			FX_CARS,			// sampleId
			FX_RANDOM,		// type
			120,					// delay (or random chance)
			{							// roomVolList
				{10,8,1},
				{12,7,7},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 52 Passing car sound
		{
			FX_FIESTA,		// sampleId
			FX_RANDOM,		// type
			127,					// delay (or random chance)
			{							// roomVolList
				{10,8,1},
				{12,7,7},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 53 Passing car sound
		{
			FX_CARLTON ,	// sampleId
			FX_RANDOM,		// type
			119,					// delay (or random chance)
			{							// roomVolList
				{10,8,1},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 54 Bird
		{
			FX_BIRD,			// sampleId
			FX_RANDOM,		// type
			500,					// delay (or random chance)
			{							// roomVolList
				{9,10,10},	// {roomNo,leftVol,rightVol}
				{10,2,1},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 55 George tries the door: GEOTRY
		{
			FX_DOORTRY,		// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance)
			{							// roomVolList
				{9,9,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 56 George opens the door: GEODOOR9
		{
			FX_FLATDOOR,	// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{9,9,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 57 George picks the 'phone up: GEOPHN10
		{
			FX_FONEUP,		// sampleId
			FX_SPOT,			// type
			15,						// delay (or random chance)
			{							// roomVolList
				{10,9,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 58 George puts the 'phone down: GEPDWN10
		{
			FX_FONEDN,		// sampleId
			FX_SPOT,			// type
			4,						// delay (or random chance)
			{							// roomVolList
				{10,9,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 59 Albert opens the door: ALBOPEN
		{
			FX_ALBOP,			// sampleId
			FX_SPOT,			// type
			13,						// delay (or random chance)
			{							// roomVolList
				{5,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 60 Albert closes the door: ALBCLOSE
		{
			FX_ALBCLO,		// sampleId
			FX_SPOT,			// type
			20,						// delay (or random chance)
			{							// roomVolList
				{5,9,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 61 George enter Nico's flat. GEOENT10
		{
			FX_NICOPEN,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{10,7,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 62 George leaves Nico's. GEOLVS10
		{
			FX_NICLOSE,		// sampleId
			FX_SPOT,			// type
			13,						// delay (or random chance)
			{							// roomVolList
				{10,7,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 63 Another bird for the street. 
		{
			FX_BIRD2,			// sampleId
			FX_RANDOM,		// type
			500,					// WAS 15 (TOO LATE)
			{							// roomVolList
				{9,10,10},	// {roomNo,leftVol,rightVol}
				{10,2,1},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 64 George sits in the chair: GEOCHR 
		{
			FX_GEOCHAIR,	// sampleId
			FX_SPOT,			// type
			14,						// delay (or random chance)
			{							// roomVolList
				{10,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 65 George sits on the couch: GEOCCH 
		{
			FX_GEOCCH,		// sampleId
			FX_SPOT,			// type
			14,						// delay (or random chance)
			{							// roomVolList
				{10,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 66 George gets up from the chair:  GEOCHR9
		{
			FX_GEOCHR9,		// sampleId
			FX_SPOT,			// type
			5,						// delay (or random chance)
			{							// roomVolList
				{10,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 67 George is electrocuted: COSSHK
		{
			FX_SHOCK1,		// sampleId
			FX_SPOT,			// type
			19,						// delay (or random chance)
			{							// roomVolList
				{11,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 68 George plays record: GEOWIND
		{
			FX_GRAMOFON,	// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{11,11,13},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 69 George is frisked: GORFRK
		{
			FX_FRISK,			// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance)
			{							// roomVolList
				{12,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 70 Traffic sound
		{
			FX_TRAFFIC3,	// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{11,5,4},
				{12,1,1},
				{16,4,4},
				{18,2,3},
				{46,4,3},
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 71 Latvian reading: LATRDS
		{
			FX_PAPER6,		// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance)
			{							// roomVolList
				{13,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 72 Deskbell
		{
			FX_DESKBELL,	// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{13,10,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 73 George picks up hotel 'phone: GEOTEL
		{
			FX_PHONEUP2,	// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{13,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 74 George puts down hotel 'phone: GEOTEL9
		{
			FX_PHONEDN2,	// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{13,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 75 George tries doors in corridor: GEODOR
		{
			FX_TRYDOR14,	// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{14,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 76 George opens bedside cabinet: BEDDOR
		{
			FX_CABOPEN,		// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance)
			{							// roomVolList
				{15,10,14},	// {roomNo,leftVol,rightVol}
				{17,10,14},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 77 George closes bedside cabinet: BEDDOR (reversed)
		{
			FX_CABCLOSE,	// sampleId
			FX_SPOT,			// type
			5,						// delay (or random chance)
			{							// roomVolList
				{15,10,14},	// {roomNo,leftVol,rightVol}
				{17,10,14},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 78 George opens the window: WINDOW
		{
			FX_WINDOPEN,	// sampleId
			FX_SPOT,			// type
			19,						// delay (or random chance)
			{							// roomVolList
				{15,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 79 George goes right along the ledge: GEOIRW
		{
			FX_LEDGE1,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{16,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 80 George goes left along the ledge: GEOILW
		{
			FX_LEDGE2,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{16,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 81 Pigeon noises
		{
			FX_COO,				// sampleId
			FX_RANDOM,		// type
			80,						// delay (or random chance)
			{							// roomVolList
				{16,7,9},		// {roomNo,leftVol,rightVol}
				{46,5,4},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 82 Pigeon noises
		{
			FX_COO2,			// sampleId
			FX_RANDOM,		// type
			60,						// delay (or random chance)
			{							// roomVolList
				{15,3,4},		// {roomNo,leftVol,rightVol}
				{16,8,5},		// {roomNo,leftVol,rightVol}
				{17,3,4},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 83 George picks up and opens case: GEOBFC
		{
			FX_BRIEFON,		// sampleId
			FX_SPOT,			// type
			16,						// delay (or random chance)
			{							// roomVolList
				{17,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 84 George closes and puts down case: GEOBFC (reversed)
		{
			FX_BRIEFOFF,	// sampleId
			FX_SPOT,			// type
			12,						// delay (or random chance)
			{							// roomVolList
				{17,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 85 George gets into wardrobe. GEOWRB2 Attention, James. This is new as of 15/7/96
		{
			FX_WARDIN,		// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance)
			{							// roomVolList
				{17,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 86 George gets out of wardrobe. GEOWRB2  (Reversed). Attention, James. This is new as of 15/7/96
		{
			FX_WARDOUT,		// sampleId
			FX_SPOT,			// type
			41,						// delay (or random chance)
			{							// roomVolList
				{17,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 87 George jumps in through window: GEOWIN2
		{
			FX_JUMPIN,		// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance)
			{							// roomVolList
				{15,8,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 88 George climbs in: GEOWIN2/GEOWIN8
		{
			FX_CLIMBIN,		// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{17,8,16},	// {roomNo,leftVol,rightVol}
				{15,8,16},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 89 George climbs out: GEOWIN1/GEOWIN9
		{
			FX_CLIMBOUT,	// sampleId
			FX_SPOT,			// type
			17,						// delay (or random chance)
			{							// roomVolList
				{17,9,10},	// {roomNo,leftVol,rightVol}
				{15,9,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 90 George picks the 'phone up: GEOTEL18
		{
			FX_FONEUP,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{18,4,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 91 George puts the 'phone down: GEOTL18A
		{
			FX_FONEDN,		// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance)
			{							// roomVolList
				{18,4,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 92 George tries to get keys. GEOKEY
		{
			FX_KEY13,			// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance)
			{							// roomVolList
				{13,3,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 93 George manages to get keys. GEOKEY13
		{
			FX_KEY13,			// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance)
			{							// roomVolList
				{13,3,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 94 George electrocutes Maguire: MAGSHK
		{
			FX_SHOCK2,		// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance)
			{							// roomVolList
				{19,9,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 95 George opens dray door : GEOTRP8
		{
			FX_TRAPOPEN,	// sampleId
			FX_SPOT,			// type
			20,						// delay (or random chance)
			{							// roomVolList
				{19,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 96 George breaks switch : Which anim?
		{
			FX_SWITCH19,	// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{19,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 97 Leary pulls pint: LESPMP
		{
			FX_PULLPINT,	// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance)
			{							// roomVolList
				{20,10,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 98 Glasswasher fuse blows (and the glass washer grinds to a halt)
		{
			FX_FUSE20,		// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{20,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 99 Fitz leaps to his feet: FTZSTD
		{
			FX_FITZUP,		// sampleId
			FX_SPOT,			// type
			5,						// delay (or random chance)
			{							// roomVolList
				{20,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 100 Fitz runs for it: FTZRUN
		{
			FX_FITZRUN,		// sampleId
			FX_SPOT,			// type
			15,						// delay (or random chance)
			{							// roomVolList
				{20,12,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 101 George pulls lever: GEOLVR & GEOLVR26
		{
			FX_LEVER,			// sampleId
			FX_SPOT,			// type
			26,						// delay (or random chance)
			{							// roomVolList
				{21,8,10},	// {roomNo,leftVol,rightVol}
				{26,8,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 102 George pulls lever: GEOLVR8 & GEOLVR08
		{
			FX_LEVER2,		// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance)
			{							// roomVolList
				{21,8,10},	// {roomNo,leftVol,rightVol}
				{26,8,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 103 George opens tap: No idea what the anim is
		{
			FX_TAP,				// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{21,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 104 George closes tap: No idea what this anim is either
		{
			FX_TAP2,			// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{21,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 105 Bar flap: FLPOPN
		{
			FX_BARFLAP,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{20,6,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 106 Farmer leaves: FRMWLK
		{
			FX_FARMERGO,	// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance)
			{							// roomVolList
				{22,6,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 107 George climbs haystack: GEOCLM
		{
			FX_CLIMBHAY,	// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance)
			{							// roomVolList
				{22,14,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 108 George drives sewer key into wall: GEOKEY23
		{
			FX_KEYSTEP,		// sampleId
			FX_SPOT,			// type
			39,						// delay (or random chance)
			{							// roomVolList
				{23,8,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 109 George climbs over wall: GEOCLM23
		{
			FX_CASTLWAL,	// sampleId
			FX_SPOT,			// type
			17,						// delay (or random chance)
			{							// roomVolList
				{23,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 110 George falls from wall: GEOTRY23
		{
			FX_CLIMBFAL,	// sampleId
			FX_SPOT,			// type
			43,						// delay (or random chance)
			{							// roomVolList
				{23,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 111 Goat chewing: GOTEAT
		{
			FX_GOATCHEW,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{24,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 112 George moves plough: GEOPLW
		{
			FX_PLOUGH,		// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance)
			{							// roomVolList
				{24,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 113 George drops slab: STNFALL 
		{
			FX_SLABFALL,	// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance)
			{							// roomVolList
				{25,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 114 George picks up slab: GEOSTN8 
		{
			FX_SLABUP,		// sampleId
			FX_SPOT,			// type
			29,						// delay (or random chance)
			{							// roomVolList
				{25,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 115 Secret door opens: ALTOPN 
		{
			FX_SECDOR25,	// sampleId
			FX_SPOT,			// type
			17,						// delay (or random chance)
			{							// roomVolList
				{25,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 116 George wrings out cloth: GEOTWL25 
		{
			FX_WRING,			// sampleId
			FX_SPOT,			// type
			24,						// delay (or random chance)
			{							// roomVolList
				{25,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 117 Rat running across barrels: RATJMP 
		{
			FX_RAT3A,			// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{26,8,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 118 Rat running across barrels: RATJMP 
		{
			FX_RAT3B,			// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance)
			{							// roomVolList
				{26,7,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 119 Rat running across barrels: RATJMP 
		{
			FX_RAT3C,			// sampleId
			FX_SPOT,			// type
			26,						// delay (or random chance)
			{							// roomVolList
				{26,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 120 Irish bird song 1:  
		{
			FX_EIRBIRD1,	// sampleId
			FX_RANDOM,		// type
			720,					// delay (or random chance)
			{							// roomVolList
				{19,6,8},		// {roomNo,leftVol,rightVol}
				{21,2,3},
				{22,8,5},
				{23,6,5},
				{24,8,8},
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 121 Irish bird song 2:
		{
			FX_EIRBIRD2,	// sampleId
			FX_RANDOM,		// type
			720,					// delay (or random chance)
			{							// roomVolList
				{19,8,6},		// {roomNo,leftVol,rightVol}
				{21,2,3},
				{22,6,8},
				{23,5,5},
				{24,8,8},
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 122 Irish bird song 3:
		{
			FX_EIRBIRD3,	// sampleId
			FX_RANDOM,		// type
			720,					// delay (or random chance)
			{							// roomVolList
				{19,8,8},		// {roomNo,leftVol,rightVol}
				{21,3,4},		
				{22,8,8},
				{23,5,6},
				{24,6,8},
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 123 Rat 3D: 
		{
			FX_RAT3D,			// sampleId
			FX_RANDOM,		// type
			600,					// delay (or random chance)
			{							// roomVolList
				{26,2,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 124 Wind atop the battlements 
		{
			FX_WIND,			// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{23,6,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 125 Glasswasher in the pub (Room 20) *JEL* Stops after fuse blows and starts when george fixes it. 
		{
			FX_WASHER,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{20,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 126 Running tap in the cellar: (Room 21) *JEL* Only when the tap is on.
		{
			FX_CELTAP,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{21,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 127 Lopez's hose. Basically a loop but stops when George cuts the water supply. Replaces MUTTER1. 
		{
			FX_HOSE57,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{57,3,1},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 128 Lopez's hose being switched off. Anim GARD05. Replaces MUTTER2. 
		{
			FX_HOSE57B,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{57,3,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 129 Nejo bouncing the ball off the door. NEJ8
		{
			FX_BALLPLAY,	// sampleId
			FX_SPOT,			// type
			13,						// delay (or random chance)
			{							// roomVolList
				{45,5,1},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		//------------------------
		// 130 Cricket loop for Syrian desert Only audible in 55 when the cave door is open.
		{
			FX_CRICKET,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{54,8,8},		// {roomNo,leftVol,rightVol}
				{55,3,5},
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 131 Display case shatters: GEOTOTB	
		{
			FX_SMASHGLA,	// sampleId
			FX_SPOT,			// type
			35,						// delay (or random chance)
			{							// roomVolList
				{29,16,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 132 Burglar alarm: Once the case is smashed (see 131)
		{
			FX_ALARM,			// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{28,12,12},	// {roomNo,leftVol,rightVol}
				{29,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 133 Guido fires: GUIGUN
		{
			FX_GUN1,			// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance)
			{							// roomVolList
				{29,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 134 Guido knocked down: NICPUS1
		{
			FX_GUI_HIT,		// sampleId
			FX_SPOT,			// type
			40,						// delay (or random chance)
			{							// roomVolList
				{29,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 135 Museum exterior ambience 
		{
			FX_MUESEXT,		// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{27,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 136 Cat gets nowty: CAT3
		{
			FX_STALLCAT,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{45,10,6},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 137 Cat gets very nowty: CAT5
		{
			FX_CATHIT,		// sampleId
			FX_SPOT,			// type
			4,						// delay (or random chance)
			{							// roomVolList
				{45,10,6},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 138 Desert wind: Only audible in 55 when the cave door is open.
		{
			FX_SYRIWIND,	// sampleId
			FX_RANDOM,		// type
			720,					// delay (or random chance)
			{							// roomVolList
				{54,10,10},	// {roomNo,leftVol,rightVol}
				{55,5,7},
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		// 139 Bell on Nejo's stall: GEOSYR7
		{
			FX_STALLBEL,	// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance)
			{							// roomVolList
				{45,10,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			}
		},
		//------------------------
		//------------------------
		// 140 George electrocutes Khan: GEOSYR40
		{
			FX_SHOCK3,		// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance)
			{							// roomVolList
				{54,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 141 George thumps Khan: GEOSYR40
		{
			FX_THUMP1,		// sampleId
			FX_SPOT,			// type
			22,						// delay (or random chance)
			{							// roomVolList
				{54,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 142 Khan hits the floor: KHS9
		{
			FX_KHANDOWN,	// sampleId
			FX_SPOT,			// type
			24,						// delay (or random chance)
			{							// roomVolList
				{54,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 143 Hospital ambience
		{
			FX_HOSPNOIS,	// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{32,6,4},		// {roomNo,leftVol,rightVol}
				{33,7,7},
				{34,3,4},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 144 Mr Shiny switched on: DOMPLG (Start FX_SHINY)
		{
			FX_SHINYON,		// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{33,12,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 145 Mr Shiny running 
		{
			FX_SHINY,			// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{32,4,3},		// {roomNo,leftVol,rightVol}
				{33,12,14}, 
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 146 Mr Shiny switched off: GEOPLG33 (Turn off FX_SHINY at the same time)
		{
			FX_SHINYOFF,	// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{33,12,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 147 Benoir takes blood pressure: BENBP1 or BENBP2
		{
			FX_BLOODPRE,	// sampleId
			FX_SPOT,			// type
			39,						// delay (or random chance)
			{							// roomVolList
				{34,14,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 148 George takes blood pressure: GEOBP1 or GEOBP2
		{
			FX_BLOODPRE,	// sampleId
			FX_SPOT,			// type
			62,						// delay (or random chance)
			{							// roomVolList
				{34,14,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 149 Goat baas as it attacks: GOTCR and GOTCL
		{
			FX_GOATBAA,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{24,12,12},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 150 Goat peeved at being trapped: GOTPLW (I'd advise triggering this anim randomly if you haven't done that)
		{
			FX_GOATDOH,		// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance)
			{							// roomVolList
				{24,7,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 151 George triggers the Irish secret door: GEOPUT
		{
			FX_TRIGER25,	// sampleId
			FX_SPOT,			// type
			35,						// delay (or random chance)
			{							// roomVolList
				{25,6,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 152 George winds up gramophone: GEOWIND
		{
			FX_WINDUP11,	// sampleId
			FX_SPOT,			// type
			16,						// delay (or random chance)
			{							// roomVolList
				{11,7,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 153 Marib ambience
		{
			FX_MARIB,			// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{45,7,7},		// {roomNo,leftVol,rightVol}
				{47,5,5},
				{50,5,4},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 154 Statuette breaks: STA2
		{
			FX_STATBREK,	// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{45,7,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 155 George opens toilet door: CUBDOR50
		{
			FX_CUBDOR,		// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance)
			{							// roomVolList
				{50,6,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 156 Crowd goes, "Ooh!": CRO36APP
		{
			FX_OOH,				// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance)
			{							// roomVolList
				{36,6,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 157 Phone rings: When Nico calls back in room 41. Loops until the guard answers it. 
		{
			FX_PHONCALL,	// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{41,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 158 Phone picked up in 41: GUA41ANS
		{
			FX_FONEUP41,	// sampleId
			FX_SPOT,			// type
			18,						// delay (or random chance)
			{							// roomVolList
				{41,5,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 159 George turns thermostat: GEO41THE (another dummy). Also used on the reverse. 
		{
			FX_THERMO1,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance)
			{							// roomVolList
				{41,6,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 160 Low echoing rumble of large church 
		{
			FX_CHURCHFX,	// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance)
			{							// roomVolList
				{38,5,5},		// {roomNo,leftVol,rightVol}
				{48,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 161 George drys hand: GEO43HAN 
		{
			FX_DRIER1,		// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance)
			{							// roomVolList
				{43,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 162 George jumps in through window: GEOWIN8
		{
			FX_JUMPIN,		// sampleId
			FX_SPOT,			// type
			49,						// delay (or random chance)
			{							// roomVolList
				{17,8,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 163 Khan fires: KHS12
		{
			FX_SHOTKHAN,	// sampleId
			FX_SPOT,			// type
			30,						// delay (or random chance)
			{							// roomVolList
				{54,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 164 Khan fires: KHS5
		{
			FX_SHOTKHAN,	// sampleId
			FX_SPOT,			// type
			5,						// delay (or random chance)
			{							// roomVolList
				{54,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 165 George falls: GEOSYR37
		{
			FX_GEOFAL54,	// sampleId
			FX_SPOT,			// type
			25,						// delay (or random chance)
			{							// roomVolList
				{54,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 166 George falls after going for the gun (GEOSYR42)
		{
			FX_GEOFAL54,	// sampleId
			FX_SPOT,			// type
			46,						// delay (or random chance)
			{							// roomVolList
				{54,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 167 Pickaxe sound 5: Screen 1 - WRKDIG01
		{
			FX_PICK5,			// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{1,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 168 George climbs ladder in 7: GEOASC07
		{
			FX_SEWLADU7,	// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance) *
			{							// roomVolList
				{7,8,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 169 George picks keys up in Alamut: GEOKEYS1
		{
			FX_KEYS49,		// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{49,8,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 170 George puts down keys up in Alamut: GEOKEYS2
		{
			FX_KEYS49,		// sampleId
			FX_SPOT,			// type
			7,						// delay (or random chance) *
			{							// roomVolList
				{49,8,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 171 George unlocks toilet door: GEOSYR43
		{
			FX_UNLOCK49,	// sampleId
			FX_SPOT,			// type
			16,						// delay (or random chance) *
			{							// roomVolList
				{49,6,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 172 George breaks the toilet chain. GEOSYR48
		{
			FX_WCCHAIN,		// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance) *
			{							// roomVolList
				{50,6,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 173 George breaks the branch of the cliff edge tree. GEOSYR20
		{
			FX_BREKSTIK,	// sampleId
			FX_SPOT,			// type
			16,						// delay (or random chance) *
			{							// roomVolList
				{54,6,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 174 George climbs down the cliff face. GEOSYR23
		{
			FX_CLIMBDWN,	// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance) *
			{							// roomVolList
				{54,6,7},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 175 George pulls ring:  GEOSYR26
		{
			FX_RINGPULL,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{54,7,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 176 Bull's Head door opens: SECDOR
		{
			FX_SECDOR54,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{54,7,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 177 Inside Bull's Head door opens: DOOR55 (and its reverse). 
		{
			FX_SECDOR55,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{55,4,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 178 Ayub opens door. AYU1 
		{
			FX_AYUBDOOR,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{45,8,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 179 George knocks at the door in location 4: GEONOK followed by reverse of GEONOK
		{
			FX_KNOKKNOK,	// sampleId
			FX_SPOT,			// type
			13,						// delay (or random chance) *
			{							// roomVolList
				{4,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 180 George knocks at the door in location 5: GEONOK05
		{
			FX_KNOKKNOK,	// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance) *
			{							// roomVolList
				{5,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 181 Those pesky Irish birds turn up in Spain, too.
		{
			FX_SPNBIRD1,	// sampleId
			FX_RANDOM,		// type
			720,					// delay (or random chance) *
			{							// roomVolList
				{57,1,4},
				{58,8,4},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 182 Those pesky Irish birds turn up in Spain, too.
		{
			FX_SPNBIRD2,	// sampleId
			FX_RANDOM,		// type
			697,					// delay (or random chance) *
			{							// roomVolList
				{57,4,8},		// {roomNo,leftVol,rightVol}
				{58,4,1},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 183 The secret door in the well: SECDOR61 anim
		{
			FX_SECDOR61,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{61,4,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 184 Spanish countryside ambience
		{
			FX_SPAIN,			// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{57,1,2},		// 
				{58,2,2},		//  
				{60,1,1},		//  
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 185 Spanish well ambience
		{
			FX_WELLDRIP,	// sampleId
			FX_LOOP,			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{61,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 186 Fish falls on George's head: GEOTOT29
		{
			FX_FISHFALL,	// sampleId
			FX_SPOT,			// type
			60,						// delay (or random chance) *
			{							// roomVolList
				{29,10,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 187 Hospital exterior ambience
		{
			FX_HOSPEXT,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{31,3,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 188 Hospital exterior gravel footstep #1
		{
			FX_GRAVEL1,		// sampleId
			FX_SPOT, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{31,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 189 Hospital exterior gravel footstep #2
		{
			FX_GRAVEL2,		// sampleId
			FX_SPOT, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{31,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 190 George opens sarcophagus: GEOSAR
		{
			FX_SARCO28A,	// sampleId
			FX_SPOT, 			// type
			26,						// delay (or random chance) *
			{							// roomVolList
				{28,6,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 191 George closes sarcophagus: GEOSAR2
		{
			FX_SARCO28B,	// sampleId
			FX_SPOT, 			// type
			24,						// delay (or random chance) *
			{							// roomVolList
				{28,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 192 Guard opens sarcophagus: MUSOPN
		{
			FX_SARCO28C,	// sampleId
			FX_SPOT, 			// type
			14,						// delay (or random chance) *
			{							// roomVolList
				{28,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 193 George peeks out of sarcophagus: GEOPEEK
		{
			FX_SARCO29,		// sampleId
			FX_SPOT, 			// type
			4,						// delay (or random chance) *
			{							// roomVolList
				{29,5,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 194 The rope drops into the room: ROPE29
		{
			FX_ROPEDOWN,	// sampleId
			FX_SPOT, 			// type
			3,						// delay (or random chance) *
			{							// roomVolList
				{29,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 195 George pushes the totem pole: GEOTOT29
		{
			FX_TOTEM29A,	// sampleId
			FX_SPOT, 			// type
			30,						// delay (or random chance) *
			{							// roomVolList
				{29,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 196 George pushes the totem pole over: GEOTOTB
		{
			FX_TOTEM29B,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{29,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 197 George rocks the totem pole in museum hours: TOTEM28
		{
			FX_TOTEM28A,	// sampleId
			FX_SPOT, 			// type
			8,						// delay (or random chance) *
			{							// roomVolList
				{28,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 198 Ambient sound for Montfauçon Square
		{
			FX_MONTAMB,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{36,6,6},		// {roomNo,leftVol,rightVol}
				{40,6,6},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 199 Ambient sound churchyard. 
		{
			FX_WIND71,		// sampleId
			FX_RANDOM, 		// type
			720,					// delay (or random chance) *
			{							// roomVolList
				{71,10,10},	// {roomNo,leftVol,rightVol}
				{72,7,7},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 200 Owl cry #1 in churchyard
		{
			FX_OWL71A,		// sampleId
			FX_RANDOM, 		// type
			720,					// delay (or random chance) *
			{							// roomVolList
				{71,8,8},		// {roomNo,leftVol,rightVol}
				{72,6,4},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 201 Owl cry #2 in churchyard
		{
			FX_OWL71B,		// sampleId
			FX_RANDOM, 		// type
			1080,					// delay (or random chance) *
			{							// roomVolList
				{71,8,8},		// {roomNo,leftVol,rightVol}
				{72,7,6},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 202 Air conditioner in the museum
		{
			FX_AIRCON28,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{28,6,6},		// {roomNo,leftVol,rightVol}
				{29,3,3},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 203 George breaks the handle off in the church tower. GEOWND72
		{
			FX_COG72A,		// sampleId
			FX_SPOT, 			// type
			5,						// delay (or random chance) *
			{							// roomVolList
				{72,10,10},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 204 Countess' room ambience
		{
			FX_AMBIEN56,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{56,3,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 205 Musical effect for George drinking beer. GEODRN20
		{
			FX_DRINK,			// sampleId
			FX_SPOT, 			// type
			17,						// delay (or random chance) *
			{							// roomVolList
				{20,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 206 Torch thrown through the air. GEOTHROW
		{
			FX_TORCH73,		// sampleId
			FX_SPOT, 			// type
			14,						// delay (or random chance) *
			{							// roomVolList
				{73,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 207 Internal train ambience. 
		{
			FX_TRAININT,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{63,3,3},		// {roomNo,leftVol,rightVol}
				{65,2,2},
				{66,2,2},	
				{67,2,2},
				{69,2,2},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 208 Countess' clock. PENDULUM. Note: Trigger the sound effect on alternate runs of the pendulum animation. 
		{
			FX_PENDULUM,	// sampleId
			FX_SPOT, 			// type
			6,						// delay (or random chance) *
			{							// roomVolList
				{56,2,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 209 Compartment door.  DOOR65
		{
			FX_DOOR65,		// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{65,3,3},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 210 Opening window. GEOOPN1
		{
			FX_WINDOW66,	// sampleId
			FX_SPOT, 			// type
			8,						// delay (or random chance) *
			{							// roomVolList
				{66,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 211 Wind rip by the open window. Triggered at the end of effect 210. 
		{
			FX_WIND66,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{66,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 212 George electrocutes himself on the pantograph. Fool.  GEOSHK64
		{
			FX_SHOCK63,		// sampleId
			FX_SPOT, 			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{63,12,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 213 The train brakes violently. GEOSTP69
		{
			FX_BRAKES,		// sampleId
			FX_SPOT, 			// type
			13,						// delay (or random chance) *
			{							// roomVolList
				{69,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 214 The train ticks over. From the end of BRAKE. 
		{
			FX_TICK69,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{69,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 215 Eklund shoot Khan.  FIGHT69
		{
			FX_EKSHOOT,		// sampleId
			FX_SPOT, 			// type
			120,					// delay (or random chance) *
			{							// roomVolList
				{69,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 216 Eklund shoots George. GEODIE69 
		{
			FX_EKSHOOT,		// sampleId
			FX_SPOT, 			// type
			21,						// delay (or random chance) *
			{							// roomVolList
				{69,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 217 Khan pulls the door open. FIGHT69 
		{
			FX_DOOR69,		// sampleId
			FX_SPOT, 			// type
			42,						// delay (or random chance) *
			{							// roomVolList
				{69,8,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 218 Wind shriek. Loops from the end of DOOR69 wav to the beginning of BRAKES. 
		{
			FX_WIND66,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{69,8,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 219 Brakes releasing pressure. Only after BRAKE has been run. 
		{
			FX_PNEUMO69,	// sampleId
			FX_RANDOM, 		// type
			720,					// delay (or random chance) *
			{							// roomVolList
				{69,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 220 External train sound. Played while George is on the top of the train.  
		{
			FX_TRAINEXT,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{63,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 221 The passing train. FIGHT69  
		{
			FX_TRNPASS,		// sampleId
			FX_SPOT, 			// type
			102,					// delay (or random chance) *
			{							// roomVolList
				{69,4,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 222 George descends into sewer. GEODESO6  
		{
			FX_LADD2,			// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{6,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 223 George ascends into alley. GEOASC06  
		{
			FX_LADD3,			// sampleId
			FX_SPOT, 			// type
			10,						// delay (or random chance) *
			{							// roomVolList
				{6,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 224 George replaces manhole cover. GEOMAN9	  
		{
			FX_COVERON2,	// sampleId
			FX_SPOT, 			// type
			19,						// delay (or random chance) *
			{							// roomVolList
				{2,12,11},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 225 Montfaucon sewer ambience.   
		{
			FX_AMBIEN37,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{37,5,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 226 George's winning smile. GEOJMP72.   
		{
			FX_PING,			// sampleId
			FX_SPOT, 			// type
			26,						// delay (or random chance) *
			{							// roomVolList
				{72,10,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 227 George starts to open the manhole. GEO36KNE   
		{
			FX_MANOP36,		// sampleId
			FX_SPOT, 			// type
			19,						// delay (or random chance) *
			{							// roomVolList
				{36,4,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 228 George opens the manhole. GEO36OPE
		{
			FX_PULLUP36,	// sampleId
			FX_SPOT, 			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{36,4,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 229 George replaces the manhole cover. GEO36CLO
		{
			FX_REPLCE36,	// sampleId
			FX_SPOT, 			// type
			20,						// delay (or random chance) *
			{							// roomVolList
				{36,4,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 230 George knocks at righthand arch. GEO37TA3
		{
			FX_KNOCK37,		// sampleId
			FX_SPOT, 			// type
			20,						// delay (or random chance) *
			{							// roomVolList
				{37,6,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 231 George knocks at middle or lefthand arch. GEO37TA1 or GEO37TA2. 
		{
			FX_KNOCK37B,	// sampleId
			FX_SPOT, 			// type
			20,						// delay (or random chance) *
			{							// roomVolList
				{37,4,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 232 George winds the chain down  HOO37LBO
		{
			FX_CHAIN37,		// sampleId
			FX_SPOT, 			// type
			14,						// delay (or random chance) *
			{							// roomVolList
				{37,6,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 233 George winds the chain up.  HOO37LBO (In reverse)
		{
			FX_CHAIN37B,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{37,6,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 234 George breaks hole in door. GEO37TA4
		{
			FX_HOLE37,		// sampleId
			FX_SPOT, 			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{37,6,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 235 Plaster door collapses. DOR37COL
		{
			FX_DOOR37,		// sampleId
			FX_SPOT, 			// type
			23,						// delay (or random chance) *
			{							// roomVolList
				{37,8,15},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 236 Barge winch. GEO37TUL (If it runs more than once, trigger the effect on frame one. Incidentally, this is a reversible so the effect must launch on frame one of the .cdr version as well. )
		{
			FX_WINCH37,		// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{37,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 237 George places chess piece. GEOSPA17
		{
			FX_CHESS,			// sampleId
			FX_SPOT, 			// type
			23,						// delay (or random chance) *
			{							// roomVolList
				{59,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 238 Piano loop for the upstairs hotel corridor.
		{
			FX_PIANO14,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{14,2,2},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 239 Door opens in church tower. PANEL72
		{
			FX_SECDOR72,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{72,8,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 240 George rummages through debris. Tied to the end of the whichever crouch is used. Use either this one or RUMMAGE2 alternatively or randomly. Same kind of schtick as the pick axe noises, I suppose. 
		{
			FX_RUMMAGE1,	// sampleId
			FX_SPOT, 			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{72,8,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 241 George rummages through debris. See above for notes. 
		{
			FX_RUMMAGE2,	// sampleId
			FX_SPOT, 			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{72,8,6},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 242 Gust of wind in the graveyard.  
		{
			FX_GUST71,		// sampleId
			FX_RANDOM, 		// type
			1080,					// delay (or random chance) *
			{							// roomVolList
				{71,3,3},		// {roomNo,leftVol,rightVol}
				{72,2,1},	
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 243 Violin ambience for Ireland.   
		{
			FX_VIOLIN19,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{19,3,3},		// {roomNo,leftVol,rightVol}
				{21,2,2},	
				{26,2,2},	
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 244 Footstep #1 for underground locations. Same schtick as for 188 and 189. 
		{
			FX_SEWSTEP1,	// sampleId
			FX_SPOT, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{6,8,8},		// {roomNo,leftVol,rightVol}
				{7,8,8},	
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 245 Footstep #2 for underground locations. Same schtick as for 188 and 189. 
		{
			FX_SEWSTEP2,	// sampleId
			FX_SPOT, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{6,16,16},	// {roomNo,leftVol,rightVol}
				{7,16,16},	
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 246 Nico's carabiner as she descends into the museum. NICPUS1 
		{
			FX_CARABINE,	// sampleId
			FX_SPOT, 			// type
			4,						// delay (or random chance) *
			{							// roomVolList
				{29,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 247 Rosso is shot (with a piece of field artillery).  ROSSHOT
		{
			FX_GUN79,			// sampleId
			FX_SPOT, 			// type
			2,						// delay (or random chance) *
			{							// roomVolList
				{79,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 248 George is hit by the thrown stilletto. GEODIE1
		{
			FX_DAGGER1,		// sampleId
			FX_SPOT, 			// type
			2,						// delay (or random chance) *
			{							// roomVolList
				{73,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 249 George is hit by the thrown stilletto after walking forward. GEODIE2
		{
			FX_DAGGER1,		// sampleId
			FX_SPOT, 			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{73,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 250 Can hits the well water. The cue is in GAR2SC57.TXT immediately after the line, "over: Lopez threw the can away. It seemed to fall an awfully long way."
		{
			FX_CANFALL,		// sampleId
			FX_SPOT, 			// type
			4,						// delay (or random chance) *
			{							// roomVolList
				{57,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 251 Mad, fizzing damp and ancient gunpowder after the application of a torch. 
		{
			FX_GUNPOWDR,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{73,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 252 Maguire whistling. MAGSLK. Plays while Maguire is idling, stops abruptly when he does something else. 
		{
			FX_WHISTLE,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{19,2,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 253  George is hit by the goat. GEOHITR and GEOHITL. 
		{
			FX_GEOGOAT,		// sampleId
			FX_SPOT, 			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{24,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 254 Manager says, "Hello". MAN2
		{
			FX_MANG1,			// sampleId
			FX_SPOT, 			// type
			7,						// delay (or random chance) *
			{							// roomVolList
				{49,6,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 255 Manager says, Don't go in there!" MAN3
		{
			FX_MANG2,			// sampleId
			FX_SPOT, 			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{49,6,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 256 Manager says, "Here are the keys." MAN4
		{
			FX_MANG3,			// sampleId
			FX_SPOT, 			// type
			6,						// delay (or random chance) *
			{							// roomVolList
				{49,6,5},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 257 George pulls the lion's tooth. GEOSPA26
		{
			FX_TOOTHPUL,	// sampleId
			FX_SPOT, 			// type
			19,						// delay (or random chance) *
			{							// roomVolList
				{61,8,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 258 George escapes the lion.  LION1
		{
			FX_LIONFALL,	// sampleId
			FX_SPOT, 			// type
			7,						// delay (or random chance) *
			{							// roomVolList
				{61,8,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 259 George gets flattened. LION2
		{
			FX_LIONFAL2,	// sampleId
			FX_SPOT, 			// type
			4,						// delay (or random chance) *
			{							// roomVolList
				{61,8,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 260 Rosso dies. ROSSFALL
		{
			FX_ROSSODIE,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{74,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 261 Eklund chokes George. FIGHT79
		{
			FX_CHOKE1,		// sampleId
			FX_SPOT, 			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{79,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 262 Eklund chokes George some more. FIGHT79
		{
			FX_CHOKE2,		// sampleId
			FX_SPOT, 			// type
			54,						// delay (or random chance) *
			{							// roomVolList
				{79,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 263 Eklund dies. FIGHT79
		{
			FX_FIGHT2,		// sampleId
			FX_SPOT, 			// type
			44,					// delay (or random chance) *
			{							// roomVolList
				{79,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 264 George hears museum break-in. GEOSUR29
		{
			FX_DOOR29,		// sampleId
			FX_SPOT, 			// type
			10,						// delay (or random chance) *
			{							// roomVolList
				{94,14,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 265 George hits the floor having been shot. GEODED. 
		{
			FX_GDROP29,		// sampleId
			FX_SPOT, 			// type
			27,						// delay (or random chance) *
			{							// roomVolList
				{29,10,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 266 George hits the floor having been stunned. GEOFISH
		{
			FX_GDROP29,		// sampleId
			FX_SPOT, 			// type
			27,						// delay (or random chance) *
			{							// roomVolList
				{29,10,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 267 Fitz being knocked down as heard from inside the pub. Triggered from the script, I think. This is just a stopgap until Hackenbacker do the full version for the Smacker, then I'll sample the requisite bit and put it in here. 
		{
			FX_FITZHIT,		// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{20,16,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 268 Gendarme shoots lock off. GENSHOT 
		{
			FX_GUN34,			// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{34,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 269 ECG alarm, Marquet in trouble. Start looping imeediately before George says, "Thanks, Bunny".
		// Incidentally, James, please switch off Mr Shiney permanently when George first gets into Marquet's room. He gets in the way when they're figuring out that Eklund's an imposter.
		{
			FX_PULSE2,		// sampleId
			FX_LOOP, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{30,16,16},	// {roomNo,leftVol,rightVol}
				{34,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 270 ECG alarm, Marquet dead. Switch off the previous effect and replace with this immediately before the gendarme says, "Stand back, messieurs."
		{
			FX_PULSE3,		// sampleId
			FX_LOOP, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{30,16,16},	// {roomNo,leftVol,rightVol}
				{34,13,13},
				{35,13,13},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 271 Door closing. GEOENT15
		{
			FX_DORCLOSE,	// sampleId
			FX_SPOT, 			// type
			4,						// delay (or random chance) *
			{							// roomVolList
				{15,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 272 Cupboard opening. GEOCOT
		{
			FX_CUPBOPEN,	// sampleId
			FX_SPOT, 			// type
			8,						// delay (or random chance) *
			{							// roomVolList
				{33,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 273 Cupboard closing. GEOCOT
		{
			FX_CUPBCLOS,	// sampleId
			FX_SPOT, 			// type
			33,						// delay (or random chance) *
			{							// roomVolList
				{33,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 274 Closing door when George leaves hotel room. GEOLVS15 and GEODOR17 (they're identical). 
		{
			FX_DORCLOSE,	// sampleId
			FX_SPOT, 			// type
			44,						// delay (or random chance) *
			{							// roomVolList
				{15,12,12},	// {roomNo,leftVol,rightVol}
				{17,12,12},	
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 275 Closing door when George leaves the pub. DOROPN20 (Reversed) 
		{
			FX_DORCLOSE20,// sampleId
			FX_SPOT, 			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{20,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 276 Nico call for a cab.  NICPHN10
		{
			FX_PHONICO1,	// sampleId
			FX_SPOT, 			// type
			15,						// delay (or random chance) *
			{							// roomVolList
				{10,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 277 Nico puts down the phone. NICDWN10
		{
			FX_FONEDN,		// sampleId
			FX_SPOT, 			// type
			6,						// delay (or random chance) *
			{							// roomVolList
				{10,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 278 Painter puts down the phone. PAI41HAN
		{
			FX_FONEDN41,	// sampleId
			FX_SPOT, 			// type
			5,						// delay (or random chance) *
			{							// roomVolList
				{41,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 279 Mechanical hum of heating system in the dig lobby.
		{
			FX_AIRCON41,	// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{41,6,6},		// {roomNo,leftVol,rightVol}
				{43,8,8},
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		//------------------------
		// 280 The Sword is Reforged (Grandmaster gets zapped) GMPOWER
		{
			FX_REFORGE1,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{78,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 281 The Sword is Reforged (G&N gawp at the spectacle) There's no anim I know of to tie it to unless the flickering blue light is one. 
		{
			FX_REFORGE2,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{75,12,12},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 282 The Sword is Reforged (We watch over G&N's heads as the Grandmaster gets zapped) GMWRIT74
		{
			FX_REFORGE2,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{74,14,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 283 The Sword is Reforged (Grandmaster finishes being zapped) GMWRITH
		{
			FX_REFORGE4,	// sampleId
			FX_SPOT, 			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{78,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 284 Baphomet Cavern Ambience
		{
			FX_BAPHAMB,		// sampleId
			FX_LOOP, 			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{74,6,8},		// {roomNo,leftVol,rightVol}
				{75,7,8},		// {roomNo,leftVol,rightVol}
				{76,8,8},		// {roomNo,leftVol,rightVol}
				{77,8,8},		// {roomNo,leftVol,rightVol}
				{78,8,8},		// {roomNo,leftVol,rightVol}
				{79,7,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 285 Duane's Happy-Snappy Camera. XDNEPHO3 and XDNEPHO5. 
		{
			FX_CAMERA45,	// sampleId
			FX_SPOT, 			// type
			30,						// delay (or random chance) *
			{							// roomVolList
				{45,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 286 Grand Master strikes the floor with his cane. GMENTER 
		{
			FX_STAFF,			// sampleId
			FX_SPOT, 			// type
			28,						// delay (or random chance) *
			{							// roomVolList
				{73,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 287 George descends ladder in 7: GEOASC07 (Reversed) This used to be handled by effect #46 but it didn't fit at all. 
		{
			FX_SEWLADD7,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{7,8,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 288 Sam kicks the recalcitrant Mr. Shiny. DOMKIK
		{
			FX_KIKSHINY,	// sampleId
			FX_SPOT,			// type
			16,						// delay (or random chance) *
			{							// roomVolList
				{33,9,9},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 289 Gust of wind outside bombed cafe. LVSFLY
		{
			FX_LVSFLY,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{1,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 290 Ron's disgusting No.1 Sneeze. Either this or the next effect (randomly chosen) is used for the following animations, RONSNZ & RONSNZ2
		{
			FX_SNEEZE1,		// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{20,10,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 291 Ron's disgusting No.2 Sneeze. Either this or the previous effect (randomly chosen) is used for the following animations, RONSNZ & RONSNZ2
		{
			FX_SNEEZE2,		// sampleId
			FX_SPOT,			// type
			11,						// delay (or random chance) *
			{							// roomVolList
				{20,10,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 292 Dripping tap in the pub cellar. TAPDRP
		{
			FX_DRIPIRE,		// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{21,4,4},		// {roomNo,leftVol,rightVol}
				{26,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 293 Dripping tap in the pub cellar. TAPDRP
		{
			FX_DRIPIRE2,	// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{21,4,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 294 Dripping tap in the excavation toilet. (see WATER43 - but it's looped anyway, not triggered with anim)
		{
			FX_TAPDRIP,		// sampleId
			FX_SPOT,			// type
			6,						// delay (or random chance) *
			{							// roomVolList
				{43,8,8},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 295 George closes the mausoleum window. GEOSPA23
		{
			FX_WINDOW59,	// sampleId
			FX_SPOT,			// type
			24,						// delay (or random chance) *
			{							// roomVolList
				{59,10,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 296 George opens the mausoleum window, the feebleminded loon. GEOSPA23 reversed.
		{
			FX_WINDOW59,	// sampleId
			FX_SPOT,			// type
			14,						// delay (or random chance) *
			{							// roomVolList
				{59,10,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 297	When George & Nico hear chanting from sc73
		{
			FX_CHANT,			// sampleId
			FX_SPOT,			// type
			10,						// delay (or random chance) *
			{							// roomVolList
				{73,2,4},		// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 298	EKFIGHT
		{
			FX_FIGHT1,		// sampleId
			FX_SPOT,			// type
			31,						// delay (or random chance) *
			{							// roomVolList
				{74,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 299 Small van passes, left to right. CARA9 and CARC9
		{
			FX_LITEVEHR,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{9,16,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 300 Small van passes, right to left to right. CARB9
		{
			FX_LITEVEHL,	// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{9,16,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 301 Truck passes, left to right. TRUCKA9 and TRUCKB9
		{
			FX_HVYVEHR,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{9,14,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		},
		//------------------------
		// 302 Truck passes, right to left. TRUCKC9
		{
			FX_HVYVEHL,		// sampleId
			FX_SPOT,			// type
			1,						// delay (or random chance) *
			{							// roomVolList
				{9,14,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 303 With anim FIGHT69
		{
			FX_FIGHT69,		// sampleId
			FX_SPOT,			// type
			78,						// delay (or random chance) *
			{							// roomVolList
				{69,12,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 304 With anim GEODIE1 in sc73
		{
			FX_GDROP73,		// sampleId
			FX_SPOT,			// type
			14,						// delay (or random chance) *
			{							// roomVolList
				{73,12,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 305 With anim GEODIE2 in sc73
		{
			FX_GDROP73,		// sampleId
			FX_SPOT,			// type
			21,						// delay (or random chance) *
			{							// roomVolList
				{73,12,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 306 With anim GEODES25
		{
			FX_LADDWN25,	// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{25,12,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 307 With anim GEOASC25
		{
			FX_LADDUP25,	// sampleId
			FX_SPOT,			// type
			8,						// delay (or random chance) *
			{							// roomVolList
				{25,12,8},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 308 With anim GKSWORD in sc76
		{
			FX_GKSWORD,		// sampleId
			FX_SPOT,			// type
			9,						// delay (or random chance) *
			{							// roomVolList
				{76,10,10},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 309 With anim GEO36KNE in sc36
		{
			FX_KEYIN,			// sampleId
			FX_SPOT,			// type
			18,						// delay (or random chance) *
			{							// roomVolList
				{36,14,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 310 With anim GEO36ENT in sc36
		{
			FX_COVDWN,		// sampleId
			FX_SPOT,			// type
			85,						// delay (or random chance) *
			{							// roomVolList
				{36,14,14},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
		// 311 With anim SECDOR59 in sc59
		{
			FX_SECDOR59,	// sampleId
			FX_SPOT,			// type
			0,						// delay (or random chance) *
			{							// roomVolList
				{59,16,16},	// {roomNo,leftVol,rightVol}
				{0,0,0},		// NULL-TERMINATOR
			},
		}, 
		//------------------------
	};

	writeRecord(_fxList, sizeof(_fxList), GBVARS_FXLIST_INDEX , GBVARS_SWORD1);
}

void Sword1_addStaticres() {
	addStaticres_fxList();
}
