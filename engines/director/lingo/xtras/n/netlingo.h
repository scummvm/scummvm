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

#ifndef DIRECTOR_LINGO_XTRAS_N_NETLINGO_H
#define DIRECTOR_LINGO_XTRAS_N_NETLINGO_H

namespace Director {

class NetLingoXtraObject : public Object<NetLingoXtraObject> {
public:
	NetLingoXtraObject(ObjectType objType);

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
};

namespace NetLingoXtra {

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

void m_new(int nargs);
void m_netStatus(int nargs);
void m_getNetText(int nargs);
void m_gotoNetMovie(int nargs);
void m_preloadNetThing(int nargs);
void m_netAbort(int nargs);
void m_gotoNetPage(int nargs);
void m_getLatestNetID(int nargs);
void m_netError(int nargs);
void m_netDone(int nargs);
void m_netTextResult(int nargs);
void m_netMime(int nargs);
void m_netLastModDate(int nargs);
void m_externalEvent(int nargs);
void m_netPresent(int nargs);
void m_downloadNetThing(int nargs);
void m_clearCache(int nargs);
void m_cacheSize(int nargs);
void m_cacheDocVerify(int nargs);
void m_proxyServer(int nargs);
void m_browserName(int nargs);
void m_tellStreamStatus(int nargs);

} // End of namespace NetLingoXtra

} // End of namespace Director

#endif
