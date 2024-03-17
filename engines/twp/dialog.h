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

#ifndef TWP_DIALOG_H
#define TWP_DIALOG_H

#include "common/array.h"
#include "common/str.h"
#include "twp/yack.h"
#include "twp/scenegraph.h"
#include "twp/font.h"
#include "twp/motor.h"

#define MAXDIALOGSLOTS 9
#define MAXCHOICES 6
#define SLIDINGSPEED 25.f
#define SLOTMARGIN 8.f

namespace Twp {

class Dialog;
class DialogSlot : public Node {
public:
	DialogSlot();
	virtual ~DialogSlot() override {}

public:
	bool _isValid = false;
	Text _text;
	Common::SharedPtr<YStatement> _stmt;
	Dialog *_dlg = nullptr;
};

struct DialogContext {
	Common::String actor;
	Common::String dialogName;
	bool parrot = true;
	int limit = MAXDIALOGSLOTS;
};

enum DialogState {
	None,
	Active,
	WaitingForChoice
};

enum DialogConditionMode {
	Once,
	ShowOnce,
	OnceEver,
	ShowOnceEver,
	TempOnce
};

enum DialogSelMode {
	Choose,
	Show
};

struct DialogConditionState {
	DialogConditionMode mode;
	Common::String actorKey, dialog;
	int line;

	DialogConditionState();
	DialogConditionState(DialogConditionMode mode, const Common::String &actorKey, const Common::String &dialog, int line);
};

class DialogTarget {
public:
	virtual ~DialogTarget() {}

	virtual Color actorColor(const Common::String &actor) = 0;
	virtual Color actorColorHover(const Common::String &actor) = 0;
	virtual Common::SharedPtr<Motor> say(const Common::String &actor, const Common::String &text) = 0;
	virtual Common::SharedPtr<Motor> waitWhile(const Common::String &cond) = 0;
	virtual void shutup() = 0;
	virtual Common::SharedPtr<Motor> pause(float time) = 0;
	virtual bool execCond(const Common::String &cond) = 0;
};

class CondStateVisitor : public YackVisitor {
public:
	CondStateVisitor(Dialog *dlg, DialogSelMode mode);
	DialogConditionState createState(int line, DialogConditionMode mode);

private:
	void visit(const YOnce &node) override;
	void visit(const YShowOnce &node) override;
	void visit(const YOnceEver &node) override;
	void visit(const YTempOnce &node) override;

private:
	DialogSelMode _mode;
	Dialog *_dlg = nullptr;
};

class IsGoto : public YackVisitor {
public:
	virtual ~IsGoto() override {}
	void visit(const YGoto &node) override { _isGoto = true; }

public:
	bool _isGoto = false;
};

class IsChoice : public YackVisitor {
public:
	virtual ~IsChoice() override {}
	void visit(const YChoice &node) override { _isChoice = true; }

public:
	bool _isChoice = false;
};

class ExpVisitor : public YackVisitor {
public:
	explicit ExpVisitor(Dialog *dialog);
	virtual ~ExpVisitor() override;

private:
	void visit(const YCodeExp &node) override;
	void visit(const YGoto &node) override;
	void visit(const YSay &node) override;
	void visit(const YPause &node) override;
	void visit(const YParrot &node) override;
	void visit(const YDialog &node) override;
	void visit(const YOverride &node) override;
	void visit(const YAllowObjects &node) override;
	void visit(const YLimit &node) override;
	void visit(const YWaitWhile &node) override;
	void visit(const YWaitFor &node) override;
	void visit(const YShutup &node) override;

private:
	Dialog *_dialog = nullptr;
};

class CondVisitor : public YackVisitor {
public:
	CondVisitor(Dialog *dialog);
	virtual ~CondVisitor() override;

private:
	void visit(const YCodeCond &node) override;
	void visit(const YOnce &node) override;
	void visit(const YShowOnce &node) override;
	void visit(const YOnceEver &node) override;
	void visit(const YTempOnce &node) override;

public:
	bool _accepted = true;

private:
	Dialog *_dialog = nullptr;
};

class Dialog : public Node {
public:
	Dialog();
	virtual ~Dialog() override;

	void choose(int choice);
	void update(float dt);
	DialogState getState() const { return _state; }

	void setMousePos(Math::Vector2d pos) { _mousePos = pos; }

	void start(const Common::String &actor, const Common::String &name, const Common::String &node);
	void selectLabel(int line, const Common::String &name);
	bool isOnce(int line) const;
	bool isShowOnce(int line) const;
	bool isOnceEver(int line) const;
	bool isTempOnce(int line) const;
	bool isCond(const Common::String &cond) const;

private:
	void choose(DialogSlot *slot);
	Common::SharedPtr<YLabel> label(int line, const Common::String &name) const;
	void gotoNextLabel();
	bool choicesReady() const { return numSlots() > 0; }
	void updateChoiceStates();
	void run(Common::SharedPtr<YStatement> stmt);
	bool acceptConditions(Common::SharedPtr<YStatement> stmt);
	void running(float dt);

	void addSlot(Common::SharedPtr<YStatement> stmt);
	int numSlots() const;
	void clearSlots();

	virtual void drawCore(Math::Matrix4 trsf) override final;

public:
	Common::Array<DialogConditionState> _states;
	DialogContext _context;
	unique_ptr<DialogTarget> _tgt;
	Common::SharedPtr<Motor> _action;

private:
	DialogState _state = DialogState::None;
	size_t _currentStatement = 0;
	unique_ptr<YCompilationUnit> _cu;
	Common::SharedPtr<YLabel> _lbl;
	DialogSlot _slots[MAXDIALOGSLOTS];
	Math::Vector2d _mousePos;
};

} // namespace Twp

#endif
