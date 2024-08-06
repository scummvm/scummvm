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


#ifndef QDENGINE_QDCORE_QD_TRIGGER_PROFILER_H
#define QDENGINE_QDCORE_QD_TRIGGER_PROFILER_H

#include "common/stream.h"
#define __QD_TRIGGER_PROFILER__

#ifdef __QD_TRIGGER_PROFILER__

namespace QDEngine {

class qdTriggerElement;
class qdTriggerLink;
class qdTriggerChain;

typedef qdTriggerElement *qdTriggerElementPtr;
typedef qdTriggerElement const *qdTriggerElementConstPtr;

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
	qdTriggerProfilerRecord(uint32 tm, event_t ev, const qdTriggerChain *trigger, int el_id, int lnk_id, int st);
	qdTriggerProfilerRecord(const qdTriggerProfilerRecord &rec);
	~qdTriggerProfilerRecord();

	qdTriggerProfilerRecord &operator = (const qdTriggerProfilerRecord &rec);

	bool save(Common::WriteStream &fh) const;
	bool load(Common::SeekableReadStream &fh);

	//! Время события (в миллисекундах от старта приложения).
	uint32 time() const {
		return _time;
	}
	//! Идентификатор события.
	event_t event() const {
		return _event;
	}
	//! Идентификатор триггера (номер триггера в списке гейм-диспетчера).
	int trigger_id() const {
		return _trigger_id;
	}
	//! Идентификатор элемента триггера.
	int element_id() const {
		return _element_id;
	}
	//! Идентификатор линка триггера.
	int link_id() const {
		return _link_id;
	}
	//! Новое состояние элемента/линка.
	int status() const {
		return _status;
	}

private:

	//! Время события (в миллисекундах от старта приложения).
	uint32 _time;

	//! Идентификатор события.
	event_t _event;

	//! Идентификатор триггера (номер триггера в списке гейм-диспетчера).
	int _trigger_id;
	//! Идентификатор элемента триггера.
	int _element_id;
	//! Идентификатор линка триггера.
	int _link_id;
	//! Новое состояние элемента/линка.
	int _status;
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
		if (_is_logging_enabled && !_is_read_only) _records.push_back(rec);
	}

	int num_records() const {
		return _records.size();
	}

	bool is_logging_enabled() const {
		return _is_logging_enabled;
	}
	void enable() {
		_is_logging_enabled = true;
	}
	void disable() {
		_is_logging_enabled = false;
	}

	void set_read_only(bool v) {
		_is_read_only = v;
	}
	bool is_read_only() const {
		return _is_read_only;
	}

	typedef Std::vector<qdTriggerProfilerRecord> record_container_t;
	typedef record_container_t::const_iterator record_iterator_t;

	record_iterator_t records_begin() const {
		return _records.begin();
	}
	record_iterator_t records_end() const {
		return _records.end();
	}
	record_iterator_t get_record(int record_num = 0) const {
		return _records.begin() + record_num;
	}

	bool evolve(int record_num) const;

	static qdTriggerProfiler &instance();

	static const char *activation_comline() {
		return _activation_comline;
	}

	static int record_text_format() {
		return _record_text_format;
	}
	static void set_record_text_format(int fmt) {
		_record_text_format = fmt;
	}

	const char *work_file() const {
		return _work_file.c_str();
	}
	void set_work_file(const char *fname);

private:

	qdTriggerProfiler();

	record_container_t _records;

	bool _is_logging_enabled;
	bool _is_read_only;

	Common::String _work_file;

	static int _record_text_format;

	static const char *const _activation_comline;


	static qdTriggerElementPtr get_record_element(const qdTriggerProfilerRecord &rec);
	static qdTriggerLink *get_record_link(const qdTriggerProfilerRecord &rec);
	static qdTriggerChain  *get_record_trigger(const qdTriggerProfilerRecord &rec);
};

} // namespace QDEngine

#endif /* __QD_TRIGGER_PROFILER__ */

#endif // QDENGINE_QDCORE_QD_TRIGGER_PROFILER_H
