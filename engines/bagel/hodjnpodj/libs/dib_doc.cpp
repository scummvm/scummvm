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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/libs/dib_doc.h"

namespace Bagel {
namespace HodjNPodj {

CDibDoc::CDibDoc() {
	m_hDIB = nullptr;
	m_palDIB = nullptr;
	m_sizeDoc = CSize(1, 1);     // dummy value to make CScrollView happy
}

CDibDoc::~CDibDoc() {
	if (m_hDIB != nullptr)
		free(m_hDIB);
	if (m_palDIB != nullptr) {
		(*m_palDIB).DeleteObject();
		delete m_palDIB;
	}
}

bool CDibDoc::OpenResourceDocument(const int nResID) {
#ifdef TODO
	char	chResID[8];

	DeleteContents();

	sprintf(chResID, "#%d", nResID);
	m_hDIB = ReadDIBResource(chResID);
	if (m_hDIB != nullptr)
		InitDIBData();

	if (m_hDIB == nullptr)
	{
		char	buf[128];

		sprintf(buf, "Unable to load artwork resource: %s", chResID);
		ShowMemoryInfo(buf, "Internal Problem");
		return FALSE;
	}
	SetPathName(chResID);
	SetModifiedFlag(FALSE);     // start off with unmodified
	return TRUE;
#else
	error("TODO: OpenResourceDocument");
#endif
}


bool CDibDoc::OpenResourceDocument(const char *pszPathName) {
#ifdef TODO
	DeleteContents();

	m_hDIB = ReadDIBResource(pszPathName);
	if (m_hDIB != nullptr)
		InitDIBData();

	if (m_hDIB == nullptr)
	{
		char	buf[128];

		sprintf(buf, "Unable to load artwork file: %s", pszPathName);
		ShowMemoryInfo(buf, "Internal Problem");
		return FALSE;
	}
	SetPathName(" ");
	SetModifiedFlag(FALSE);     // start off with unmodified
	return TRUE;
#else
	error("TODO: OpenResourceDocument");
#endif
}



CPalette *CDibDoc::DetachPalette() {
	CPalette *pMyPalette;

	pMyPalette = m_palDIB;
	m_palDIB = nullptr;

	return pMyPalette;
}


bool CDibDoc::OpenDocument(const char *pszPathName) {
	Common::File f;
	if (!f.open(pszPathName))
		error("Unable to open artwork file: %s", pszPathName);

	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(f))
		error("Unable to load artwork file: %s", pszPathName);
	assert(decoder.hasPalette() && decoder.getPaletteColorCount() == 256);

	m_hDIB = new Graphics::ManagedSurface();
	m_hDIB->copyFrom(decoder.getSurface());
	m_hDIB->setPalette(decoder.getPalette(), 0, 256);

	m_sizeDoc = CBofSize(m_hDIB->w, m_hDIB->h);

	// Create copy of palette
	delete m_palDIB;
	m_palDIB = new CPalette();
	m_palDIB->setData(decoder.getPalette());

	SetPathName(pszPathName);
	SetModifiedFlag(false);

	return TRUE;
}

bool CDibDoc::SaveDocument(const char *pszPathName) {
#ifdef TODO
	CFile file;
	CFileException fe;

	if (!file.Open(pszPathName, CFile::modeCreate |
		CFile::modeReadWrite | CFile::shareExclusive, &fe))
	{
		ReportSaveLoadException(pszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	// replace calls to Serialize with SaveDIB function
	bool bSuccess = FALSE;
	TRY
	{
		//		BeginWaitCursor();
				bSuccess = ::SaveDIB(m_hDIB, file);
				file.Close();
	}
		CATCH(CException, eSave) {
		file.Abort(); // will not throw an exception
		//		EndWaitCursor();
		ReportSaveLoadException(pszPathName, eSave,
			TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		return FALSE;
	}
	END_CATCH

		//	EndWaitCursor();
		SetModifiedFlag(FALSE);     // back to unmodified

	return bSuccess;
#else
	error("TODO: SaveDocument");
#endif
}

void CDibDoc::ReplaceHDIB(HDIB hDIB) {
	free(m_hDIB);
	m_hDIB = hDIB;
}

} // namespace HodjNPodj
} // namespace Bagel
