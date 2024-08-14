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

#include "common/debug.h"
#include "common/stream.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"

#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"

#include "qdengine/qdcore/qd_trigger_element.h"
#include "qdengine/qdcore/qd_trigger_chain.h"
#include "qdengine/qdcore/qd_video.h"
#include "qdengine/qdcore/qd_minigame.h"
#include "qdengine/qdcore/qd_music_track.h"
#include "qdengine/qdcore/qd_game_scene.h"
#include "qdengine/qdcore/qd_grid_zone.h"
#include "qdengine/qdcore/qd_game_object_state.h"
#include "qdengine/qdcore/qd_game_object_animated.h"
#include "qdengine/qdcore/qd_named_object_indexer.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"

namespace QDEngine {

qdTriggerLink::qdTriggerLink(qdTriggerElementPtr p, int tp)
	: _element(p),
	  _element_ID(qdTriggerElement::INVALID_ID),
	  _type(tp),
	  _auto_restart(false) {
	if (_element)
		_element_ID = _element->ID();

	set_status(LINK_INACTIVE);
}

qdTriggerLink::qdTriggerLink() : _element(NULL),
	_element_ID(qdTriggerElement::INVALID_ID),
	_type(0),
	_auto_restart(false) {
	set_status(LINK_INACTIVE);
}

bool qdTriggerLink::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);
		switch (it->ID()) {
		case QDSCR_ID:
			set_element_ID(buf.get_int());
			break;
		case QDSCR_TYPE:
			_type = buf.get_int();
			break;
		case QDSCR_TRIGGER_ELEMENT_LINK_AUTO_RESTART:
			if (buf.get_int())
				_auto_restart = true;
			else
				_auto_restart = false;
			break;
		}
	}
	return true;
}

bool qdTriggerLink::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<link");

	if (_element) {
		fh.writeString(Common::String::format(" ID=\"%d\"", _element->ID()));
	}

	if (_type) {
		fh.writeString(Common::String::format(" type=\"%d\"", _type));
	}

	if (_auto_restart) {
		fh.writeString(" auto_restart=\"1\"");
	}

	fh.writeString("/>\r\n");
	return true;
}

qdTriggerElement::qdTriggerElement() : _object(NULL),
	_ID(0),
	_is_active(false),
	_status(TRIGGER_EL_INACTIVE) {
}

qdTriggerElement::qdTriggerElement(qdNamedObject *p) : _object(p),
	_ID(0),
	_is_active(false),
	_status(TRIGGER_EL_INACTIVE) {
	p->add_trigger_reference();
}

qdTriggerElement::~qdTriggerElement() {
	if (_object) _object->remove_trigger_reference();

	_parents.clear();
	_children.clear();
}

bool qdTriggerElement::check_external_conditions(int link_type) {
	if (_parents.empty()) return true;

	for (auto &it : _parents) {
		if (it.type() == link_type && it.element()->status() != TRIGGER_EL_DONE) {
			return false;
		}
	}

	return true;
}

bool qdTriggerElement::check_internal_conditions() {
	if (!_object) return true;

	if (qdConditionalObject * p = dynamic_cast<qdConditionalObject * >(_object)) {
		if (p->trigger_can_start())
			return p->check_conditions();
		else
			return false;
	}

	return true;
}

qdTriggerLink *qdTriggerElement::find_child_link(qdTriggerElementConstPtr ptrChild) {
	qdTriggerLinkList::iterator itr = Common::find(_children.begin(), _children.end(), ptrChild);

	if (itr != _children.end())
		return &*itr;
	return NULL;
}

qdTriggerLink *qdTriggerElement::find_child_link(int child_id) {
	for (qdTriggerLinkList::iterator it = _children.begin(); it != _children.end(); ++it) {
		if (it->element()->ID() == child_id)
			return &*it;
	}

	return NULL;
}

qdTriggerLink *qdTriggerElement::find_parent_link(qdTriggerElementConstPtr ptrParent) {
	qdTriggerLinkList::iterator itr = Common::find(_parents.begin(), _parents.end(), ptrParent);

	if (itr != _parents.end())
		return &*itr;
	return NULL;
}

qdTriggerLink *qdTriggerElement::find_parent_link(int parent_id) {
	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
		if (it->element()->ID() == parent_id)
			return &*it;
	}

	return NULL;
}

bool qdTriggerElement::is_parent(qdTriggerElementConstPtr p) {
	for (auto &it : _parents) {
		if (it.element() == p) return true;
	}

	return false;
}

bool qdTriggerElement::is_child(qdTriggerElementConstPtr p) {
	for (auto &it : _children) {
		if (it.element() == p) return true;
	}

	return false;
}

bool qdTriggerElement::add_parent(qdTriggerElementPtr p, int link_type) {
	if (p == this || is_parent(p)) return false;
	_parents.push_back(qdTriggerLink(p, link_type));
	return true;
}

bool qdTriggerElement::add_child(qdTriggerElementPtr p, int link_type, bool auto_restart) {
	if (p == this || is_child(p)) return false;
	_children.push_back(qdTriggerLink(p, link_type));
	if (auto_restart)
		_children.back().toggle_auto_restart(true);

	return true;
}

bool qdTriggerElement::remove_parent(qdTriggerElementPtr p) {
	for (auto it = _parents.begin(); it != _parents.end(); it++) {
		if ((*it).element() == p) {
			_parents.erase(it);
			return true;
		}
	}
	return false;
}

bool qdTriggerElement::remove_child(qdTriggerElementPtr p) {
	for (auto it = _children.begin(); it != _children.end(); it++) {
		if ((*it).element() == p) {
			_children.erase(it);
			return true;
		}
	}
	return false;
}

bool qdTriggerElement::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_ID:
			set_id(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_NAMED_OBJECT: {
			qdNamedObjectReference ref;
			ref.load_script(&*it);
			retrieve_object(ref);
		}
		break;
		case QDSCR_TRIGGER_START_ELEMENT:
			if (xml::tag_buffer(*it).get_int())
				_is_active = true;
			else
				_is_active = false;
			break;
		case QDSCR_TRIGGER_ELEMENT_PARENT_LINKS:
			load_links_script(&*it, true);
			break;
		case QDSCR_TRIGGER_ELEMENT_CHILD_LINKS:
			load_links_script(&*it, false);
			break;
		}
	}

	set_status(TRIGGER_EL_INACTIVE);
	return true;
}

bool qdTriggerElement::load_links_script(const xml::tag *p, bool load_parents) {
	qdTriggerLinkList &lst = (load_parents) ? _parents : _children;

	int size = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		if (it->ID() == QDSCR_TRIGGER_ELEMENT_LINK)
			size++;
	}

	if (size)
		lst.resize(size);

	int link_idx = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		if (it->ID() == QDSCR_TRIGGER_ELEMENT_LINK) {
			if (link_idx < size)
				lst[link_idx++].load_script(&*it);
		}
	}

	return true;
}

bool qdTriggerElement::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	if (_ID == ROOT_ID) {
		fh.writeString("<trigger_chain_root");
	} else {
		fh.writeString(Common::String::format("<trigger_element ID=\"%d\"", _ID));
	}

	if (_is_active) {
		fh.writeString(" start_element=\"1\"");
	}

	fh.writeString(">\r\n");

	if (_object) {
		qdNamedObjectReference ref(_object);
		ref.save_script(fh, indent + 1);
	}

	if (_parents.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<parent_links>\r\n");

		for (auto &it: _parents) {
			it.save_script(fh, indent + 2);
		}

		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("</parent_links>\r\n");
	}

	if (_children.size()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<child_links>\r\n");

		for (auto &it : _children) {
			it.save_script(fh, indent + 2);
		}

		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("</child_links>\r\n");
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	if (_ID == ROOT_ID) {
		fh.writeString("</trigger_chain_root>\r\n");
	} else {
		fh.writeString("</trigger_element>\r\n");
	}

	return true;
}

bool qdTriggerElement::retrieve_object(const qdNamedObjectReference &ref) {
	if (qdGameDispatcher * dsp = qdGameDispatcher::get_dispatcher()) {
		if (_object) _object->remove_trigger_reference();
		_object = dsp->get_named_object(&ref);

		if (!_object) {
			qdNamedObjectReference &r = qdNamedObjectIndexer::instance().add_reference((qdNamedObject *&)_object);
			r = ref;

#ifdef __QD_DEBUG_ENABLE__
			debugC(3, kDebugLog, "qdTriggerElement::retrieve_object() failed");
			debugC(3, kDebugLog, "%s", ref.toString().c_str());
#endif
			return false;
		}

		_object->add_trigger_reference();
	}

	return false;
}

bool qdTriggerElement::retrieve_link_elements(qdTriggerChain *p) {
	bool result = true;

	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
		if (!it->element()) {
			if (qdTriggerElementPtr el = p->search_element(it->element_ID())) {
				it->set_element(el);
			} else {
				debugC(3, kDebugLog, "parent link element not found: %d->%d", ID(), it->element_ID());
				result = false;
			}
		}
	}

	for (qdTriggerLinkList::iterator it = _children.begin(); it != _children.end(); ++it) {
		if (!it->element()) {
			if (qdTriggerElementPtr el = p->search_element(it->element_ID())) {
				it->set_element(el);
			} else {
				debugC(3, kDebugLog, "child link element not found: %d->%d", ID(), it->element_ID());
				result = false;
			}
		}
	}

	return result;
}

bool qdTriggerElement::quant(float dt) {
	bool ret = false;

	for (auto &it : _children) {
		if (it.status() == qdTriggerLink::LINK_ACTIVE) {
			if (it.element()->conditions_quant(it.type())) {
				if (!it.auto_restart())
					it.set_status(qdTriggerLink::LINK_DONE);
				ret = true;
			}
		}
	}

	return ret;
}

void qdTriggerElement::start() {
	if (qdConditionalObject * p = dynamic_cast<qdConditionalObject *>(_object)) {
		switch (p->trigger_start()) {
		case qdConditionalObject::TRIGGER_START_ACTIVATE:
			set_status(TRIGGER_EL_WORKING);

			for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
				if (qdTriggerLink * lp = it->element()->find_child_link(this)) {
					if (lp->status() == qdTriggerLink::LINK_ACTIVE) {
						it->element()->activate_links(this);
						it->element()->deactivate_links(this);
					}
				}
			}
			break;
		case qdConditionalObject::TRIGGER_START_WAIT:
			set_status(TRIGGER_EL_WAITING);
			break;
		default:
			break;
		}
	} else
		set_status(TRIGGER_EL_WAITING);
}

bool qdTriggerElement::conditions_quant(int link_type) {
	switch (_status) {
	case TRIGGER_EL_DONE:
	case TRIGGER_EL_INACTIVE:
		if (check_external_conditions(link_type) && check_internal_conditions() && _object) {
			if (qdCondition::successful_click())
				qdGameDispatcher::get_dispatcher()->drop_flag(qdGameDispatcher::CLICK_FAILED_FLAG);
			if (qdCondition::successful_object_click())
				qdGameDispatcher::get_dispatcher()->drop_flag(qdGameDispatcher::OBJECT_CLICK_FAILED_FLAG);
			start();
		} else {
			if (_object && _object->named_object_type() == QD_NAMED_OBJECT_OBJ_STATE) {
				qdGameObjectState *p = static_cast<qdGameObjectState *>(_object);
				if (p->has_text() && p->check_flag(qdGameObjectState::QD_OBJ_STATE_FLAG_DIALOG_PHRASE)) {
					if (qdGameDispatcher * gp = qd_get_game_dispatcher())
						gp->add_dialog_state(p);
				}
			}
		}
		break;
	case TRIGGER_EL_WAITING:
		if (_object && _object->named_object_type() == QD_NAMED_OBJECT_OBJ_STATE) {
			qdGameObjectState *p = static_cast<qdGameObjectState *>(_object);
			qdGameObjectAnimated *obj = static_cast<qdGameObjectAnimated *>(_object->owner());

			switch (obj->state_status(p)) {
			case qdGameObjectAnimated::STATE_INACTIVE:
				set_status(TRIGGER_EL_INACTIVE);
				break;
			case qdGameObjectAnimated::STATE_ACTIVE:
			case qdGameObjectAnimated::STATE_DONE:
				set_status(TRIGGER_EL_WORKING);
				break;
			default:
				break;
			}
		} else
			set_status(TRIGGER_EL_WORKING);

		if (status() == TRIGGER_EL_WORKING) {
			for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
				if (qdTriggerLink * lp = it->element()->find_child_link(this)) {
					if (lp->status() != qdTriggerLink::LINK_INACTIVE) {
						it->element()->activate_links(this);
						it->element()->deactivate_links(this);
					}
				}
			}
		}
		break;
	case TRIGGER_EL_WORKING:
		if (_object && _object->named_object_type() == QD_NAMED_OBJECT_OBJ_STATE) {
			qdGameObjectState *p = static_cast<qdGameObjectState *>(_object);
			qdGameObjectAnimated *obj = static_cast<qdGameObjectAnimated *>(_object->owner());

			switch (obj->state_status(p)) {
			case qdGameObjectAnimated::STATE_INACTIVE:
				set_status(TRIGGER_EL_DONE);
				break;
			case qdGameObjectAnimated::STATE_ACTIVE:
				break;
			case qdGameObjectAnimated::STATE_DONE:
				obj->handle_state_end();
				set_status(TRIGGER_EL_DONE);
				break;
			default:
				break;
			}
		} else
			set_status(TRIGGER_EL_DONE);

		if (_status == TRIGGER_EL_DONE) {
			for (auto &it : _parents)
				it.element()->deactivate_link(this);

			for (auto &it : _children) {
				it.activate();
			}

			return true;
		}
		break;
//		case TRIGGER_EL_DONE:
//			break;
		default:
			break;
	}

	return false;
}

bool qdTriggerElement::activate_links(qdTriggerElementPtr child) {
	int link_type = -1;
	for (auto &it : _children) {
		if (it.element() == child) {
			link_type = it.type();
			break;
		}
	}

	if (link_type == -1) return false;

	for (auto &it : _children) {
		if (it.type() == link_type) {
			if (it.element() != child && it.status() == qdTriggerLink::LINK_INACTIVE) {
				it.activate();
			}
		}
	}

	return true;
}

bool qdTriggerElement::deactivate_links(qdTriggerElementPtr child) {
	int link_type = -1;
	for (auto &it : _children) {
		if (it.element() == child) {
			link_type = it.type();
			break;
		}
	}

	if (link_type == -1) return false;

	for (auto &it : _children) {
		if (it.type() != link_type) {
			it.deactivate();
//			it->element()->set_status(qdTriggerElement::TRIGGER_EL_INACTIVE);
		}
	}

	return true;
}

bool qdTriggerElement::deactivate_link(qdTriggerElementPtr child) {
	for (auto &it : _children) {
		if (it.element() == child) {
			it.deactivate();
			return true;
		}
	}

	return false;
}

bool qdTriggerElement::debug_set_active() {
	set_status(TRIGGER_EL_INACTIVE);

	for (qdTriggerLinkList::iterator it = _children.begin(); it != _children.end(); ++it) {
		if (it->element()->status() == TRIGGER_EL_DONE && !it->element()->is_active())
			it->element()->debug_set_inactive();
	}

	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
		it->element()->set_child_link_status(this, qdTriggerLink::LINK_ACTIVE);
		it->element()->set_status(TRIGGER_EL_DONE);
	}

	return true;
}

bool qdTriggerElement::debug_set_inactive() {
	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it)
		it->element()->set_child_link_status(this, qdTriggerLink::LINK_INACTIVE);

	set_status(TRIGGER_EL_INACTIVE);

	for (qdTriggerLinkList::iterator it = _children.begin(); it != _children.end(); ++it) {
		if (it->element()->status() == TRIGGER_EL_DONE && !it->element()->is_active())
			it->element()->debug_set_inactive();
	}

	return true;
}

bool qdTriggerElement::debug_set_done() {
	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it)
		it->element()->set_child_link_status(this, qdTriggerLink::LINK_DONE);

	set_status(TRIGGER_EL_DONE);

	return true;
}

bool qdTriggerElement::set_child_link_status(qdTriggerElementConstPtr child, qdTriggerLink::LinkStatus st) {
	if (qdTriggerLink * p = find_child_link(child)) {
		if (!p->auto_restart() || st == qdTriggerLink::LINK_ACTIVE) {
			p->set_status(st);
		}
		return true;
	}

	return false;
}

bool qdTriggerElement::set_parent_link_status(qdTriggerElementConstPtr parent, qdTriggerLink::LinkStatus st) {
	if (qdTriggerLink * p = find_parent_link(parent)) {
		p->set_status(st);
		return true;
	}

	return false;
}

bool qdTriggerElement::load_data(Common::SeekableReadStream &fh, int saveVersion) {
	debugC(5, kDebugSave, "      qdTriggerElement::load_data before: %ld", fh.pos());
	char st = fh.readByte();
	set_status(ElementStatus(st));

	for (auto &it : _parents) {
		st = fh.readByte();
		it.set_status(qdTriggerLink::LinkStatus(st));
	}

	for (auto &it : _children) {
		st = fh.readByte();
		it.set_status(qdTriggerLink::LinkStatus(st));
	}

	debugC(5, kDebugSave, "      qdTriggerElement::load_data after: %ld", fh.pos());
	return true;
}

bool qdTriggerElement::save_data(Common::WriteStream &fh) const {
	debugC(5, kDebugSave, "      qdTriggerElement::save_data before: %ld", fh.pos());
	/*  switch(_status){
	    case TRIGGER_EL_INACTIVE:
	        for(qdTriggerLinkList::const_iterator it = _parents.begin(); it != _parents.end(); ++it){
	            if(it->status() != qdTriggerLink::LINK_INACTIVE)
	                break;
	        }
	        for(qdTriggerLinkList::const_iterator it = _children.begin(); it != _children.end(); ++it){
	            if(it->status() != qdTriggerLink::LINK_INACTIVE)
	                break;
	        }
	        fh < char(TRIGGER_EL_INACTIVE_ALL);
	        return true;
	    case TRIGGER_EL_DONE:
	        for(qdTriggerLinkList::const_iterator it = _parents.begin(); it != _parents.end(); ++it){
	            if(it->status() != qdTriggerLink::LINK_DONE)
	                break;
	        }
	        for(qdTriggerLinkList::const_iterator it = _children.begin(); it != _children.end(); ++it){
	            if(it->status() != qdTriggerLink::LINK_DONE)
	                break;
	        }
	        fh < char(TRIGGER_EL_DONE_ALL);
	        return true;
	    }*/

	fh.writeByte(_status);

	for (auto &it : _parents) {
		fh.writeByte(it.status());
	}

	for (auto &it : _children) {
		fh.writeByte(it.status());
	}

	debugC(5, kDebugSave, "      qdTriggerElement::save_data after: %ld", fh.pos());
	return true;
}

void qdTriggerLink::activate() {
	set_status(LINK_ACTIVE);

//	if(_element && _element->status() != qdTriggerElement::TRIGGER_EL_INACTIVE)
//		_element->set_status(qdTriggerElement::TRIGGER_EL_INACTIVE);

// debugC(3, kDebugLog, "%p", this);
}

void qdTriggerLink::deactivate() {
	if (_status == LINK_ACTIVE) {
		if (!auto_restart())
			set_status(LINK_INACTIVE);
#ifdef __QD_DEBUG_ENABLE__
		else
			debugC(3, kDebugLog, "%s", toString().c_str());
#endif
	}
}


Common::String qdTriggerLink::toString() {
	Common::String res;

	if (element()) {
		if (qdNamedObject *p = element()->object()) {
			res = Common::String::format("->type %d ", type());

			switch (status()) {
			case qdTriggerLink::LINK_ACTIVE:
				res += " on ";
				break;
			case qdTriggerLink::LINK_INACTIVE:
				res += " off ";
				break;
			case qdTriggerLink::LINK_DONE:
				res += " cut ";
				break;
			}

			if (p->named_object_type() == QD_NAMED_OBJECT_OBJ_STATE) {
				if (p->owner() && p->owner()->name())
					res += Common::String((char *)transCyrillic(p->owner()->name()));
			}
			res += Common::String((char *)transCyrillic(p->name()));
		}
	}

	return res;
}

Common::String qdNamedObjectReference::toString() const {
	Common::String res;

	for (int i = 0; i < num_levels(); i++) {
		if (i)
			res += Common::String("::");
		res += Common::String((char *)transCyrillic(object_name(num_levels() - i - 1)));
	}

	return res;
}

void qdTriggerElement::set_status(ElementStatus st) {
	_status = st;
}

void qdTriggerElement::reset() {
	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it)
		it->set_status(qdTriggerLink::LINK_INACTIVE);
	for (qdTriggerLinkList::iterator it = _children.begin(); it != _children.end(); ++it)
		it->set_status(qdTriggerLink::LINK_INACTIVE);

	set_status(TRIGGER_EL_INACTIVE);
}

void qdTriggerElement::deactivate(const qdNamedObject *ignore_object) {
	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
		if (!ignore_object || it->element()->object() != ignore_object) {
			if (!it->auto_restart())
				it->set_status(qdTriggerLink::LINK_INACTIVE);
		}
	}
	for (qdTriggerLinkList::iterator it = _children.begin(); it != _children.end(); ++it) {
		if (!ignore_object || it->element()->object() != ignore_object) {
			if (!it->auto_restart())
				it->set_status(qdTriggerLink::LINK_INACTIVE);
		}
	}

	set_status(TRIGGER_EL_INACTIVE);

	for (qdTriggerLinkList::iterator it = _parents.begin(); it != _parents.end(); ++it) {
		if (!ignore_object || it->element()->object() != ignore_object)
			it->element()->set_child_link_status(this, qdTriggerLink::LINK_INACTIVE);
	}
}

bool qdTriggerElement::add_object_trigger_reference() {
	if (_object) {
		_object->add_trigger_reference();
		return true;
	}

	return false;
}

bool qdTriggerElement::clear_object_trigger_references() {
	if (_object) {
		_object->clear_trigger_references();
		return true;
	}

	return false;
}
} // namespace QDEngine
