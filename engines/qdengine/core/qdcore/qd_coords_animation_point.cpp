
/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "xml_tag_buffer.h"
#include "qdscr_parser.h"
#include "qd_coords_animation_point.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

const float qdCoordsAnimationPoint::NO_DIRECTION = -1.f;
qdCoordsAnimationPoint::qdCoordsAnimationPoint() : pos_(0,0,0),
direction_angle_(NO_DIRECTION)
{
	path_length_ = passed_path_length_ = 0.0f;
}

qdCoordsAnimationPoint::~qdCoordsAnimationPoint()
{
}

void qdCoordsAnimationPoint::load_script(const xml::tag* p)
{
	for(xml::tag::subtag_iterator it = p -> subtags_begin(); it != p -> subtags_end(); ++it){
		switch(it -> ID()){
			case QDSCR_DEST_POS:
				xml::tag_buffer(*it) > pos_.x > pos_.y > pos_.z;
				break;
			case QDSCR_OBJECT_DIRECTION:
				xml::tag_buffer(*it) > direction_angle_;
				break;
		}
	}
}

bool qdCoordsAnimationPoint::save_script(XStream& fh,int indent) const
{
	for(int i = 0; i < indent; i ++) fh < "\t";
	fh < "<coords_animation_point";

	fh < " dest_pos=\"" <= pos_.x < " " <= pos_.y < " " <= pos_.z < "\"";

	if(direction_angle_ >= 0.0f){
		fh < " object_direction=\"" <= direction_angle_ < "\"";
	}

	fh < "/>\r\n";

	return true;
}

void qdCoordsAnimationPoint::calc_path(const qdCoordsAnimationPoint& p, const Vect3f& shift) const
{
	Vect3f dr = pos_ - shift - p.dest_pos();
	path_length_ = dr.norm();
}																					

float qdCoordsAnimationPoint::passed_path() const
{
	if(path_length_ < 0.01f) return 1.0f;

	return passed_path_length_ / path_length_;
}

bool qdCoordsAnimationPoint::load_data(qdSaveStream& fh,int save_version)
{
	fh > path_length_ > passed_path_length_;

	return true;
}

bool qdCoordsAnimationPoint::save_data(qdSaveStream& fh) const
{
	fh < path_length_ < passed_path_length_;

	return true;
}
