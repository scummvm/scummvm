#ifndef __QD_ANIMATION_MAKER_H__
#define __QD_ANIMATION_MAKER_H__

typedef void (*maker_progress_fnc)(int percents_loaded,void* data);

class qdAnimationMaker
{
public:
	qdAnimationMaker();
	~qdAnimationMaker();

	bool insert_frame(class qdAnimation* p,const char* fname,int& insert_pos,bool insert_after = true,bool dupe_check = false);
	bool insert_frames(class qdAnimation* p,const char* folder,int& insert_pos,bool insert_after = true);

	maker_progress_fnc set_callback(maker_progress_fnc p,void* data = 0);

	void set_default_frame_length(float len){ default_frame_length_ = len; }

private:
	float default_frame_length_;	// длительность кадра по умолчанию (в секундах)

	void* callback_data_;
	maker_progress_fnc progress_callback_;
};

#endif /* __QD_ANIMATION_MAKER_H__ */
