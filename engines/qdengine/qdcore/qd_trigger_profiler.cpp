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

/* ---------------------------- INCLUDE SECTION ----------------------------- */
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/qdcore/qd_trigger_profiler.h"

#ifdef __QD_TRIGGER_PROFILER__

#include "qdengine/qdcore/qd_trigger_chain.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"


namespace QDEngine {

const char *const qdTriggerProfiler::_activation_comline = "trigger_profiler";

int qdTriggerProfiler::_record_text_format = PROFILER_TEXT_TIME | PROFILER_TEXT_TRIGGER_NAME | PROFILER_TEXT_SCENE_NAME;

qdTriggerProfilerRecord::qdTriggerProfilerRecord() : time_(0),
	event_(ELEMENT_STATUS_UPDATE),
	trigger_id_(0),
	element_id_(0),
	link_id_(0),
	status_(0) {
}

qdTriggerProfilerRecord::qdTriggerProfilerRecord(uint32 tm, event_t ev, const qdTriggerChain *trigger, int el_id, int lnk_id, int st) : time_(tm),
	event_(ev),
	trigger_id_(0),
	element_id_(el_id),
	link_id_(lnk_id),
	status_(st) {
	if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
		for (qdTriggerChainList::const_iterator it = p->trigger_chain_list().begin(); it != p->trigger_chain_list().end(); ++it) {
			if (*it == trigger) break;
			trigger_id_++;
		}
	}
}

qdTriggerProfilerRecord::qdTriggerProfilerRecord(const qdTriggerProfilerRecord &rec) {
	*this = rec;
}

qdTriggerProfilerRecord::~qdTriggerProfilerRecord() {
}

qdTriggerProfilerRecord &qdTriggerProfilerRecord::operator = (const qdTriggerProfilerRecord &rec) {
	if (this == &rec) return *this;

	time_ = rec.time_;

	event_ = rec.event_;

	trigger_id_ = rec.trigger_id_;
	element_id_ = rec.element_id_;
	link_id_ = rec.link_id_;
	status_ = rec.status_;

	return *this;
}

bool qdTriggerProfilerRecord::save(Common::WriteStream &fh) const {
	warning("STUB: Test qdTriggerProfilerRecord::save(Common::WriteStream &fh)");

	fh.writeUint32LE(time_);
	fh.writeSint32LE((int)event_);
	fh.writeSint32LE(trigger_id_);
	fh.writeSint32LE(element_id_);
	fh.writeSint32LE(link_id_);
	fh.writeSint32LE(status_);

	return true;
}

bool qdTriggerProfilerRecord::load(Common::SeekableReadStream &fh) {
	warning("STUB: Test qdTriggerProfilerRecord::load(Common::SeekableReadStream &fh)");
	int ev;

	time_ = fh.readUint32LE();
	ev = fh.readSint32LE();
	trigger_id_ = fh.readSint32LE();
	element_id_ = fh.readSint32LE();
	link_id_ = fh.readSint32LE();
	status_ = fh.readSint32LE();

	event_ = event_t(ev);
	return true;
}

qdTriggerProfiler::qdTriggerProfiler() : _is_logging_enabled(false), _is_read_only(true) {
	_work_file = "trigger_profiler.dat";
}

qdTriggerProfiler::~qdTriggerProfiler() {
	if (_is_logging_enabled)
		save_to_work_file();
}

bool qdTriggerProfiler::save_to_work_file() const {
	warning("STUB: qdTriggerProfiler::save_to_work_file()");
	Common::DumpFile fh;

	fh.writeUint32LE(_records.size());
	for (auto &it : _records) {
		it.save(fh);
	}

	fh.close();
	return true;
}

bool qdTriggerProfiler::load_from_work_file() {
	warning("STUB: qdTriggerProfiler::load_from_work_file()");
	Common::File fh;

	_records.clear();

	if (fh.open(_work_file.c_str())) {
		int size;
		size = fh.readSint32LE();
		_records.resize(size);
		for (record_container_t::iterator it = _records.begin(); it != _records.end(); ++it)
			it->load(fh);

		fh.close();
		return true;
	}

	return false;
}

qdTriggerElementPtr qdTriggerProfiler::get_record_element(const qdTriggerProfilerRecord &rec) {
	if (qdTriggerChain * p = get_record_trigger(rec))
		return p->search_element(rec.element_id());

	return NULL;
}

qdTriggerLink *qdTriggerProfiler::get_record_link(const qdTriggerProfilerRecord &rec) {
	if (qdTriggerElementPtr p = get_record_element(rec)) {
		if (rec.event() == qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE)
			return p->find_child_link(rec.link_id());
		else
			return p->find_parent_link(rec.link_id());
	}

	return NULL;
}

qdTriggerChain *qdTriggerProfiler::get_record_trigger(const qdTriggerProfilerRecord &rec) {
	if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher()) {
		qdTriggerChainList::const_iterator it = p->trigger_chain_list().begin();
		for (int i = 0; i < rec.trigger_id(); i++) {
			if (it != p->trigger_chain_list().end()) ++it;
		}

		if (it != p->trigger_chain_list().end()) return *it;
	}

	return NULL;
}

bool qdTriggerProfiler::evolve(int record_num) const {
	assert(record_num >= 0 && record_num < _records.size());

	if (qdGameDispatcher * p = qdGameDispatcher::get_dispatcher())
		p->reset_triggers();

	for (int i = 0; i <= record_num; i++) {
		switch (_records[i].event()) {
		case qdTriggerProfilerRecord::ELEMENT_STATUS_UPDATE:
			if (qdTriggerElementPtr p = get_record_element(_records[i]))
				p->set_status(qdTriggerElement::ElementStatus(_records[i].status()));
			break;
		case qdTriggerProfilerRecord::PARENT_LINK_STATUS_UPDATE:
		case qdTriggerProfilerRecord::CHILD_LINK_STATUS_UPDATE:
			if (qdTriggerLink * p = get_record_link(_records[i]))
				p->set_status(qdTriggerLink::LinkStatus(_records[i].status()));
			break;
		}
	}

	return true;
}

qdTriggerProfiler &qdTriggerProfiler::instance() {
	static qdTriggerProfiler pr;

	return pr;
}

void qdTriggerProfiler::set_work_file(const char *fname) {
	if (NULL != fname) _work_file = fname;
	else _work_file.clear();
}

#endif /* __QD_TRIGGER_PROFILER__ */

} // namespace QDEngine
