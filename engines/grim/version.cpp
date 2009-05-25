/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/internal_version.h"
#include "engines/grim/version.h"

namespace Grim {

const char *gResidualVersion = RESIDUAL_VERSION;
const char *gResidualBuildDate = __DATE__ " " __TIME__;
const char *gResidualVersionDate = RESIDUAL_VERSION " (" __DATE__ " " __TIME__ ")";
const char *gResidualFullVersion = "Residual " RESIDUAL_VERSION " (" __DATE__ " " __TIME__ ")";
const char *gResidualFeatures = ""
#ifdef USE_TREMOR
	"Tremor "
#else
#ifdef USE_VORBIS
	"Vorbis "
#endif
#endif

#ifdef USE_FLAC
	"FLAC "
#endif

#ifdef USE_MAD
	"MP3 "
#endif

#ifdef USE_ZLIB
	"zLib "
#endif
	;

} // end of namespace Grim
