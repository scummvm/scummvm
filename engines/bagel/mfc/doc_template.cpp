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
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CDocTemplate, CCmdTarget)
BEGIN_MESSAGE_MAP(CDocTemplate, CCmdTarget)
END_MESSAGE_MAP()

CDocTemplate::CDocTemplate(UINT nIDResource, const CRuntimeClass *pDocClass,
		const CRuntimeClass *pFrameClass, const CRuntimeClass *pViewClass) {
	assert(nIDResource != 0);
	ASSERT(pDocClass == nullptr ||
		pDocClass->IsDerivedFrom(RUNTIME_CLASS(CDocument)));
	ASSERT(pFrameClass == nullptr ||
		pFrameClass->IsDerivedFrom(RUNTIME_CLASS(CFrameWnd)));
	ASSERT(pViewClass == nullptr ||
		pViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));

	m_nIDResource = nIDResource;
	m_pDocClass = pDocClass;
	m_pFrameClass = pFrameClass;
	m_pViewClass = pViewClass;
}

} // namespace MFC
} // namespace Bagel
