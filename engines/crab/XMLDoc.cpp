#include "engines/crab/common_header.h"
#include "engines/crab/XMLDoc.h"

namespace Crab {

void XMLDoc::Load(const Common::String &filename) {
	const Common::Path path(filename);

	if (ready())
		doc.clear();

	if (FileOpen(path, text))
		if (text != NULL)
			doc.parse<0>(text);
}

const rapidxml::xml_document<>* XMLDoc::Doc() const {
	if (text != NULL)
		return &doc;
	else
		return NULL;
}

} // End of namespace Crab
