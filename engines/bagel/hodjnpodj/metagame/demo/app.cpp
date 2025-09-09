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

#include "bagel/hodjnpodj/metagame/demo/app.h"
#include "bagel/hodjnpodj/metagame/demo/hodjpodj.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Demo {

CTheApp::CTheApp() {
}

bool CTheApp::InitApplication() {
	addResources("hnpdemo.exe");
	addResources("hnpzm.dll");
	addFontResource("msserif.fon");

	return CWinApp::InitApplication();
}

bool CTheApp::InitInstance() {
	// Main demo
	m_pMainWnd = new CHodjPodjWindow();
	m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
	m_pMainWnd->UpdateWindow();

	return true;
}

int CTheApp::ExitInstance() {
	return 0;
}

bool CTheApp::OnIdle(long lCount) {
	CBofSound::audioTask();
	return true;
}

} // namespace Demo
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
