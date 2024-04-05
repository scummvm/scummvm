/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <algorithm>

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"
#include "qd_contour.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdContour::qdContour(qdContourType tp) : contour_type_(tp),
	size_(0,0),
	mask_pos_(0,0)
{
}

qdContour::qdContour(const qdContour& ct) : contour_type_(ct.contour_type_),
	size_(ct.size_),
#ifdef _QUEST_EDITOR
	contour_updated_(ct.contour_updated_),
#endif
	contour_(ct.contour_)
{
	mask_pos_ = ct.mask_pos_;
}

qdContour::~qdContour()
{
	contour_.clear();
#ifdef _QUEST_EDITOR
	contour_updated_.clear();
#endif
}

qdContour& qdContour::operator = (const qdContour& ct)
{
	if(this == &ct) return *this;

	contour_type_ = ct.contour_type_;
	size_ = ct.size_;
	mask_pos_ = ct.mask_pos_;

	contour_ = ct.contour_;

#ifdef _QUEST_EDITOR
	contour_updated_ = ct.contour_updated_;
#endif

	return *this;
}

#ifdef _QUEST_EDITOR
void qdContour::set_contour(std::vector<Vect2s> const& contour)
{
	clear_contour();
	contour_ = contour;
}
#endif // _QUEST_EDITOR

void qdContour::add_contour_point(const Vect2s& pt)
{
	contour_.push_back(pt);
}

void qdContour::insert_contour_point(const Vect2s& pt,int insert_pos)
{
	if(insert_pos < contour_.size()){
		if(insert_pos < 0) insert_pos = 0;
		contour_.insert(contour_.begin() + insert_pos,pt);
	}
	else
		contour_.push_back(pt);
}

bool qdContour::remove_contour_point(int pos)
{
	if(pos >= 0 && pos < contour_.size()){
		contour_.erase(contour_.begin() + pos);
		return true;
	}
	return false;
}

bool qdContour::update_contour_point(const Vect2s& pt,int pos)
{
	if(pos >= 0 && pos < contour_.size()){
		contour_[pos] = pt;
		return true;
	}
	return false;
}

bool qdContour::update_contour()
{
	if(contour_type_ != CONTOUR_POLYGON) return false;

#ifdef _QUEST_EDITOR
	contour_updated_ = contour_;
#endif

	if(contour_.empty())
		return false;

	int x0 = contour_[0].x;
	int x1 = contour_[0].x;
	int y0 = contour_[0].y;
	int y1 = contour_[0].y;
	
	for(int i = 0; i < contour_.size(); i++){
		if(contour_[i].x < x0) x0 = contour_[i].x;
		if(contour_[i].x > x1) x1 = contour_[i].x;
		if(contour_[i].y < y0) y0 = contour_[i].y;
		if(contour_[i].y > y1) y1 = contour_[i].y;
	}

	size_ = Vect2s(x1 - x0 + 1, y1 - y0 + 1);
	mask_pos_ = Vect2s(x0 + size_.x/2, y0 + size_.y/2);

	return true;
}

bool qdContour::is_inside(const Vect2s& pos) const
{
	switch(contour_type_){
	case CONTOUR_RECTANGLE:
		if(pos.x >= -size_.x/2 && pos.x < size_.x/2 && pos.y >= -size_.y/2 && pos.y < size_.y/2)
			return true;
		break;
	case CONTOUR_CIRCLE:
		if(pos.x * pos.x + pos.y * pos.y <= size_.x * size_.x)
			return true;
		break;
	case CONTOUR_POLYGON: {
			Vect2s p = pos;
			int intersections_lt0 = 0;
			int intersections_gt0 = 0;
			int intersections_lt1 = 0;
			int intersections_gt1 = 0;
#ifdef _QUEST_EDITOR
			for(int i = 0; i < contour_updated_.size(); i ++){
				Vect2s p0 = contour_updated_[i];
				Vect2s p1 = (i < contour_updated_.size() - 1) ? contour_updated_[i + 1] : contour_updated_[0];
#else
			for(int i = 0; i < contour_.size(); i ++){
				Vect2s p0 = contour_[i];
				Vect2s p1 = (i < contour_.size() - 1) ? contour_[i + 1] : contour_[0];
#endif
				if(p0.y != p1.y){
					if((p0.y < p.y && p1.y >= p.y) || (p0.y >= p.y && p1.y < p.y)){
						if(p0.x < p.x && p1.x < p.x)
							intersections_lt0++;
						else if(p0.x > p.x && p1.x > p.x)
							intersections_gt0++;
						else {
							int x = (p.y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y) + p0.x;

							if(x == p.x)
								return true;
							else if(x > p.x)
								intersections_gt0++;
							else
								intersections_lt0++;
						}
					}
					if((p0.y <= p.y && p1.y > p.y) || (p0.y > p.y && p1.y <= p.y)){
						if(p0.x < p.x && p1.x < p.x)
							intersections_lt1++;
						else if(p0.x > p.x && p1.x > p.x)
							intersections_gt1++;
						else {
							int x = (p.y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y) + p0.x;

							if(x == p.x)
								return true;
							else if(x > p.x)
								intersections_gt1++;
							else
								intersections_lt1++;
						}
					}
				}
			}

			return ((intersections_lt0 & 1) && intersections_gt0 != 0) ||
				((intersections_lt1 & 1) && intersections_gt1 != 0);
			break;
		}
	}
	return false;
}

bool qdContour::save_script(XStream& fh,int indent) const
{
	if(contour_type_ == CONTOUR_POLYGON){
		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "<contour_polygon>" <= contour_size() * 2;

		for(int j = 0; j < contour_size(); j ++){
			fh < " " <= contour_[j].x < " " <= contour_[j].y;
		}
		fh < "</contour_polygon>\r\n";
		return true;
	}

	if(contour_type_ == CONTOUR_RECTANGLE){
		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "<contour_rect>" <= size_.x < " " <= size_.y < "</contour_rect>\r\n";
		return true;
	}

	if(contour_type_ == CONTOUR_CIRCLE){
		for(int i = 0; i < indent; i ++) fh < "\t";
		fh < "<contour_circle>" <= size_.x < "</contour_circle>\r\n";
		return true;
	}

	return false;
}

bool qdContour::load_script(const xml::tag* p)
{
	int i;

	Vect2s v;
	xml::tag_buffer buf(*p);

	switch(contour_type_){
		case CONTOUR_RECTANGLE:
			buf > v.x > v.y;
			size_ = Vect2s(v);
			return true;
		case CONTOUR_CIRCLE:
			size_.x = size_.y = buf.get_short();
			return true;
		case CONTOUR_POLYGON: 
			contour_.reserve(p -> data_size() / 2);
			for(i = 0; i < p -> data_size(); i += 2){
				buf > v.x > v.y;
				add_contour_point(v);
			}
			update_contour();
			return true;
	}

	return false;
}

void qdContour::divide_contour(int shift)
{
	std::vector<Vect2s>::iterator _itr = contour_.begin(), _end = contour_.end();
	for(;_itr != _end; ++_itr)
	{
		Vect2s& v = *_itr;
		v.x >>= shift;
		v.y >>= shift;
	}
}

void qdContour::mult_contour(int shift)
{
	std::vector<Vect2s>::iterator _itr = contour_.begin(), _end = contour_.end();
	for(;_itr != _end; ++_itr)
	{
		Vect2s& v = *_itr;
		v.x <<= shift;
		v.y <<= shift;
	}
}

void qdContour::shift_contour(int dx,int dy)
{
	std::vector<Vect2s>::iterator _itr = contour_.begin(), _end = contour_.end();
	for(;_itr != _end; ++_itr)
	{
		Vect2s& v = *_itr;
		v.x += dx;
		v.y += dy;
	}
}

bool qdContour::can_be_closed() const
{
	if(contour_type_ != CONTOUR_POLYGON) 
		return false;

	return (contour_.size()>2); 
}

bool qdContour::is_contour_empty() const
{
	if(contour_type_ == CONTOUR_POLYGON)
		return (contour_.size() < 3);

	return false;
}
