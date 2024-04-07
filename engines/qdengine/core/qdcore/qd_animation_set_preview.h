#ifndef __ANIMATION_SET_PREVIEW_H__
#define __ANIMATION_SET_PREVIEW_H__

#include "gr_dispatcher.h"

class qdCamera;

class qdAnimation;
class qdAnimationSet;

//! Превью походки.
class qdAnimationSetPreview
{
public:
	enum preview_mode_t {
		VIEW_WALK_ANIMATIONS,
		VIEW_STATIC_ANIMATIONS,
		VIEW_START_ANIMATIONS,
		VIEW_STOP_ANIMATIONS
	};

	qdAnimationSetPreview(qdAnimationSet* p);
	~qdAnimationSetPreview();

	preview_mode_t preview_mode() const { return preview_mode_; }
	void set_preview_mode(preview_mode_t md){ preview_mode_ = md; }

	void set_screen(Vect2s offs,Vect2s size);

	unsigned back_color() const { return back_color_; }
	void set_back_color(unsigned col){ back_color_ = col; }

	unsigned grid_color() const { return grid_color_; }
	void set_grid_color(unsigned col){ grid_color_ = col; }

	int camera_angle() const { return camera_angle_; }
	void set_camera_angle(int ang);

	float camera_focus();
	void set_camera_focus(float f);

	int cell_size() const { return cell_size_; }
	void set_cell_size(int sz);

	float personage_speed() const { return personage_speed_; }
	void set_personage_speed(float sp){ personage_speed_ = sp; }

	float personage_height() const { return personage_height_; }
	void set_personage_height(float h){ personage_height_ = h; }

	int direction() const { return direction_; }
	bool set_direction(int dir);

	bool set_phase(float phase);

	void set_graph_dispatcher(grDispatcher* p);

	void start();
	void quant();
	void quant(float tm);

	void redraw();

	const qdAnimation* cur_animation() const { return animation_; }

private:
	int start_time_;

	int direction_;
	float speed_;

	float personage_speed_;
	float personage_height_;

	qdAnimation* animation_;
	qdAnimationSet* animation_set_;

	qdCamera* camera_;
	int camera_angle_;

	grDispatcher* graph_d_;

	unsigned back_color_;
	unsigned grid_color_;

	int cell_size_;
	float cell_offset_;

	Vect2s screen_offset_;
	Vect2s screen_size_;

	preview_mode_t preview_mode_;

	void redraw_grid();
};

#endif /* __ANIMATION_SET_PREVIEW_H__ */
