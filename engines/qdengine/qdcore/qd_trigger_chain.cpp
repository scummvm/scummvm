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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_trigger_chain.h"

namespace QDEngine {

qdTriggerChain::qdTriggerChain() {
	root_element()->set_id(qdTriggerElement::ROOT_ID);
	root_element()->set_status(qdTriggerElement::TRIGGER_EL_DONE);
}

qdTriggerChain::~qdTriggerChain() {
	for (auto &it : _elements) {
		delete it;
	}

	_elements.clear();
}

bool qdTriggerChain::reindex_elements() {
	int id = 0;
	for (auto &it : _elements) {
		it->set_id(id++);
	}

	return true;
}

qdTriggerElementPtr qdTriggerChain::search_element(int id) {
	if (id == qdTriggerElement::ROOT_ID)
		return root_element();

	for (auto it : _elements)
		if (it->ID() == id)
			return it;

	return 0;
}

qdTriggerElementPtr qdTriggerChain::add_element(qdNamedObject *p) {
	if (!can_add_element(p)) return 0;

	qdTriggerElementPtr el = new qdTriggerElement(p);
	_elements.push_back(el);

	reindex_elements();

	return el;
}

bool qdTriggerChain::remove_element(qdTriggerElementPtr p, bool free_mem, bool relink_elements) {
	for (auto it = _elements.begin(); it != _elements.end(); it++) {
		if (*it == p) {
			if (relink_elements) {
				for (auto &it_c : (*it)->children()) {
					it_c.element()->remove_parent(*it);
				}

				for (auto &it_p : (*it)->parents()) {
					it_p.element()->remove_child(*it);
				}
				for (auto &it_c : (*it)->children()) {
					for (auto &it_p : (*it)->parents()) {
						it_c.element()->add_parent(it_p.element());
						it_p.element()->add_child(it_c.element());
					}
				}
			}

			if (free_mem)
				delete *it;

			_elements.erase(it);
			reindex_elements();

			return true;
		}
	}

	return false;
}

bool qdTriggerChain::can_add_element(const qdNamedObject *p) const {
	if (is_element_in_list(p) && p->named_object_type() != QD_NAMED_OBJECT_SCENE) return false;

	return true;
}

bool qdTriggerChain::init_elements() {
	for (auto &it : _elements) {
		it->clear_object_trigger_references();
	}

	for (auto &it : _elements) {
		it->add_object_trigger_reference();
	}


	return true;
}

bool qdTriggerChain::is_element_in_list(qdNamedObject const *p) const {
	for (auto &it : _elements) {
		if (it->object() == p)
			return true;
	}
	return false;
}

bool qdTriggerChain::is_element_in_list(qdTriggerElementConstPtr p) const {
	for (auto &it : _elements) {
		if (it == p || (it->object() && it->object() == p->object()))
			return true;
	}
	return false;
}

bool qdTriggerChain::add_link(qdTriggerElementPtr from, qdTriggerElementPtr to, int link_type, bool auto_restart) {
	if (!from->add_child(to, link_type, auto_restart)) return false;
	if (!to->add_parent(from, link_type)) return false;

	return true;
}

bool qdTriggerChain::remove_link(qdTriggerElementPtr from, qdTriggerElementPtr to) {
	if (!from->remove_child(to)) return false;
	if (!to->remove_parent(from)) return false;

	return true;
}

bool qdTriggerChain::load_script(const xml::tag *p) {
	int id0, id1, tp0;
	qdTriggerElementPtr el, el1;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_TRIGGER_CHAIN_ROOT:
			root_element()->load_script(&*it);
			root_element()->set_status(qdTriggerElement::TRIGGER_EL_DONE);
			break;
		case QDSCR_TRIGGER_ELEMENT:
			el = qdTriggerElementPtr(new qdTriggerElement);
			el->load_script(&*it);
			_elements.push_back(el);
			break;
		case QDSCR_TRIGGER_ELEMENT_LINK:
			xml::tag_buffer(*it) > id0 > id1 > tp0;

			el = search_element(id0);
			el1 = search_element(id1);

			if (el && el1) {
				bool auto_restart = false;
				if (const xml::tag * tp = it->search_subtag(QDSCR_TRIGGER_ELEMENT_LINK_AUTO_RESTART)) {
					if (xml::tag_buffer(*tp).get_int())
						auto_restart = true;
					else
						auto_restart = false;
				}
				add_link(el, el1, tp0, auto_restart);
			}
			break;
		}
	}

	root_element()->retrieve_link_elements(this);
	for (qdTriggerElementList::iterator it = _elements.begin(); it != _elements.end(); ++it)
		(*it)->retrieve_link_elements(this);

	for (qdTriggerLinkList::iterator itl = root_element()->children().begin();
	        itl != root_element()->children().end();
	        ++itl)
		itl->activate();

	return true;
}

bool qdTriggerChain::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<trigger_chain name=");

	if (name()) {
		fh.writeString(Common::String::format("\"%s\"", qdscr_XML_string(name())));
	} else {
		fh.writeString("\" \"");
	}

	fh.writeString(">\r\n");

	root_element()->save_script(fh, indent + 1);

	for (auto &it : _elements) {
		it->save_script(fh, indent + 1);
	};

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</trigger_chain>\r\n");

	return true;
}

void qdTriggerChain::quant(float dt) {
	root_element()->quant(dt);

	for (auto &it : _elements)
		it->quant(dt);
}

bool qdTriggerChain::init_debug_check() {
	root_element()->debug_set_done();
	for (qdTriggerElementList::iterator it = _elements.begin(); it != _elements.end(); ++it)
		(*it)->debug_set_done();

	if (root_element()->is_active()) {
		root_element()->debug_set_active();
		root_element()->set_status(qdTriggerElement::TRIGGER_EL_DONE);

		for (qdTriggerLinkList::iterator itl = root_element()->children().begin(); itl != root_element()->children().end(); ++itl) {
			itl->activate();
		}

	}

	for (qdTriggerElementList::iterator it = _elements.begin(); it != _elements.end(); ++it) {
		if ((*it)->is_active())
			(*it)->debug_set_active();
	}

	return true;
}

const char *qdTriggerChain::debug_comline() {
	static const char  *arg = "triggers_debug";
	return arg;
}

bool qdTriggerChain::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(4, kDebugSave, "    qdTriggerChain::load_data before: %ld", fh.pos());
	int32 size = fh.readSint32LE();

	if (size != (int)_elements.size()) {
		return false;
	}

	if (!root_element()->load_data(fh, save_version)) {
		return false;
	}

	for (auto &it : _elements) {
		if (!it->load_data(fh, save_version)) {
			return false;
		}
	}
	debugC(4, kDebugSave, "    qdTriggerChain::load_data after: %ld", fh.pos());

	return true;
}

bool qdTriggerChain::save_data(Common::WriteStream &fh) const {
	debugC(4, kDebugSave, "    qdTriggerChain::save_data before: %ld", fh.pos());
	fh.writeUint32LE(_elements.size());

	if (!root_element()->save_data(fh)) {
		return false;
	}

	for (auto &it : _elements) {
		if (!it->save_data(fh)) {
			return false;
		}
	}

	debugC(4, kDebugSave, "    qdTriggerChain::save_data after: %ld", fh.pos());
	return true;
}

void qdTriggerChain::reset() {
	root_element()->reset();

	for (qdTriggerElementList::iterator it = _elements.begin(); it != _elements.end(); ++it)
		(*it)->reset();

	root_element()->set_status(qdTriggerElement::TRIGGER_EL_DONE);

	for (qdTriggerLinkList::iterator itl = root_element()->children().begin(); itl != root_element()->children().end(); ++itl)
		itl->activate();
}

bool qdTriggerChain::activate_links(const qdNamedObject *from) {
	bool ret = false;

	for (qdTriggerElementList::const_iterator it = _elements.begin(); it != _elements.end(); ++it) {
		if ((*it)->object() == from) {
			for (qdTriggerLinkList::iterator itl = (*it)->children().begin(); itl != (*it)->children().end(); ++itl)
				itl->activate();

			ret = true;
		}
	}

	return ret;
}

bool qdTriggerChain::deactivate_object_triggers(const qdNamedObject *p) {
	bool ret = false;

	for (qdTriggerElementList::const_iterator it = _elements.begin(); it != _elements.end(); ++it) {
		if ((*it)->object()) {
			const qdNamedObject *obj = (*it)->object()->owner(qdNamedObjectType(p->named_object_type()));
			if (obj == p) {
				(*it)->deactivate(p);
				ret = true;
			}
		}
	}

	return ret;
}
} // namespace QDEngine
