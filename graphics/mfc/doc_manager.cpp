/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "graphics/mfc/afxwin.h"

namespace Graphics {
namespace MFC {

void CDocManager::AddDocTemplate(CDocTemplate *pTemplate) {
	if (pTemplate == nullptr) {
		for (auto it = pStaticList.begin(); it != pStaticList.end(); ++it) {
			CDocTemplate *tmp = *it;
			AddDocTemplate(tmp);
		}
		pStaticList.clear();
		bStaticInit = false;
	} else {
		ASSERT_VALID(pTemplate);
		assert(!m_templateList.contains(pTemplate));
		pTemplate->LoadTemplate();
		m_templateList.push_back(pTemplate);
	}
}

void CDocManager::OnFileNew() {
	assert(!m_templateList.empty());

	// ScummVM doesn't support more than 1 template
	assert(m_templateList.size() == 1);

	CDocTemplate *pTemplate = m_templateList.front();
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	pTemplate->OpenDocumentFile(nullptr);
}

void CDocManager::OnFileOpen() {
	error("TODO: CDocManager::onFileOpen");
}

void CDocManager::CloseAllDocuments(bool bEndSession) {
	for (auto it = m_templateList.begin(); it != m_templateList.end(); ++it) {
		CDocTemplate *pTemplate = *it;
		ASSERT_KINDOF(CDocTemplate, pTemplate);
		pTemplate->CloseAllDocuments(bEndSession);
	}
}

} // namespace MFC
} // namespace Graphics
