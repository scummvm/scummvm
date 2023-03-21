#include "engines/crab/common_header.h"
#include "engines/crab/XMLDoc.h"

namespace Crab {

void XMLDoc::Load(const Common::String &filename) {
	if (ready())
		doc.clear();

	if (FileOpen(filename.c_str(), text))
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
