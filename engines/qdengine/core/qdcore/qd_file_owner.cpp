/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_file_owner.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

void qdFileOwner::calc_files_size() {
	files_size_ = 0;

	qdFileNameList list0;
	qdFileNameList list1;

	if (get_files_list(list0, list1)) {
		for (qdFileNameList::const_iterator it = list0.begin(); it != list0.end(); ++it)
			files_size_ += app_io::file_size(it -> c_str());
		for (qdFileNameList::const_iterator it = list1.begin(); it != list1.end(); ++it)
			files_size_ += app_io::file_size(it -> c_str());
	}
}

} // namespace QDEngine
