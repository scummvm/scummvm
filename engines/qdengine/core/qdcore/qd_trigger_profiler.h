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


#ifndef QDENGINE_CORE_QDCORE_QD_TRIGGER_PROFILER_H
#define QDENGINE_CORE_QDCORE_QD_TRIGGER_PROFILER_H

#define __QD_TRIGGER_PROFILER__

#ifdef __QD_TRIGGER_PROFILER__

#include <vector>


namespace QDEngine {

class XStream;

class qdTriggerElement;
class qdTriggerLink;
class qdTriggerChain;

#ifdef _QUEST_EDITOR
#include <boost/intrusive_ptr.hpp>
typedef boost::intrusive_ptr<qdTriggerElement> qdTriggerElementPtr;
typedef boost::intrusive_ptr<qdTriggerElement const> qdTriggerElementConstPtr;
#else
typedef qdTriggerElement *qdTriggerElementPtr;
typedef qdTriggerElement const *qdTriggerElementConstPtr;
#endif // _QUEST_EDITOR

//! Запись в базе профайлера триггеров.
class qdTriggerProfilerRecord {
public:
	enum event_t {
		//! изменение состояния элемента триггера
		ELEMENT_STATUS_UPDATE,
		//! изменение состояния линка триггера
		PARENT_LINK_STATUS_UPDATE,
		//! изменение состояния линка триггера
		CHILD_LINK_STATUS_UPDATE,
	};

	qdTriggerProfilerRecord();
	qdTriggerProfilerRecord(unsigned int tm, event_t ev, const qdTriggerChain *trigger, int el_id, int lnk_id, int st);
	qdTriggerProfilerRecord(const qdTriggerProfilerRecord &rec);
	~qdTriggerProfilerRecord();

	qdTriggerProfilerRecord &operator = (const qdTriggerProfilerRecord &rec);

	bool save(XStream &fh) const;
	bool load(XStream &fh);

	//! Время события (в миллисекундах от старта приложения).
	unsigned int time() const {
		return time_;
	}
	//! Идентификатор события.
	event_t event() const {
		return event_;
	}
	//! Идентификатор триггера (номер триггера в списке гейм-диспетчера).
	int trigger_id() const {
		return trigger_id_;
	}
	//! Идентификатор элемента триггера.
	int element_id() const {
		return element_id_;
	}
	//! Идентификатор линка триггера.
	int link_id() const {
		return link_id_;
	}
	//! Новое состояние элемента/линка.
	int status() const {
		return status_;
	}

private:

	//! Время события (в миллисекундах от старта приложения).
	unsigned int time_;

	//! Идентификатор события.
	event_t event_;

	//! Идентификатор триггера (номер триггера в списке гейм-диспетчера).
	int trigger_id_;
	//! Идентификатор элемента триггера.
	int element_id_;
	//! Идентификатор линка триггера.
	int link_id_;
	//! Новое состояние элемента/линка.
	int status_;
};

//! Профайлер триггеров.
class qdTriggerProfiler {
public:
	//! Формат описания записи профайлера.
	enum {
		//! отображать время события
		PROFILER_TEXT_TIME      = 0x01,
		//! отображать имя триггера
		PROFILER_TEXT_TRIGGER_NAME  = 0x02,
		//! отображать имя сцены
		PROFILER_TEXT_SCENE_NAME    = 0x04
	};

	~qdTriggerProfiler();

	bool save_to_work_file() const;
	bool load_from_work_file();

	void add_record(const qdTriggerProfilerRecord &rec) {
		if (is_logging_enabled_ && !is_read_only_) records_.push_back(rec);
	}

	int num_records() const {
		return records_.size();
	}

	bool is_logging_enabled() const {
		return is_logging_enabled_;
	}
	void enable() {
		is_logging_enabled_ = true;
	}
	void disable() {
		is_logging_enabled_ = false;
	}

	void set_read_only(bool v) {
		is_read_only_ = v;
	}
	bool is_read_only() const {
		return is_read_only_;
	}

	typedef std::vector<qdTriggerProfilerRecord> record_container_t;
	typedef record_container_t::const_iterator record_iterator_t;

	record_iterator_t records_begin() const {
		return records_.begin();
	}
	record_iterator_t records_end() const {
		return records_.end();
	}
	record_iterator_t get_record(int record_num = 0) const {
		return records_.begin() + record_num;
	}

	bool evolve(int record_num) const;

	static qdTriggerProfiler &instance();

	static const char *const activation_comline() {
		return activation_comline_;
	}

	static const char *record_text(const qdTriggerProfilerRecord &rec, char separator = ' ');

	static int record_text_format() {
		return record_text_format_;
	}
	static void set_record_text_format(int fmt) {
		record_text_format_ = fmt;
	}

	const char *work_file() const {
		return work_file_.c_str();
	}
	void set_work_file(const char *fname);

private:

	qdTriggerProfiler();

	record_container_t records_;

	bool is_logging_enabled_;
	bool is_read_only_;

	std::string work_file_;

	static int record_text_format_;

	static const char *const activation_comline_;

	static const char *element_text(qdTriggerElementPtr el);

	static qdTriggerElementPtr get_record_element(const qdTriggerProfilerRecord &rec);
	static qdTriggerLink *get_record_link(const qdTriggerProfilerRecord &rec);
	static qdTriggerChain  *get_record_trigger(const qdTriggerProfilerRecord &rec);
};

} // namespace QDEngine

#endif /* __QD_TRIGGER_PROFILER__ */

#endif // QDENGINE_CORE_QDCORE_QD_TRIGGER_PROFILER_H
