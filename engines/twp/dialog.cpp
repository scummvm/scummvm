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

#include "twp/twp.h"
#include "twp/util.h"
#include "twp/squtil.h"
#include "twp/dialog.h"
#include "common/debug.h"

namespace Twp {

class SerialMotors : public Motor {
public:
	SerialMotors(const std::initializer_list<Motor *> &motors) : _motors(motors) {}
	SerialMotors(const Common::Array<Motor *> &motors) : _motors(motors) {}

	virtual void update(float elapsed) override {
		if (_motors.size() > 0) {
			_motors[0]->update(elapsed);
			if (!_motors[0]->isEnabled()) {
				debug("SerialMotors next");
				_motors.remove_at(0);
			}
		} else {
			debug("SerialMotors is over");
			disable();
		}
	}

private:
	Common::Array<Motor *> _motors;
};

class SelectLabelMotor : public Motor {
public:
	SelectLabelMotor(Dialog *dlg, int line, const Common::String &name)
		: _dlg(dlg), _line(line), _name(name) {
	}

	virtual void update(float elapsed) override {
		_dlg->selectLabel(_line, _name);
		disable();
	}

private:
	Dialog *_dlg;
	int _line;
	Common::String _name;
};

CondStateVisitor::CondStateVisitor(Dialog *dlg, DialogSelMode mode) : _dlg(dlg), _mode(mode) {
}

DialogConditionState CondStateVisitor::createState(int line, DialogConditionMode mode) {
	return DialogConditionState(mode, _dlg->_context.actor, _dlg->_context.dialogName, line);
}

DialogConditionState::DialogConditionState() {}

DialogConditionState::DialogConditionState(DialogConditionMode m, const Common::String& k, const Common::String& dlg, int ln)
: mode(m), actorKey(k), dialog(dlg), line(ln) {
}

void CondStateVisitor::visit(const YOnce &node) {
	if (_mode == DialogSelMode::Choose)
		_dlg->_states.push_back(createState(node._line, DialogConditionMode::Once));
}

void CondStateVisitor::visit(const YShowOnce &node) {
	if (_mode == DialogSelMode::Show)
		_dlg->_states.push_back(createState(node._line, DialogConditionMode::ShowOnce));
}

void CondStateVisitor::visit(const YOnceEver &node) {
	if (_mode == DialogSelMode::Choose)
		_dlg->_states.push_back(createState(node._line, DialogConditionMode::OnceEver));
}

void CondStateVisitor::visit(const YTempOnce &node) {
	if (_mode == DialogSelMode::Show)
		_dlg->_states.push_back(createState(node._line, DialogConditionMode::TempOnce));
}

ExpVisitor::ExpVisitor(Dialog *dialog) : _dialog(dialog) {}
ExpVisitor::~ExpVisitor() {}

void ExpVisitor::visit(const YCodeExp &node) {
	debug("execute code %s", node._code.c_str());
	sqexec(g_engine->getVm(), node._code.c_str(), "dialog");
}

void ExpVisitor::visit(const YGoto &node) {
	debug("execute goto %s", node._name.c_str());
	_dialog->selectLabel(node._line, node._name);
}

void ExpVisitor::visit(const YShutup &node) {
	debug("shutup");
	_dialog->_tgt->shutup();
}

void ExpVisitor::visit(const YPause &node) {
	debug("pause %d", node._time);
	_dialog->_action = _dialog->_tgt->pause(node._time);
}

void ExpVisitor::visit(const YWaitFor &node) {
	debug("TODO: waitFor {%s}", node._actor.c_str());
}

void ExpVisitor::visit(const YParrot &node) {
	_dialog->_context.parrot = node._active;
}

void ExpVisitor::visit(const YDialog &node) {
	_dialog->_context.actor = node._actor;
}

void ExpVisitor::visit(const YOverride &node) {
	warning("TODO: override %s", node._node.c_str());
}

void ExpVisitor::visit(const YAllowObjects &node) {
	warning("TODO: allowObjects");
}

void ExpVisitor::visit(const YWaitWhile &node) {
	debug("wait while");
	_dialog->_action = _dialog->_tgt->waitWhile(node._cond);
}

void ExpVisitor::visit(const YLimit &node) {
	debug("limit");
	_dialog->_context.limit = node._max;
}

void ExpVisitor::visit(const YSay &node) {
	_dialog->_action = _dialog->_tgt->say(node._actor, node._text);
}

CondVisitor::CondVisitor(Dialog *dialog) : _dialog(dialog) {}
CondVisitor::~CondVisitor() {}

void CondVisitor::visit(const YCodeCond &node) {
	_accepted = _dialog->isCond(node._code);
}

void CondVisitor::visit(const YOnce &node) {
	_accepted = _dialog->isOnce(node._line);
}

void CondVisitor::visit(const YShowOnce &node) {
	_accepted = _dialog->isShowOnce(node._line);
}

void CondVisitor::visit(const YOnceEver &node) {
	_accepted = _dialog->isOnceEver(node._line);
}

void CondVisitor::visit(const YTempOnce &node) {
	_accepted = _dialog->isTempOnce(node._line);
}

DialogSlot::DialogSlot() : Node("DialogSlot") {}

Dialog::Dialog() : Node("Dialog") {}
Dialog::~Dialog() {}

static YChoice *getChoice(DialogSlot *slot) {
	return (YChoice *)(slot->_stmt->_exp.get());
}

void Dialog::choose(int choice) {
	if (_state == WaitingForChoice) {
		choose(&_slots[choice]);
	}
}

void Dialog::choose(DialogSlot *slot) {
	if (slot && slot->_isValid) {
		sqcall("onChoiceClick");
		for (size_t i = 0; i < slot->_stmt->_conds.size(); i++) {
			YCond *cond = slot->_stmt->_conds[i];
			CondStateVisitor v(slot->_dlg, DialogSelMode::Choose);
			cond->accept(v);
		}
		YChoice *choice = getChoice(slot);
		if (slot->_dlg->_context.parrot) {
			slot->_dlg->_state = DialogState::Active;
			slot->_dlg->_action = new SerialMotors(
				{slot->_dlg->_tgt->say(slot->_dlg->_context.actor, choice->_text),
				 new SelectLabelMotor(slot->_dlg, choice->_goto->_line, choice->_goto->_name)});
			slot->_dlg->clearSlots();
		} else {
			slot->_dlg->selectLabel(choice->_goto->_line, choice->_goto->_name);
		}
	}
}

void Dialog::start(const Common::String &actor, const Common::String &name, const Common::String &node) {
	_context = DialogContext{.actor = actor, .dialogName = name, .parrot = true, .limit = MAXCHOICES};
	// keepIf(self.states, proc(x: DialogConditionState): bool = x.mode != TempOnce);
	Common::String path = name + ".byack";
	debug("start dialog %s", path.c_str());
	GGPackEntryReader reader;
	reader.open(g_engine->_pack, path);
	YackParser parser;
	_cu.reset(parser.parse(&reader));
	selectLabel(0, node);
	update(0);
}

void Dialog::update(float dt) {
	switch (_state) {
	case DialogState::None:
		break;
	case DialogState::Active:
		running(dt);
		break;
	case DialogState::WaitingForChoice: {
		Color color = _tgt->actorColor(_context.actor);
		Color colorHover = _tgt->actorColorHover(_context.actor);
		for (size_t i = 0; i < MAXDIALOGSLOTS; i++) {
			DialogSlot *slot = &_slots[i];
			if (slot->_isValid) {
				Rectf rect = Rectf::fromPosAndSize(slot->getPos() - Math::Vector2d(0.f, slot->_text.getBounds().getY()), slot->_text.getBounds());
				bool over = slot && rect.contains(_mousePos);
				if (rect.r.w > (SCREEN_WIDTH - SLOTMARGIN)) {
					if (over) {
						if ((rect.r.w + slot->getPos().getX()) > (SCREEN_WIDTH - SLOTMARGIN)) {
							slot->setPos(Math::Vector2d(slot->getPos().getX() - SLIDINGSPEED * dt, slot->getPos().getY()));
							if ((rect.r.w + slot->getPos().getX()) < (SCREEN_WIDTH - SLOTMARGIN)) {
								slot->setPos(Math::Vector2d((SCREEN_WIDTH - SLOTMARGIN) - rect.r.w, slot->getPos().getY()));
							}
						}
					} else if (slot->getPos().getX() < SLOTMARGIN) {
						slot->setPos(Math::Vector2d(slot->getPos().getX() + SLIDINGSPEED * dt, slot->getPos().getY()));
						if (slot->getPos().getX() > SLOTMARGIN) {
							slot->setPos(Math::Vector2d(SLOTMARGIN, slot->getPos().getY()));
						}
					}
				}
				slot->_text.setColor(over ? colorHover : color);
				if (over && g_engine->_cursor.isLeftDown())
					choose(i);
			}
		}
	} break;
	}
}

bool Dialog::isOnce(int line) const {
	for (size_t i = 0; i < _states.size(); i++) {
		const DialogConditionState &state = _states[i];
		if (state.mode == Once && state.actorKey == _context.actor && state.dialog == _context.dialogName && state.line == line) {
			debug("isOnce %d: false", line);
			return false;
		}
	}
	debug("isOnce %d: true", line);
	return true;
}

bool Dialog::isShowOnce(int line) const {
	for (size_t i = 0; i < _states.size(); i++) {
		const DialogConditionState &state = _states[i];
		if (state.mode == ShowOnce && state.actorKey == _context.actor && state.dialog == _context.dialogName && state.line == line) {
			debug("isShowOnce %d: false", line);
			return false;
		}
	}
	debug("isShowOnce %d: true", line);
	return true;
}

bool Dialog::isOnceEver(int line) const {
	for (size_t i = 0; i < _states.size(); i++) {
		const DialogConditionState &state = _states[i];
		if (state.mode == OnceEver && state.dialog == _context.dialogName && state.line == line) {
			debug("isOnceEver %d: false", line);
			return false;
		}
	}
	debug("isOnceEver %d: true", line);
	return true;
}

bool Dialog::isTempOnce(int line) const {
	for (size_t i = 0; i < _states.size(); i++) {
		const DialogConditionState &state = _states[i];
		if (state.mode == TempOnce && state.actorKey == _context.actor && state.dialog == _context.dialogName && state.line == line) {
			debug("isTempOnce %d: false", line);
			return false;
		}
	}
	debug("isTempOnce %d: true", line);
	return true;
}

bool Dialog::isCond(const Common::String &cond) const {
	bool result = _tgt->execCond(cond);
	debug("isCond '%s': %s", cond.c_str(), result ? "TRUE" : "FALSE");
	return result;
}

YLabel *Dialog::label(int line, const Common::String &name) const {
	for (size_t i = 0; i < _cu->_labels.size(); i++) {
		YLabel *label = _cu->_labels[i];
		if ((label->_name == name) && (label->_line >= line)) {
			return label;
		}
	}
	line = 0;
	for (size_t i = 0; i < _cu->_labels.size(); i++) {
		YLabel *label = _cu->_labels[i];
		if ((label->_name == name) && (label->_line >= line)) {
			return label;
		}
	}
	return nullptr;
}

void Dialog::selectLabel(int line, const Common::String &name) {
	debug("select label %s", name.c_str());
	_lbl = label(line, name);
	_currentStatement = 0;
	clearSlots();
	_state = _lbl ? Active : None;
}

void Dialog::gotoNextLabel() {
	if (_lbl) {
        size_t i = Twp::find(_cu->_labels, _lbl);
		if ((i != (size_t)-1) && (i != _cu->_labels.size() - 1)) {
			YLabel *label = _cu->_labels[i + 1];
			selectLabel(label->_line, label->_name);
		} else {
			_state = None;
		}
	}
}

void Dialog::updateChoiceStates() {
	_state = WaitingForChoice;
	for (size_t i = 0; i < MAXDIALOGSLOTS; i++) {
		DialogSlot *slot = &_slots[i];
		if (slot->_isValid) {
			for (size_t j = 0; j < slot->_stmt->_conds.size(); j++) {
				YCond *cond = slot->_stmt->_conds[j];
				CondStateVisitor v(this, DialogSelMode::Show);
				cond->accept(v);
			}
		}
	}
}

void Dialog::run(YStatement *stmt) {
	if (acceptConditions(stmt)) {
		ExpVisitor visitor(this);
		stmt->_exp->accept(visitor);
		IsGoto isGoto;
		stmt->_exp->accept(isGoto);
		if (isGoto._isGoto)
			return;
	}
	_currentStatement++;
}

bool Dialog::acceptConditions(YStatement *stmt) {
	CondVisitor vis(this);
	for (size_t i = 0; i < stmt->_conds.size(); i++) {
		YCond *cond = stmt->_conds[i];
		cond->accept(vis);
		if (!vis._accepted) {
			return false;
		}
	}
	return true;
}
void Dialog::running(float dt) {
	if (_action && _action->isEnabled())
		_action->update(dt);
	else if (!_lbl)
		_state = None;
	else if (_currentStatement == _lbl->_stmts.size())
		gotoNextLabel();
	else {
		_state = Active;
		while (_lbl && (_currentStatement < _lbl->_stmts.size()) && (_state == Active)) {
			YStatement *statmt = _lbl->_stmts[_currentStatement];
			IsChoice isChoice;
			statmt->_exp->accept(isChoice);
			if (!acceptConditions(statmt))
				_currentStatement++;
			else if (isChoice._isChoice) {
				addSlot(statmt);
				_currentStatement++;
			} else if (choicesReady())
				updateChoiceStates();
			else if (_action && _action->isEnabled()) {
				_action->update(dt);
				return;
			} else {
				run(statmt);
				if (_lbl && (_currentStatement == _lbl->_stmts.size()))
					gotoNextLabel();
			}
		}
		if (choicesReady())
			updateChoiceStates();
		else if (!_action || !_action->isEnabled())
			_state = None;
	}
}

static Common::String text(const Common::String &txt) {
	Common::String result = g_engine->getTextDb().getText(txt);
	result = remove(result, '(', ')');
	result = remove(result, '{', '}');
	return result;
}

void Dialog::addSlot(YStatement *stmt) {
	YChoice *choice = (YChoice *)stmt->_exp.get();
	if ((!_slots[choice->_number - 1]._isValid) && (numSlots() < _context.limit)) {
		DialogSlot *slot = &_slots[choice->_number - 1];
		slot->_text.setFont("sayline");
		slot->_text.setText(Common::String::format("● %s", text(choice->_text).c_str()));
		slot->_stmt = stmt;
		slot->_dlg = this;
		slot->setPos(Math::Vector2d(SLOTMARGIN, SLOTMARGIN + slot->_text.getBounds().getY() * (MAXCHOICES - numSlots())));
		slot->_isValid = true;
	}
}

int Dialog::numSlots() const {
	int num = 0;
	for (size_t i = 0; i < MAXDIALOGSLOTS; i++) {
		if (_slots[i]._isValid)
			num++;
	}
	return num;
}

void Dialog::clearSlots() {
	for (size_t i = 0; i < MAXDIALOGSLOTS; i++) {
		_slots[i]._isValid = false;
	}
}

void Dialog::drawCore(Math::Matrix4 trsf) {
	for (size_t i = 0; i < MAXDIALOGSLOTS; i++) {
		DialogSlot *slot = &_slots[i];
		if (slot->_isValid) {
			Math::Matrix4 t(trsf);
			t.translate(Math::Vector3d(slot->getPos().getX(), slot->getPos().getY(), 0.f));
			slot->_text.draw(g_engine->getGfx(), t);
		}
	}
}

} // namespace Twp
