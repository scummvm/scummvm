/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "qd_file_owner.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

void qdFileOwner::calc_files_size()
{
	files_size_ = 0;

	qdFileNameList list0;
	qdFileNameList list1;

	if(get_files_list(list0,list1)){
		for(qdFileNameList::const_iterator it = list0.begin(); it != list0.end(); ++it)
			files_size_ += app_io::file_size(it -> c_str());
		for(qdFileNameList::const_iterator it = list1.begin(); it != list1.end(); ++it)
			files_size_ += app_io::file_size(it -> c_str());
	}
}

