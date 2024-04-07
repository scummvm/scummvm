/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_animation.h"
#include "qd_animation_maker.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdAnimationMaker::qdAnimationMaker() : default_frame_length_(0.05f),
	callback_data_(0),
	progress_callback_(0)
{
}

qdAnimationMaker::~qdAnimationMaker()
{
}

maker_progress_fnc qdAnimationMaker::set_callback(maker_progress_fnc p,void* data)
{
	maker_progress_fnc old_p = progress_callback_;
	progress_callback_ = p;
	callback_data_ = data;

	return old_p;
}

bool qdAnimationMaker::insert_frame(class qdAnimation* p,const char* fname,int& insert_pos,bool insert_after,bool dupe_check)
{
	// IMPORTANT(pabdulin): auto_ptr usage was removed
	qdAnimationFrame* fp = new qdAnimationFrame;
	fp -> set_file(fname);
	fp -> set_length(default_frame_length_);

	if(!fp -> load_resources())
		return false;

	if(p -> check_flag(QD_ANIMATION_FLAG_CROP))
		fp -> crop();

	if(p -> check_flag(QD_ANIMATION_FLAG_COMPRESS))
		fp -> compress();
	
	qdAnimationFrame* ins_p = p -> get_frame(insert_pos);
	if(dupe_check && ins_p && *ins_p == *fp){
		ins_p -> set_length(ins_p -> length() + fp -> length());
		return true;
	}
	
	if(!p -> add_frame(fp,ins_p,insert_after))
		return false;
	
	if(insert_after)
		insert_pos++;
	
	p -> init_size();
	
	//fp->release();
	delete fp;
	return true;
}

bool qdAnimationMaker::insert_frames(class qdAnimation* p,const char* folder,int& insert_pos,bool insert_after)
{
	bool result = false;
	
	char old_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,old_path);
	SetCurrentDirectory(folder);
	
	qdFileNameList flist;

	char* fname = XFindFirst("*.tga");
	while(fname){
		flist.push_back(fname);
		fname = XFindNext();
	}

	flist.sort();

	if(!flist.empty()){
		int i = 0;
		qdFileNameList::iterator it;
		FOR_EACH(flist,it){
			if(insert_frame(p,it -> c_str(),insert_pos,insert_after,true))
				result = true;

			if(progress_callback_){
				int percents = i++ * 100 / flist.size();
				(*progress_callback_)(percents,callback_data_);
			}
		}
		flist.clear();
	}

	SetCurrentDirectory(old_path);
	return result;
}
