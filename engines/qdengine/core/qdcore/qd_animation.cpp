/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "gr_dispatcher.h"
#include "gr_tile_animation.h"

#ifndef __QD_SYSLIB__
#include "qdscr_parser.h"
#include "xml_tag_buffer.h"

#include "qd_animation_info.h"
#include "qd_named_object_reference.h"

#include "qd_game_dispatcher.h"
#endif

#include "qd_animation.h"

#include "qd_file_manager.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimation::qdAnimation() : parent_(NULL)
{
	tileAnimation_ = 0;

	length_ = cur_time_ = 0.0f;
	status_ = QD_ANIMATION_STOPPED;

	sx_ = sy_ = 0;

	is_finished_ = false;

	playback_speed_ = 1.0f;

	frames_ptr = &frames;
	scaled_frames_ptr_ = &scaled_frames_;
}

qdAnimation::qdAnimation(const qdAnimation& anm) : qdNamedObject(anm), qdResource(anm),
	parent_(anm.parent_),
	length_(anm.length_),
	cur_time_(anm.cur_time_),
	status_(anm.status_),
	is_finished_(anm.is_finished_),
	sx_(anm.sx_),
	sy_(anm.sy_),
	num_frames_(anm.num_frames_),
	playback_speed_(1.0f),
	tileAnimation_(0)
{
	copy_frames(anm);

	if(anm.tileAnimation_)
		tileAnimation_ = new grTileAnimation(*anm.tileAnimation_);
}

qdAnimation::~qdAnimation()
{
	clear_frames();
	qda_file_.clear();

	delete tileAnimation_;
}

qdAnimation& qdAnimation::operator = (const qdAnimation& anm)
{
	if(this == &anm) return *this;

	*static_cast<qdNamedObject*>(this) = anm;
	*static_cast<qdResource*>(this) = anm;

	parent_ = anm.parent_;

	length_ = anm.length_;
	cur_time_ = anm.cur_time_;
	status_ = QD_ANIMATION_STOPPED;
	is_finished_ = false;

	playback_speed_ = anm.playback_speed_;

	sx_ = anm.sx_;
	sy_ = anm.sy_;

	copy_frames(anm);

	num_frames_ = anm.num_frames_;

	delete tileAnimation_;
	tileAnimation_ = 0;

	if(anm.tileAnimation_)
		tileAnimation_ = new grTileAnimation(*anm.tileAnimation_);

	return *this;
}

void qdAnimation::quant(float dt)
{
	if(status_ == QD_ANIMATION_PLAYING){
		if(need_stop()){
			stop();
			return;
		}

		cur_time_ += dt * playback_speed_;
		if(cur_time_ >= length()){
			if(length() > 0.01f){
				if(!check_flag(QD_ANIMATION_FLAG_LOOP)){
					cur_time_ = length() - 0.01f;
					status_ = QD_ANIMATION_END_PLAYING;
				}
				else {
					cur_time_ = fmodf(cur_time_,length());
				}
			}
			else
				cur_time_ = 0.0f;

			is_finished_ = true;
		}
	}
}

void qdAnimation::redraw(int x,int y,int z,int mode) const
{
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if(tileAnimation()){
		tileAnimation()->drawFrame(Vect2i(x,y), get_cur_frame_number(), mode);
	}
	else if(const qdAnimationFrame* p = get_cur_frame())
		p -> redraw(x,y,z,mode);
}

void qdAnimation::redraw(int x,int y,int z,float scale,int mode) const
{
	if(fabs(scale - 1.0f) < 0.01f){
		redraw(x,y,z,mode);
		return;
	}
	
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if(const qdAnimationFrame* p = get_cur_frame(scale))
		p -> redraw(x,y,z,scale,mode);
}

void qdAnimation::redraw_rot(int x,int y,int z,float angle,int mode) const
{
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(tileAnimation()){
		tileAnimation()->drawFrame(Vect2i(x,y), get_cur_frame_number(), angle, mode);
	}
	else if(const qdAnimationFrame* p = get_cur_frame())
		p -> redraw_rot(x,y,z,angle,mode);
}

void qdAnimation::redraw_rot(int x,int y,int z,float angle, const Vect2f& scale,int mode) const
{
	if(fabs(scale.x - 1.0f) < 0.01f && fabs(scale.y - 1.0f) < 0.01f){
		redraw_rot(x,y,z,angle,mode);
		return;
	}
	
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(const qdAnimationFrame* p = get_cur_frame())
		p -> redraw_rot(x,y,z,angle,scale,mode);
}

void qdAnimation::draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,int mode) const
{
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if(const qdAnimationFrame* p = get_cur_frame())
		p -> draw_mask(x,y,z,mask_color,mask_alpha,mode);
}

void qdAnimation::draw_mask(int x,int y,int z,unsigned mask_color,int mask_alpha,float scale,int mode) const
{
	if(fabs(scale - 1.0f) < 0.01f){
		draw_mask(x,y,z,mask_color,mask_alpha,mode);
		return;
	}
	
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	if(const qdAnimationFrame* p = get_cur_frame(scale))
		p -> draw_mask(x,y,z,mask_color,mask_alpha,scale,mode);
}

void qdAnimation::draw_mask_rot(int x,int y,int z,float angle,unsigned mask_color,int mask_alpha,int mode) const
{
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(tileAnimation()){
		tileAnimation()->drawFrame(Vect2i(x,y), get_cur_frame_number(), angle, mode);
	}
	else if(const qdAnimationFrame* p = get_cur_frame())
		p -> draw_mask_rot(x,y,z,angle,mask_color,mask_alpha,mode);
}

void qdAnimation::draw_mask_rot(int x,int y,int z,float angle,unsigned mask_color,int mask_alpha, const Vect2f& scale,int mode) const
{
	if(fabs(scale.x - 1.0f) < 0.01f && fabs(scale.y - 1.0f) < 0.01f){
		draw_mask_rot(x,y,z,angle,mask_color,mask_alpha,mode);
		return;
	}
	
	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(const qdAnimationFrame* p = get_cur_frame())
		p -> draw_mask_rot(x,y,z,angle,mask_color,mask_alpha,scale,mode);
}

void qdAnimation::draw_contour(int x,int y,unsigned color) const
{
	int mode = 0;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	const qdAnimationFrame* p = get_cur_frame();
	if(p) p -> draw_contour(x,y,color,mode);
}

void qdAnimation::draw_contour(int x,int y,unsigned color,float scale) const
{
	int mode = 0;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
		mode |= GR_FLIP_HORIZONTAL;

	if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
		mode |= GR_FLIP_VERTICAL;

	if(check_flag(QD_ANIMATION_FLAG_BLACK_FON))
		mode |= GR_BLACK_FON;

	const qdAnimationFrame* p = get_cur_frame();
	if(p) p -> draw_contour(x,y,color,scale,mode);
}

qdAnimationFrame* qdAnimation::get_cur_frame()
{
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
		if((*iaf) -> end_time() >= cur_time())
			return *iaf;
	}

	return NULL;
}

const qdAnimationFrame* qdAnimation::get_cur_frame() const
{
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
		if((*iaf) -> end_time() >= cur_time())
			return *iaf;
	}

	return NULL;
}

const qdAnimationFrame* qdAnimation::get_cur_frame(float& scale) const
{
	int index = get_scale_index(scale);

	if(index == -1)
		return get_cur_frame();

	return get_scaled_frame(get_cur_frame_number(),index);
}

bool qdAnimation::add_frame(qdAnimationFrame* p,qdAnimationFrame* insert_pos,bool insert_after)
{
	if(check_flag(QD_ANIMATION_FLAG_REFERENCE)) return false;

	if(insert_pos){
		qdAnimationFrameList::iterator iaf;
		FOR_EACH(frames,iaf){
			if(*iaf == insert_pos){
				if(insert_after)
					++iaf;
				frames.insert(iaf,p);
				num_frames_ = frames.size();
				return true;
			}
		}
	}
	else {
		if(insert_after)
			frames.push_back(p);
		else
			frames.insert(frames.end(),p);

		return true;
	}

	return false;
}

void qdAnimation::init_size()
{
	length_ = 0.0f;
	if(!tileAnimation()){
		sx_ = sy_ = 0;

		for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
			qdAnimationFrame* p = *iaf;

			p -> set_start_time(length_);

			if(p -> size_x() > sx_)
				sx_ = p -> size_x();

			if(p -> size_y() > sy_)
				sy_ = p -> size_y();

			length_ += p -> length();
		}
	}
	else {
		for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
			qdAnimationFrame* p = *iaf;

			p -> set_start_time(length_);
			p->set_size(tileAnimation()->frameSize());
			p->set_picture_offset(Vect2i(0,0));
			p->set_picture_size(tileAnimation()->frameSize());

			length_ += p -> length();
		}
	}

	if(cur_time_ >= length_)
		cur_time_ = length_ - 0.01f;

	num_frames_ = frames_ptr -> size();
}

void qdAnimation::load_script(const xml::tag* p)
{
#ifndef __QD_SYSLIB__
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_NAME:
				set_name(it -> data());
				break;
			case QDSCR_ANIMATION_FILE:
				qda_set_file((it) -> data());
				break;
			case QDSCR_FLAG:
				set_flag(xml::tag_buffer(*it).get_int());
				break;
		}
	}

	init_size();
#endif
}

bool qdAnimation::save_script(XStream& fh,int indent) const
{
#ifndef __QD_SYSLIB__
	for(int i = 0; i < indent; i ++) fh < "\t";

	fh < "<animation name=";

	if(name()) fh < "\"" < qdscr_XML_string(name()) < "\"";
	else fh < "\" \"";

	if(flags())
		fh < " flags=\"" <= flags() < "\"";

	if(qda_file())
		fh < " animation_file=\"" < qdscr_XML_string(qda_file()) < "\"";

	fh < "/>\r\n";
#endif
	return true;
}

bool qdAnimation::load_resources()
{
	if(check_flag(QD_ANIMATION_FLAG_REFERENCE)) return false;

	if(!qda_file()){
		qdAnimationFrameList::iterator iaf;
		for(iaf = frames.begin(); iaf != frames.end(); ++iaf){
			(*iaf) -> load_resources();
		}

		init_size();
		return true;
	}
	else
		return qda_load(qda_file());

	return false;
}

void qdAnimation::free_resources()
{
	toggle_resource_status(false); 
	if(check_flag(QD_ANIMATION_FLAG_REFERENCE)) return;

	for(qdAnimationFrameList::iterator iaf = frames.begin(); iaf != frames.end(); ++iaf)
		(*iaf) -> free_resources();

	for(qdAnimationFrameList::iterator iaf = scaled_frames_.begin(); iaf != scaled_frames_.end(); ++iaf)
		(*iaf) -> free_resources();
}

void qdAnimation::create_reference(qdAnimation* p,const qdAnimationInfo* inf) const
{
	p -> frames_ptr = &frames;
	p -> scaled_frames_ptr_ = &scaled_frames_;

	p -> clear_flags();
	p -> set_flag(flags() | QD_ANIMATION_FLAG_REFERENCE);

	p -> length_ = length_;
	p -> cur_time_ = 0.0f;

	p -> sx_ = sx_;
	p -> sy_ = sy_;

	p -> num_frames_ = num_frames_;

#ifndef __QD_SYSLIB__
	if(inf){
		if(inf -> check_flag(QD_ANIMATION_FLAG_LOOP))
			p -> set_flag(QD_ANIMATION_FLAG_LOOP);

		if(inf -> check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			p -> set_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL);

		if(inf -> check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			p -> set_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL);

		p -> playback_speed_ = inf -> animation_speed();
	}
#endif

	p -> parent_ = this;
}

bool qdAnimation::hit(int x,int y) const
{
	int xx = x;
	int yy = y;

	const qdAnimationFrame* p = get_cur_frame();
	if(p){
		if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			xx = -x;
		if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			yy = -y;

		return p -> hit(xx,yy);
	}

	return false;
}

bool qdAnimation::hit(int x,int y,float scale) const
{
	int xx = x;
	int yy = y;

	const qdAnimationFrame* p = get_cur_frame();
	if(p){
		if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			xx = -x;
		if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			yy = -y;

		return p -> hit(xx,yy,scale);
	}

	return false;
}

void qdAnimation::qda_save(const char* fname)
{
	XStream fh(fname,XS_OUT);

	int num_frames = frames.size();
	int num_scales = scales_.size();
	fh < qda_version < sx_ < sy_ < length_ < flags() < num_frames < num_scales;

	if(!tileAnimation_){
		fh < char(0);

		for(int i = 0; i < num_scales; i++)
			fh < scales_[i];

		qdAnimationFrameList::iterator it;
		FOR_EACH(frames,it)
			(*it) -> qda_save(fh);

		assert(scaled_frames_.size() == num_scales * frames.size());

		FOR_EACH(scaled_frames_,it)
			(*it) -> qda_save(fh);
	}
	else {
		fh < char(1) < sx_ < sy_;

		qdAnimationFrameList::iterator it;
		FOR_EACH(frames,it)
			fh < (*it) -> start_time() < (*it) -> length();

		tileAnimation_->save(fh);
	}
}

bool qdAnimation::qda_load(const char* fname)
{
	clear_frames();

#ifndef __QD_SYSLIB__
	XZipStream fh;
	if(!qdFileManager::instance().open_file(fh, fname)) return false;
#else
	XStream fh;
	fh.open(fname,XS_IN);
#endif

	int i,version,fl,num_fr;
	fh > version > sx_ > sy_ > length_ > fl > num_fr;

	int num_scales = 0;
	if(version >= 103)
		fh > num_scales;

	char tile_flag = 0;
	if(version >= 104)
		fh > tile_flag;

	if(!tile_flag){
		if(num_scales){
			scales_.resize(num_scales);
			for(int i = 0; i < num_scales; i++)
				fh > scales_[i];
		}
		else
			scales_.clear();

		set_flag(fl & (QD_ANIMATION_FLAG_CROP | QD_ANIMATION_FLAG_COMPRESS));

		for(i = 0; i < num_fr; i ++){
			qdAnimationFrame* p = new qdAnimationFrame;
			p -> qda_load(fh,version);
			add_frame(p);
		}

		for(i = 0; i < num_fr * num_scales; i ++){
			qdAnimationFrame* p = new qdAnimationFrame;
			p -> qda_load(fh,version);
			scaled_frames_.push_back(p);
		}
	}
	else {
		set_flag(fl);

		fh > sx_ > sy_;

		for(i = 0; i < num_fr; i ++){
			float start_time, length;
			fh > start_time > length;

			qdAnimationFrame* p = new qdAnimationFrame;
			p->set_start_time(start_time);
			p->set_length(length);
			add_frame(p);
		}

		tileAnimation_ = new grTileAnimation;
		tileAnimation_->load(fh);
	}

	init_size();

	return true;
}

bool qdAnimation::qda_load_header(const char* fname)
{
#ifndef __QD_SYSLIB__
	XZipStream fh;
	if(!qdFileManager::instance().open_file(fh, fname)) return false;
#else
	XStream fh;
	fh.open(fname,XS_IN);
#endif

	int fl,version;
	fh > version > sx_ > sy_ > length_ > fl > num_frames_;
	clear_flags();
	set_flag(fl);

	return true;
}

void qdAnimation::qda_set_file(const char* fname)
{
	if(fname)
		qda_file_ = fname;
	else
		qda_file_.clear();
}

bool qdAnimation::crop()
{
	for(qdAnimationFrameList::iterator it = frames.begin(); it != frames.end(); ++it)
		(*it) -> crop();
	for(qdAnimationFrameList::iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it)
		(*it) -> crop();

	return true;
}

bool qdAnimation::undo_crop()
{
	for(qdAnimationFrameList::iterator it = frames.begin(); it != frames.end(); ++it)
		(*it) -> undo_crop();
	for(qdAnimationFrameList::iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it)
		(*it) -> undo_crop();

	return true;
}

bool qdAnimation::compress()
{
	if(check_flag(QD_ANIMATION_FLAG_COMPRESS)) return false;

	bool result = true;
	for(qdAnimationFrameList::iterator it = frames.begin(); it != frames.end(); ++it){
		if(!(*it) -> compress()) result = false;
	}
	for(qdAnimationFrameList::iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it){
		if(!(*it) -> compress()) result = false;
	}

	set_flag(QD_ANIMATION_FLAG_COMPRESS);
	return result;
}

bool qdAnimation::tileCompress(grTileCompressionMethod method, int tolerance)
{
	if(!num_frames_ || check_flag(QD_ANIMATION_FLAG_TILE_COMPRESS)) return false;

	uncompress();
	undo_crop();

	grTileSprite::setComprasionTolerance(tolerance);

	tileAnimation_ = new grTileAnimation;
	tileAnimation_->init(num_frames_, Vect2i(sx_, sy_), frames.front()->check_flag(qdSprite::ALPHA_FLAG));

	for(int i = 0; i < num_frames_; i++)
		tileAnimation_->addFrame((const unsigned*)get_frame(i)->data());

	if(method != TILE_UNCOMPRESSED)
		tileAnimation_->compress(method);

	tileAnimation_->compact();

	set_flag(QD_ANIMATION_FLAG_TILE_COMPRESS);

	return true;
}

bool qdAnimation::uncompress()
{
	if(!check_flag(QD_ANIMATION_FLAG_COMPRESS) || check_flag(QD_ANIMATION_FLAG_TILE_COMPRESS)) return false;

	bool result = true;
	qdAnimationFrameList::iterator iaf;
	for(qdAnimationFrameList::iterator it = frames.begin(); it != frames.end(); ++it){
		if(!(*it) -> uncompress()) result = false;
	}
	for(qdAnimationFrameList::iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it){
		if(!(*it) -> uncompress()) result = false;
	}

	drop_flag(QD_ANIMATION_FLAG_COMPRESS);
	return result;
}

int qdAnimation::get_cur_frame_number() const
{
	int num = 0;
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
		if((*iaf) -> end_time() >= cur_time()){
			return num;
		}
		num ++;
	}

	return -1;
}

void qdAnimation::set_cur_frame(int number)
{
	int num = 0;
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
		if(num ++ == number){
			set_time((*iaf) -> start_time() + (*iaf) -> length()/2.0f);
			return;
		}
	}
}

bool qdAnimation::remove_frame(int number)
{
	int num = 0;
	for(qdAnimationFrameList::iterator iaf = frames.begin(); iaf != frames.end(); ++iaf){
		if(num ++ == number){
			delete *iaf;
			frames.erase(iaf);
			init_size();
			return true;
		}
	}

	return false;
}

bool qdAnimation::remove_frame_range(int number0,int number1)
{
	int num = 0;
	qdAnimationFrameList::iterator iaf,iaf0,iaf1;

	iaf0 = iaf1 = frames.end();
	for(iaf = frames.begin(); iaf != frames.end(); ++iaf){
		if(num == number0)
			iaf0 = iaf;
		if(num == number1){
			iaf1 = iaf;
			break;
		}
		num ++;
	}

	if(iaf0 != frames.end() && iaf1 != frames.end()){
		for(iaf = iaf0; iaf != iaf1; ++iaf)
			delete *iaf;

		frames.erase(iaf0,iaf1);
		init_size();

		return true;
	}

	return false;
}

bool qdAnimation::reverse_frame_range(int number0,int number1)
{
	int num = 0;
	qdAnimationFrameList::iterator iaf0 = frames.end();
	qdAnimationFrameList::iterator iaf1 = frames.end();

	for(qdAnimationFrameList::iterator iaf = frames.begin(); iaf != frames.end(); ++iaf){
		if(num == number0)
			iaf0 = iaf;
		if(num == number1){
			iaf1 = iaf;
			break;
		}
		num ++;
	}

	if(iaf0 != frames.end() && iaf1 != frames.end()){
		iaf1++;

		std::reverse(iaf0,iaf1);
		init_size();
		return true;
	}

	return false;
}

qdAnimationFrame* qdAnimation::get_frame(int number)
{
	int num = 0;
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf){
		if(num == number)
			return *iaf;

		num ++;
	}

	return 0;
}

bool qdAnimation::load_resource()
{ 
	toggle_resource_status();
	return load_resources(); 
}

bool qdAnimation::load_resource_header(const char* fname)
{ 
	if(is_resource_loaded()) return load_resource();

	const char* p = (fname) ? fname : qda_file();
	return qda_load_header(p); 
}

bool qdAnimation::free_resource()
{ 
	toggle_resource_status(false); 

	free_resources(); 
	return true; 
}

void qdAnimation::advance_time(float tm)
{
	if(length_ <= 0.01f) return;

	tm *= playback_speed_;

	if(cur_time_ + tm >= length()){
		if(check_flag(QD_ANIMATION_FLAG_LOOP)){
			tm -= length() - cur_time_;
			while(tm >= length())
				tm -= length();

			cur_time_ = tm;
		}
		else
			cur_time_ = length() - 0.01f;
	}
	else
		cur_time_ += tm;
}

int qdAnimation::picture_size_x() const
{
	int i = 0;
	int sx = 0;
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf, i++){
		sx += (*iaf) -> picture_size_x();
	}

	if(i) return sx / i;
	return 0;
}

int qdAnimation::picture_size_y() const
{
	int i = 0;
	int sy = 0;
	for(qdAnimationFrameList::const_iterator iaf = frames_ptr -> begin(); iaf != frames_ptr -> end(); ++iaf, i++){
		sy += (*iaf) -> picture_size_y();
	}

	if(i) return sy / i;
	return 0;
}

bool qdAnimation::scale(float coeff_x,float coeff_y)
{
	bool res = true;

	qdAnimationFrameList::iterator iaf;
	for(iaf = frames.begin(); iaf != frames.end(); ++iaf){
		if(!(*iaf) -> scale(coeff_x,coeff_y)) res = false;
	}

	init_size();

	return res;
}

Vect2i qdAnimation::remove_edges()
{
	if(frames.empty()) return Vect2i(0,0);

	bool crop_flag = false;
	bool compress_flag = false;

	if(check_flag(QD_ANIMATION_FLAG_COMPRESS)){
		uncompress();
		compress_flag = true;
	}

	if(check_flag(QD_ANIMATION_FLAG_CROP)){
		undo_crop();
		crop_flag = true;
	}

	int left,top,right,bottom;
	if(!(*frames.begin()) -> get_edges_width(left,top,right,bottom)) return Vect2i(0,0);

	qdAnimationFrameList::iterator iaf;
	for(iaf = frames.begin(); iaf != frames.end(); ++iaf){
		int l,t,r,b;
		if((*iaf) -> get_edges_width(l,t,r,b)){
			if(l < left) left = l;
			if(t < top) top = t;
			if(r < right) right = r;
			if(b < bottom) bottom = b;
		}
		else
			return Vect2i(0,0);
	}

	for(iaf = frames.begin(); iaf != frames.end(); ++iaf){
		if(!(*iaf) -> crop(left,top,right,bottom,false)) return Vect2i(0,0);
	}

	sx_ -= left + right;
	sy_ -= top + bottom;

	if(crop_flag)
		crop();

	if(compress_flag)
		compress();

	return Vect2i(left,top);
}

bool qdAnimation::load_data(qdSaveStream& fh,int save_version)
{
#ifndef __QD_SYSLIB__
	if(!qdNamedObject::load_data(fh,save_version)) return false;

	char fl;
	fh > fl;

	if(fl){
		qdNamedObjectReference ref;

		if(!ref.load_data(fh,save_version))
			return false;

		if(qdGameDispatcher* p = qd_get_game_dispatcher()){
			if(qdNamedObject* obj = p -> get_named_object(&ref)){
				if(obj -> named_object_type() != QD_NAMED_OBJECT_ANIMATION) return false;
				int fl = flags();

				static_cast<qdAnimation*>(obj) -> create_reference(this);

				clear_flags();
				set_flag(fl);
			}
		}
	}
	else
		clear();

	char st,finished;
	fh > st > finished > cur_time_ > length_;

	status_ = st;
	is_finished_ = (finished) ? true : false;
#endif
	return true;
}

bool qdAnimation::save_data(qdSaveStream& fh) const
{
#ifndef __QD_SYSLIB__
	if(!qdNamedObject::save_data(fh)) return false;

	if(check_flag(QD_ANIMATION_FLAG_REFERENCE) && parent_){
		fh < (char)1;
		qdNamedObjectReference ref(parent_);

		if(!ref.save_data(fh))
			return false;
	}
	else
		fh < (char)0;

	fh < char(status_) < char(is_finished_) < cur_time_ < length_;
#endif
	return true;
}

grScreenRegion qdAnimation::screen_region(int mode,float scale) const
{
	if(const qdAnimationFrame* p = get_cur_frame()){
		if(check_flag(QD_ANIMATION_FLAG_FLIP_HORIZONTAL))
			mode |= GR_FLIP_HORIZONTAL;

		if(check_flag(QD_ANIMATION_FLAG_FLIP_VERTICAL))
			mode |= GR_FLIP_VERTICAL;

		return p -> screen_region(mode,scale);
	}
	else
		return grScreenRegion::EMPTY;
}

bool qdAnimation::copy_frames(const qdAnimation& anm)
{
	if(!check_flag(QD_ANIMATION_FLAG_REFERENCE)){
		clear_frames();

		frames_ptr = &frames;

		qdAnimationFrameList::const_iterator it;
		FOR_EACH(anm.frames,it)
			frames.push_back((*it) -> clone());

		scaled_frames_ptr_ = &scaled_frames_;

		FOR_EACH(anm.scaled_frames_,it)
			scaled_frames_.push_back((*it) -> clone());
	}
	else {
		frames_ptr = anm.frames_ptr;
		scaled_frames_ptr_ = anm.scaled_frames_ptr_;
	}

	return true;
}

void qdAnimation::clear_frames()
{
	for(qdAnimationFrameList::iterator it = frames.begin(); it != frames.end(); ++it)
		delete *it;
	for(qdAnimationFrameList::iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it)
		delete *it;

	frames.clear();
	scaled_frames_.clear();
}

bool qdAnimation::add_scale(float value)
{
	if(fabs(value - 1.0f) <= 0.01f || value <= 0.01f) return false;

	std::vector<float>::const_iterator it = std::find(scales_.begin(),scales_.end(),value);
	if(it != scales_.end()) return false;

	scales_.push_back(value);
	std::sort(scales_.begin(),scales_.end());
	return true;
}

bool qdAnimation::create_scaled_frames()
{
	if(check_flag(QD_ANIMATION_FLAG_REFERENCE)) return false;

	for(qdAnimationFrameList::iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it)
		delete *it;
	scaled_frames_.clear();

	for(int i = 0; i < scales_.size(); i++){
		for(qdAnimationFrameList::iterator it = frames.begin(); it != frames.end(); ++it){
			scaled_frames_.push_back((*it) -> clone());
			scaled_frames_.back() -> scale(scales_[i],scales_[i]);
		}
	}

	return true;
}

int qdAnimation::get_scale_index(float& scale_value) const
{
	int index = -1;
	float scl = 1.0f;

	const std::vector<float>& scales_vect = (check_flag(QD_ANIMATION_FLAG_REFERENCE) && parent_) ? parent_ -> scales_ : scales_;

	for(int i = 0; i < scales_vect.size(); i++){
		if(fabs(scale_value - scl) > fabs(scale_value - scales_vect[i])){
			scl = scales_vect[i];
			index = i;
		}
	}

	if(index != -1)
		scale_value /= scl;

	return index;
}

const qdAnimationFrame* qdAnimation::get_scaled_frame(int number,int scale_index) const
{
	int num = 0;
	number += scale_index * num_frames_;
	for(qdAnimationFrameList::const_iterator it = scaled_frames_ptr_ -> begin(); it != scaled_frames_ptr_ -> end(); ++it){
		if(num++ == number)
			return *it;
	}

	return NULL;
}

#ifdef __QD_DEBUG_ENABLE__
unsigned qdAnimation::resource_data_size() const
{
	unsigned size = 0;

	for(qdAnimationFrameList::const_iterator it = frames.begin(); it != frames.end(); ++it)
		size += (*it) -> resource_data_size();

	for(qdAnimationFrameList::const_iterator it = scaled_frames_.begin(); it != scaled_frames_.end(); ++it)
		size += (*it) -> resource_data_size();

	return size;
}
#endif
