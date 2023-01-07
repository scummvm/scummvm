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

#ifndef TETRAEDGE_GAME_QUESTION2_H
#define TETRAEDGE_GAME_QUESTION2_H

#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_layout.h"


namespace Tetraedge {

class Question2 : public TeLayout {
public:
	Question2();
	~Question2();

	class Answer {
	public:
		TeLayout *layout();
		void load(const Common::String &name, const Common::String &unk, const Common::String &path);
		void unload();
		bool onButtonValidated();

		TeLuaGUI _gui;
		Common::String _str;
		TeSignal1Param<Question2::Answer &> _onButtonValidatedSignal;
	};

	void enter();
	void leave();
	void load();
	bool isEntered() const { return _entered; }
	bool onAnswerValidated(Answer &answer);
	void pushAnswer(const Common::String &name, const Common::String &unk, const Common::String &path);
	void unload();
	TeLuaGUI &gui() { return _gui; }
	TeSignal1Param<const Common::String &> &onAnswerSignal() { return _onAnswerSignal; }

private:
	bool _entered;
	Common::Array<Answer *> _answers;
	TeLuaGUI _gui;
	TeSignal1Param<const Common::String &> _onAnswerSignal;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_QUESTION2_H
