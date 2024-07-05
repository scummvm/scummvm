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

#include "qdengine/parser/xml_fwd.h"

#ifdef __QD_DEBUG_ENABLE__
#include "qdengine/qdcore/qd_resource.h"
#endif

namespace QDEngine {

typedef void (*qdLoadingProgressFnc)(int percents_loaded, void *data);

class qdLoadingProgressInfo {
	unsigned total_size_;
	unsigned loaded_size_;
	void *data_;
	qdLoadingProgressFnc progress_fnc_;

public:

	qdLoadingProgressFnc set_callback_fnc(qdLoadingProgressFnc p, void *dp) {
		qdLoadingProgressFnc old_fnc = progress_fnc_;
		progress_fnc_ = p;
		data_ = dp;

		return old_fnc;
	}

	void set_total_size(int sz) {
		total_size_ = sz;
		loaded_size_ = 0;
	}
	int total_size() const {
		return total_size_;
	}

	void show_progress(int sz) {
		if (!total_size_ || !progress_fnc_) return;
		loaded_size_ += sz;
		unsigned percents = round(float(loaded_size_) / float(total_size_) * 100.0f);
		(*progress_fnc_)(percents, data_);
	}

	qdLoadingProgressInfo() {
		progress_fnc_ = 0;
		total_size_ = loaded_size_ = 0;
		data_ = 0;
	}
	~qdLoadingProgressInfo() { };
};

class qdGameDispatcherBase : public qdConditionalObject {
public:
	qdGameDispatcherBase();
	~qdGameDispatcherBase();

	void set_resources_size(int sz) {
		loading_progress_.set_total_size(sz);
	}

	qdLoadingProgressFnc set_loading_progress_callback(qdLoadingProgressFnc p, void *dp = 0) {
		return loading_progress_.set_callback_fnc(p, dp);
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
		return sounds.get_list();
	}
	const qdAnimationList &animation_list() const {
		return animations.get_list();
	}
	const qdAnimationSetList &animation_set_list() const {
		return animation_sets.get_list();
	}

	virtual void load_script_body(const xml::tag *p);
	virtual bool save_script_body(Common::SeekableWriteStream &fh, int indent = 0) const;

	virtual int load_resources();
	virtual void free_resources();

	virtual void quant(float dt) { };

	void add_scale_info(qdScaleInfo *p) {
		scale_infos.push_back(*p);
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

	qdObjectMapContainer<qdAnimation> animations;
	qdObjectMapContainer<qdAnimationSet> animation_sets;
	qdObjectMapContainer<qdSound> sounds;

	typedef std::vector<qdScaleInfo> scale_info_container_t;
	scale_info_container_t scale_infos;

	qdLoadingProgressInfo loading_progress_;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_GAME_DISPATCHER_BASE_H
