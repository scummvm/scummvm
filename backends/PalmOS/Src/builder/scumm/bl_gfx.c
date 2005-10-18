#include <PalmOS.h>
#include "b_globals.h"
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
typedef struct  {
	Int8 numOfIterations;
	Int8 deltaTable[16];	// four times l / t / r / b
	Int8 stripTable[16];	// ditto
} TransitionEffect;
///////////////////////////////////////////////////////////////////
static void addGfx_transitionEffects() {
		TransitionEffect transitionEffects[5] = {
		// Iris effect (looks like an opening/closing camera iris)
		{
			13,		// Number of iterations
			{
				1,  1, -1,  1,
			   -1,  1, -1, -1,
				1, -1, -1, -1,
				1,  1,  1, -1
			},
			{
				0,  0, 39,  0,
			   39,  0, 39, 24,
				0, 24, 39, 24,
				0,  0,  0, 24
			}
		},
		
		// Box wipe (a box expands from the upper-left corner to the lower-right corner)
		{
			25,		// Number of iterations
			{
				0,  1,  2,  1,
				2,  0,  2,  1,
				2,  0,  2,  1,
				0,  0,  0,  0
			},
			{
				0,  0,  0,  0,
				0,  0,  0,  0,
				1,  0,  1,  0,
			  255,  0,  0,  0
			}
		},
		
		// Box wipe (a box expands from the lower-right corner to the upper-left corner)
		{
			25,		// Number of iterations
			{
			   -2, -1,  0, -1,
			   -2, -1, -2,  0,
			   -2, -1, -2,  0,
				0,  0,  0,  0
			},
			{
			   39, 24, 39, 24,
			   39, 24, 39, 24,
			   38, 24, 38, 24,
			  255,  0,  0,  0
			}
		},
		
		// Inverse box wipe
		{
			25,		// Number of iterations
			{
				0, -1, -2, -1,
			   -2,  0, -2, -1,
			   -2,  0, -2, -1,
			    0,  0,  0,  0
			},
			{
				0, 24, 39, 24,
			   39,  0, 39, 24,
			   38,  0, 38, 24,
			  255,  0,  0,  0
			}
		},

		// Inverse iris effect, specially tailored for V1/V2 games
		{
			9,		// Number of iterations
			{
				-1, -1,  1, -1,
				-1,  1,  1,  1,
				-1, -1, -1,  1,
				 1, -1,  1,  1
			},
			{
				 7, 7, 32, 7,
				 7, 8, 32, 8,
				 7, 8,  7, 8,
				32, 7, 32, 8
			}
		}
	};

	writeRecord(transitionEffects, sizeof(transitionEffects), GBVARS_TRANSITIONEFFECTS_INDEX , GBVARS_SCUMM);
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

void addGfx() {
	addGfx_transitionEffects();

}
