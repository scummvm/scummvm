#ifndef SEARCH_MANAGER_H_INCLUDED
#define SEARCH_MANAGER_H_INCLUDED

#include "common/str.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/archive.h"
#include "common/file.h"
#include "common/list.h"

namespace ZVision {

class sManager {
public:
	sManager(const Common::String &root_path, int depth);
	~sManager();

	void addFile(const Common::String &name, Common::Archive *arch);
	void addDir(const Common::String &name);
	void addPatch(const Common::String &src, const Common::String &dst);

	Common::File *openFile(const Common::String &name);
	bool openFile(Common::File &file, const Common::String &name);
	bool hasFile(const Common::String &name);

	void loadZix(const Common::String &name);

private:

	void list_dir_recursive(Common::List<Common::String> &dir_list, const Common::FSNode &fs_node, int depth);

	struct Node {
		Common::String name;
		Common::Archive *arch;
	};

	Common::List<Common::String> dir_list;

	typedef Common::HashMap<Common::String, Node, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> MatchList;

	Common::List<Common::Archive *> archList;
	MatchList files;

	Common::String _root;

private:
};

}

#endif // SEARCH_MANAGER_H_INCLUDED
