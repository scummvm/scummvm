#pragma once

#include "common_header.h"
#include "fightmove.h"
#include "MusicManager.h"

namespace pyrodactyl
{
	namespace anim
	{
		enum FrameUpdateResult { FUR_FAIL, FUR_WAIT, FUR_SUCCESS };

		//This state value indicates that the move should execute regardless of actual sprite state
		const unsigned int SPRITE_STATE_OVERRIDE = std::numeric_limits<unsigned int>::max();

		class FightMoves
		{
			//The fighting moves of a sprite
			std::vector<FightMove> move;

			//The currently selected move
			int cur;

			//For AI - the move about to be executed
			int next;

			//The timer used for playing animations
			Timer timer;

			//We need to instantly show the new frame for a move that has started
			bool start;

			//The current frame and total frames
			unsigned int frame_cur, frame_total;

		public:

			FightMoves();
			~FightMoves(){}
			void Reset() { cur = -1; }

			void Load(rapidxml::xml_node<char> *node);

			bool CurMove(FightMove &fm);
			bool NextMove(FightMove &fm);

			FrameUpdateResult UpdateFrame(const Direction &d);
			bool CurFrame(FightAnimFrame &faf, const Direction &d);

			unsigned int FindMove(const pyrodactyl::input::FightAnimationType &type, const int &state);

			void ListAttackMoves(std::vector<unsigned int> &list);

			bool ForceUpdate(const unsigned int &index, pyrodactyl::input::FightInput &input, const Direction &d);

			bool LastFrame() { return frame_cur >= frame_total; }
			void FrameIndex(unsigned int val) { frame_cur = val; }

			void CurCombo(pyrodactyl::input::FightInput &input) { input = move[cur].input; }

			bool ValidMove() { return cur >= 0 && cur < move.size(); }
			bool Empty() { return move.empty(); }

			bool Flip(TextureFlipType &flip, Direction d);
			const ShadowOffset& Shadow(Direction d){ return move.at(cur).frames[d].shadow; }

			void Next(int val) { next = val; }
			int Next() { return next; }

			void Evaluate(pyrodactyl::event::Info &info);
		};
	}
}