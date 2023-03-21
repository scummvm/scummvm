#pragma once

#include "common_header.h"
#include "vectors.h"
#include "Rectangle.h"
#include "shadow.h"

namespace pyrodactyl
{
	namespace anim
	{
		struct AnimFrame
		{
			//Portion of sprite to show
			Rect clip;

			//The duration for which the frame must be repeated on screen
			Uint32 repeat;

			//The anchor point of the frame
			Vector2i anchor;

			//The vulnerable hit box for this frame
			Rect box_v;

			AnimFrame() { repeat = 0; }

			void Load(rapidxml::xml_node<char> *node, const Rect& VBOX,
				const Uint32 &REP = 0, const int &AX = 0, const int &AY = 0);
		};

		class AnimationFrames
		{
			//The global vulnerable hit box for all the frames
			//If the W or H of this is 0, then use individual frame values
			Rect box_v;

		public:
			//The frames for the animation
			std::vector<AnimFrame> frame;

			//The current clip
			unsigned int current_clip;

			//Should we flip the images in the frame rectangle?
			TextureFlipType flip;

			//The global repeat value for all the frames
			//If this is 0, then use individual frame values
			Uint32 repeat;

			//The global anchor value for all the frames
			//If this is 0, then use individual frame values
			Vector2i anchor;

			//true if animation starts at a random frame
			//used for idle animations so that every sprite doesn't animate in sync
			bool random;

			//Does this set of animation frames need a specific shadow offset?
			ShadowOffset shadow;

			AnimationFrames(){ Reset(); flip = FLIP_NONE; repeat = 0; random = false; }
			void Reset() { current_clip = 0; }

			void Load(rapidxml::xml_node<char> *node);

			bool UpdateClip();
			const AnimFrame& CurrentFrame();
		};
	}
}