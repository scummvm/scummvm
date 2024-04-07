/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_camera.h"
#include "qd_animation_set.h"
#include "qd_animation_set_preview.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationSetPreview::qdAnimationSetPreview(qdAnimationSet* p) : 
	graph_d_(0), 
	animation_set_(p),
	preview_mode_(VIEW_WALK_ANIMATIONS)
{
	animation_ = new qdAnimation;

	camera_ = new qdCamera;
	camera_ -> set_focus(2000.0f);
	camera_angle_ = 45;

	start_time_ = 0;
	back_color_ = RGB(0,0,0);
	grid_color_ = RGB(128,128,128);

	set_direction(0);

	cell_size_ = 100;

	personage_height_ = float(animation_ -> size_y());

	p -> load_animations(NULL);
}

qdAnimationSetPreview::~qdAnimationSetPreview()
{
	animation_set_ -> free_animations(NULL);

	delete animation_;
	delete camera_;
}

void qdAnimationSetPreview::set_graph_dispatcher(grDispatcher* p)
{ 
	graph_d_ = p;
	set_screen(Vect2s(0,0),Vect2s(p -> Get_SizeX(),p -> Get_SizeY()));
}

void qdAnimationSetPreview::start()
{
	start_time_ = xclock();

	animation_ -> start();
	cell_offset_ = 0.0f;
}

void qdAnimationSetPreview::quant()
{
	int time = xclock();
	float tm = float(time - start_time_) / 1000.0f;
	start_time_ = time;

	if(tm >= 0.3f) return;

	quant(tm);
}

void qdAnimationSetPreview::quant(float tm)
{
	animation_ -> quant(tm);

	cell_offset_ -= personage_speed_ * tm;
	while(cell_offset_ <= -float(cell_size_)) cell_offset_ += float(cell_size_);
}

void qdAnimationSetPreview::redraw()
{
	grDispatcher* gp = grDispatcher::set_instance(graph_d_);

	grDispatcher::instance() -> Fill(back_color_);

	redraw_grid();

	Vect2s v = camera_ -> global2scr(Vect3f(0.0f,0.0f,personage_height_/2.0f));
	float scale = camera_ -> get_scale(Vect3f(0.0f,0.0f,personage_height_/2.0f));
	animation_ -> redraw(v.x,v.y,scale);

	Vect2s v0 = camera_ -> global2scr(Vect3f(0.0f,0.0f,personage_height_));
	Vect2s v1 = camera_ -> global2scr(Vect3f(0.0f,0.0f,0.0f));

	const int rect_sz = 4;
	grDispatcher::instance() -> Rectangle(v.x - rect_sz/2,v.y - rect_sz/2,rect_sz,rect_sz,grid_color_,grid_color_,GR_FILLED);

	const int line_sz = 10;
	grDispatcher::instance() -> Line(v0.x - line_sz,v0.y,v0.x + line_sz,v0.y,grid_color_);
	grDispatcher::instance() -> Line(v1.x - line_sz,v1.y,v1.x + line_sz,v1.y,grid_color_);
	grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,grid_color_);
	grDispatcher::instance() -> Rectangle(screen_offset_.x,screen_offset_.y,screen_size_.x,screen_size_.y,grid_color_,0,GR_OUTLINED);
	grDispatcher::instance() -> Flush();

	grDispatcher::set_instance(gp);
}

bool qdAnimationSetPreview::set_direction(int dir)
{
	bool result = false;

	animation_ -> clear();
	personage_speed_ = 0.0f;
	direction_ = dir;
	cell_offset_ = 0.0f;

	float angle = animation_set_ -> get_index_angle(direction_) * 180.0f / M_PI;
	camera_ -> rotate_and_scale(camera_angle_,0,angle,1.0f,1.0f,1.0f);

	qdAnimationInfo* p = NULL;
	if(preview_mode_ == VIEW_WALK_ANIMATIONS)
		p = animation_set_ -> get_animation_info(dir);
	else if(preview_mode_ == VIEW_STATIC_ANIMATIONS)
		p = animation_set_ -> get_static_animation_info(dir);
	else if(preview_mode_ == VIEW_START_ANIMATIONS)
		p = animation_set_ -> get_start_animation_info(dir);
	else if(preview_mode_ == VIEW_STOP_ANIMATIONS)
		p = animation_set_ -> get_stop_animation_info(dir);

	if(p){
		qdAnimation* ap = p -> animation();
		if(ap){
			ap -> create_reference(animation_,p);
			result = true;
		}
		personage_speed_ = p -> speed();
	}

	animation_ -> start();

	return result;
}

void qdAnimationSetPreview::set_cell_size(int sz)
{
	cell_size_ = sz;

	animation_ -> start();
	cell_offset_ = 0.0f;
}

void qdAnimationSetPreview::set_screen(Vect2s offs,Vect2s size)
{
	if(!graph_d_) return;

	screen_offset_ = offs;
	screen_size_ = size;

	camera_ -> set_scr_size(size.x,size.y);
	camera_ -> set_scr_center(offs.x + size.x/2,offs.y + size.y*3/4);

	graph_d_ -> SetClip(offs.x,offs.y,offs.x + size.x,offs.y + size.y);
}

void qdAnimationSetPreview::redraw_grid()
{
	float size = 0;
	Vect2f p = camera_ -> scr2plane(screen_offset_);
	if(fabs(p.x) > size) size = fabs(p.x);
	if(fabs(p.y) > size) size = fabs(p.y);
	p = camera_ -> scr2plane(screen_offset_ + screen_size_);
	if(fabs(p.x) > size) size = fabs(p.x);
	if(fabs(p.y) > size) size = fabs(p.y);
	p = camera_ -> scr2plane(Vect2s(screen_offset_.x + screen_size_.x,screen_offset_.y));
	if(fabs(p.x) > size) size = fabs(p.x);
	if(fabs(p.y) > size) size = fabs(p.y);
	p = camera_ -> scr2plane(Vect2s(screen_offset_.x,screen_offset_.y + screen_size_.y));
	if(fabs(p.x) > size) size = fabs(p.x);
	if(fabs(p.y) > size) size = fabs(p.y);

	int sz = round(size) + cell_size_;
	sz -= sz % cell_size_;
	for(int i = -sz; i <= sz; i += cell_size_){
		int dx = round(cell_offset_);

		Vect3f v00 = camera_ -> global2camera_coord(Vect3f(i + dx,-sz,0));
		Vect3f v10 = camera_ -> global2camera_coord(Vect3f(i + dx,sz,0));
		if(camera_ -> line_cutting(v00,v10)){
			Vect2s v0 = camera_ -> camera_coord2scr(v00);
			Vect2s v1 = camera_ -> camera_coord2scr(v10);
			grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,grid_color_,0);
		}

		v00 = camera_ -> global2camera_coord(Vect3f(-sz + dx,i,0));
		v10 = camera_ -> global2camera_coord(Vect3f(sz + dx,i,0));
		if(camera_ -> line_cutting(v00,v10)){
			Vect2s v0 = camera_ -> camera_coord2scr(v00);
			Vect2s v1 = camera_ -> camera_coord2scr(v10);
			grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,grid_color_,0);
		}
/*
		v0 = camera_ -> global2scr(Vect3f(-sz + dx,i,0));
		v1 = camera_ -> global2scr(Vect3f(sz + dx,i,0));
		grDispatcher::instance() -> Line(v0.x,v0.y,v1.x,v1.y,grid_color_,0);
*/
	}
}

void qdAnimationSetPreview::set_camera_angle(int ang)
{
	camera_angle_ = ang;

	float angle = animation_set_ -> get_index_angle(direction_) * 180.0f / M_PI;
	camera_ -> rotate_and_scale(camera_angle_,0,angle,1.0f,1.0f,1.0f);
}

float qdAnimationSetPreview::camera_focus()
{
	return camera_ -> get_focus();
}

void qdAnimationSetPreview::set_camera_focus(float f)
{
	camera_ -> set_focus(f);
}

bool qdAnimationSetPreview::set_phase(float phase)
{
	if(!animation_ -> is_empty()){
		if(!animation_ -> is_playing())
			animation_ -> start();

		cell_offset_ = 0.0f;
		animation_ -> set_time(0.0f);
		quant(animation_ -> length() * phase);
		return true;
	}

	return false;
}
