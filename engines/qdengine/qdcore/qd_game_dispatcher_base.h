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

#ifndef QDENGINE_QDCORE_QD_GAME_DISPATCHER_BASE_H
#define QDENGINE_QDCORE_QD_GAME_DISPATCHER_BASE_H

#include "qdengine/qdcore/qd_conditional_object.h"
#include "qdengine/qdcore/qd_object_map_container.h"
#include "qdengine/qdcore/qd_scale_info.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_fwd.h"

#ifdef __QD_DEBUG_ENABLE__
#include "qdengine/qdcore/qd_resource.h"
#endif

namespace QDEngine {

typedef void (*qdLoadingProgressFnc)(int percents_loaded, void *data);

class qdLoadingProgressInfo {
	uint32 _total_size;
	uint32 _loaded_size;
	void *_data;
	qdLoadingProgressFnc _progress_fnc;

public:

	qdLoadingProgressFnc set_callback_fnc(qdLoadingProgressFnc p, void *dp) {
		qdLoadingProgressFnc old_fnc = _progress_fnc;
		_progress_fnc = p;
		_data = dp;

		return old_fnc;
	}

	void set_total_size(int sz) {
		_total_size = sz;
		_loaded_size = 0;
	}
	int total_size() const {
		return _total_size;
	}

	void show_progress(int sz) {
		if (!_total_size || !_progress_fnc) return;
		_loaded_size += sz;
		uint32 percents = round(float(_loaded_size) / float(_total_size) * 100.0f);
		(*_progress_fnc)(percents, _data);
	}

	qdLoadingProgressInfo() {
		_progress_fnc = 0;
		_total_size = _loaded_size = 0;
		_data = 0;
	}
	~qdLoadingProgressInfo() { };
};

class qdGameDispatcherBase : public qdConditionalObject {
public:
	qdGameDispatcherBase();
	~qdGameDispatcherBase();

	void set_resources_size(int sz) {
		_loading_progress.set_total_size(sz);
	}

	qdLoadingProgressFnc set_loading_progress_callback(qdLoadingProgressFnc p, void *dp = 0) {
		return _loading_progress.set_callback_fnc(p, dp);
	}

	void show_loading_progress(int sz = 0);

	bool add_sound(qdSound *p);
	bool remove_sound(const char *name);
	bool remove_sound(qdSound *p);
	virtual qdSound *get_sound(const char *name);
	bool is_sound_in_list(const char *name);
	bool is_sound_in_list(qdSound *p);

	bool add_animation(qdAnimation *p);
	bool remove_animation(const char *name);
	bool remove_animation(qdAnimation *p);
	virtual qdAnimation *get_animation(const char *name);
	bool is_animation_in_list(const char *name);
	bool is_animation_in_list(qdAnimation *p);

	bool add_animation_set(qdAnimationSet *p);
	bool remove_animation_set(const char *name);
	bool remove_animation_set(qdAnimationSet *p);
	virtual qdAnimationSet *get_animation_set(const char *name);
	bool is_animation_set_in_list(const char *name);
	bool is_animation_set_in_list(qdAnimationSet *p);

	const qdSoundList &sound_list() const {
		return _sounds.get_list();
	}
	const qdAnimationList &animation_list() const {
		return _animations.get_list();
	}
	const qdAnimationSetList &animation_set_list() const {
		return _animation_sets.get_list();
	}

	virtual void load_script_body(const xml::tag *p);
	virtual bool save_script_body(Common::WriteStream &fh, int indent = 0) const;

	virtual int load_resources();
	void free_resources();

	virtual void quant(float dt) { };

	void add_scale_info(qdScaleInfo *p) {
		_scale_infos.push_back(*p);
	}
	bool get_object_scale(const char *p, float &sc);
	bool set_object_scale(const char *p, float sc);
	bool remove_object_scale(const char *p);

	virtual qdScaleInfo *get_scale_info(const char *p);

	virtual int get_resources_size();

	qdConditionalObject::trigger_start_mode trigger_start() {
		return qdConditionalObject::TRIGGER_START_FAILED;
	}

#ifdef __QD_DEBUG_ENABLE__
	virtual bool get_resources_info(qdResourceInfoContainer &infos) const;
#endif

private:

	qdObjectMapContainer<qdAnimation> _animations;
	qdObjectMapContainer<qdAnimationSet> _animation_sets;
	qdObjectMapContainer<qdSound> _sounds;

	typedef Std::vector<qdScaleInfo> scale_info_container_t;
	scale_info_container_t _scale_infos;

	qdLoadingProgressInfo _loading_progress;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_DISPATCHER_BASE_H
