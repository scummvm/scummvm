#pragma once

#include "common_header.h"
#include "moveeffect.h"
#include "fm_ai_data.h"
#include "triggerset.h"
#include "fightinput.h"
#include "MusicManager.h"
#include "shadow.h"

namespace pyrodactyl
{
	namespace anim
	{
		//A single frame of a fighting move
		struct FightAnimFrame : public AnimFrame
		{
			//The hit box of the player WITH RESPECT TO the sprite bounding box
			//_v is the vulnerable hit box, _d is the damage hit box
			Rect box_d;

			//The displacement in the position caused by the frame
			Vector2i delta;

			//The sprite state for the duration of the frame
			unsigned int state;

			//Can we cancel/branch to another move from this frame?
			bool branch;

			FightAnimFrame() { state = 0; branch = false; }

			void Load(rapidxml::xml_node<char> * node, const Rect& VBOX,
				const Uint32 &REP = 0, const int &AX = 0, const int &AY = 0);
		};

		//All data related to a single fighting move in a single direction
		class FightAnimFrames
		{
			//The global vulnerable hit box for all the frames
			//If the W or H of this is 0, then use individual frame values
			Rect box_v;

		public:
			//The individual frames for each direction
			std::vector<FightAnimFrame> frame;

			//The current clip
			unsigned int current_clip;

			//Should we flip the images in the frame rectangle?
			TextureFlipType flip;

			//The amount of time in milliseconds each animation frame needs to be on screen
			//If this is zero then use the value in each individual frame
			Uint32 repeat;

			//The global anchor value for all the frames
			//If this is 0, then use individual frame values
			Vector2i anchor;

			//Does this set of animation frames need a specific shadow offset?
			ShadowOffset shadow;

			FightAnimFrames(){ Reset(); flip = FLIP_NONE; repeat = 0; }
			void Reset() { current_clip = 0; }

			void Load(rapidxml::xml_node<char> * node);
		};
	}
}