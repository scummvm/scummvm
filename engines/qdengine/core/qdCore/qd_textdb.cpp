/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_textdb.h"
//#include <tchar.h>

char const DELIMETER = '.';

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdTextDB::qdTextDB()
{
}

qdTextDB::~qdTextDB()
{
	clear();
}

qdTextDB& qdTextDB::instance()
{
	static qdTextDB db;
	return db;
}

const char* qdTextDB::getText(const char* text_id) const
{
	qdTextMap::const_iterator it = texts_.find(text_id);
	if(it != texts_.end())
		return it->second.text_.c_str();

	static const char* const str = "";
	return str;
}

const char* qdTextDB::getSound(const char* text_id) const
{
	qdTextMap::const_iterator it = texts_.find(text_id);
	if(it != texts_.end())
		return it->second.sound_.c_str();

	static const char* const str = "";
	return str;
}

const char* qdTextDB::getComment(const char* text_id) const
{
#ifndef _FINAL_VERSION_
	qdTextMap::const_iterator it = texts_.find(text_id);
	if(it != texts_.end())
		return it->second.comment_.c_str();
#endif

	static const char* const str = "";
	return str;
}

bool qdTextDB::load(const char* file_name,const char* comments_file_name,bool clear_old_texts)
{
	XStream fh(0);
	if(!fh.open(file_name,XS_IN))
		return false;

	return load(fh,comments_file_name,clear_old_texts);
}

bool qdTextDB::load(XStream& fh,const char* comments_file_name,bool clear_old_texts)
{
	if(clear_old_texts) clear();

	int text_cnt;
	fh > text_cnt;

	std::string id_str(1024,0);
	std::string txt_str(1024,0);
	std::string snd_str(1024,0);
	for(int i = 0; i < text_cnt; i++){
		int id_length;
		fh > id_length;

		id_str.resize(id_length);
		fh.read(&*id_str.begin(),id_length);

		int txt_length;
		fh > txt_length;

		txt_str.resize(txt_length);
		fh.read(&*txt_str.begin(),txt_length);

		int snd_length;
		fh > snd_length;

		snd_str.resize(snd_length);
		fh.read(&*snd_str.begin(),snd_length);

		texts_.insert(qdTextMap::value_type(id_str.c_str(),qdText(txt_str.c_str(),snd_str.c_str())));
	}

	fh.close();

#ifndef _FINAL_VERSION_
	if(comments_file_name){
		XStream fh1;
		if(!fh1.open(comments_file_name,XS_IN))
			return true;

		fh1 > text_cnt;
		for(int i = 0; i < text_cnt; i++){
			int id_length;
			fh1 > id_length;

			id_str.resize(id_length);
			fh1.read(&*id_str.begin(),id_length);

			int txt_length;
			fh1 > txt_length;

			txt_str.resize(txt_length);
			fh1.read(&*txt_str.begin(),txt_length);

			int snd_length;
			fh1 > snd_length;

			snd_str.resize(snd_length);
			fh1.read(&*snd_str.begin(),snd_length);

			qdTextMap::iterator it = texts_.find(id_str.c_str());
			if(it != texts_.end())
				it->second.comment_ = txt_str.c_str();
		}
	}
#endif

	return true;
}

bool qdTextDB::load(XZipStream& fh,const char* comments_file_name,bool clear_old_texts)
{
	if(clear_old_texts) clear();

	int text_cnt;
	fh > text_cnt;

	std::string id_str(1024,0);
	std::string txt_str(1024,0);
	std::string snd_str(1024,0);
	for(int i = 0; i < text_cnt; i++){
		int id_length;
		fh > id_length;

		id_str.resize(id_length);
		fh.read(&*id_str.begin(),id_length);

		int txt_length;
		fh > txt_length;

		txt_str.resize(txt_length);
		fh.read(&*txt_str.begin(),txt_length);

		int snd_length;
		fh > snd_length;

		snd_str.resize(snd_length);
		fh.read(&*snd_str.begin(),snd_length);

		texts_.insert(qdTextMap::value_type(id_str.c_str(),qdText(txt_str.c_str(),snd_str.c_str())));
	}

	fh.close();

#ifndef _FINAL_VERSION_
	if(comments_file_name){
		XStream fh1;
		if(!fh1.open(comments_file_name,XS_IN))
			return true;

		fh1 > text_cnt;
		for(int i = 0; i < text_cnt; i++){
			int id_length;
			fh1 > id_length;

			id_str.resize(id_length);
			fh1.read(&*id_str.begin(),id_length);

			int txt_length;
			fh1 > txt_length;

			txt_str.resize(txt_length);
			fh1.read(&*txt_str.begin(),txt_length);

			int snd_length;
			fh1 > snd_length;

			snd_str.resize(snd_length);
			fh1.read(&*snd_str.begin(),snd_length);

			qdTextMap::iterator it = texts_.find(id_str.c_str());
			if(it != texts_.end())
				it->second.comment_ = txt_str.c_str();
		}
	}
#endif

	return true;
}

void qdTextDB::getIdList(const char* mask, IdList& idList) const
{
	idList.clear();
//	int const maskLen = _tcslen(mask);
	int const maskLen = strlen(mask);
	qdTextMap::const_iterator i;
	FOR_EACH(texts_, i){
		if(!i->first.find(mask)){
			std::string str = i->first;
			str.erase(0, maskLen + 1);
			if (!str.empty())
			{
				int pos = str.find(DELIMETER);
				if(pos != std::string::npos)
					str.erase(pos, str.size());
				if(std::find(idList.begin(), idList.end(), str) == idList.end())
					idList.push_back(str);
			}
		}
	}

	idList.sort();
}

bool qdTextDB::getIdList(IdList& idList) const
{
	try
	{
		std::transform(texts_.begin(), 
						texts_.end(), 
						std::back_inserter(idList),
						std::select1st<qdTextMap::value_type>());

	}catch (std::bad_alloc& ) {
		return false;
	}
	return true;
}

bool qdTextDB::getRootIdList(IdList& idList) const
{
	qdTextMap::const_iterator i = texts_.begin(), e = texts_.end();
	std::string copy;
	for(; i != e; ++i)
	{
		std::string const& str = i->first;
		std::size_t pos = str.find(DELIMETER);
		if (pos == std::string::npos)
			copy.assign(str);
		else
			copy.assign(str, 0, pos);

		if(std::find(idList.begin(), idList.end(), copy) == idList.end())
			idList.push_back(copy);
	}
	return true;
}
