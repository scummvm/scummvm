/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/tads/tads2/tads2.h"
#include "glk/tads/tads2/types.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * Signatures for character map files.  The signature is stored at the
 * beginning of the file.  
 */
// single-byte character map version 1.0.0
#define CMAP_SIG_S100  "TADS2 charmap S100\n\r\01a"

void TADS2::cmap_init_default() {
    size_t i;

    // initialize the input table
    for (i = 0 ; i < sizeof(G_cmap_input)/sizeof(G_cmap_input[0]) ; ++i)
        G_cmap_input[i] = (unsigned char)i;

    // initialize the output table
    for (i = 0 ; i < sizeof(G_cmap_output)/sizeof(G_cmap_output[0]) ; ++i)
        G_cmap_output[i] = (unsigned char)i;

    // we have a null ID
    memset(G_cmap_id, 0, sizeof(G_cmap_id));

    // indicate that it's the default
    strcpy(G_cmap_ldesc, "(native/no mapping)");

    // note that we have no character set loaded
    S_cmap_loaded = false;
}

int TADS2::cmap_load_internal(const char *filename) {
    osfildef *fp;
    static char sig1[] = CMAP_SIG_S100;
    char buf[256];
    uchar lenbuf[2];
    size_t len;
    int sysblk;

    // if there's no mapping file, use the default mapping
    if (filename == 0) {
        // initialize with the default mapping
        cmap_init_default();

        // return success
        return 0;
    }
    
    // open the file
    fp = osfoprb(filename, OSFTCMAP);
    if (fp == 0)
        return 1;

    // check the signature
    if (osfrb(fp, buf, sizeof(sig1))
        || memcmp(buf, sig1, sizeof(sig1)) != 0) {
        osfcls(fp);
        return 2;
    }

    // load the ID
    G_cmap_id[4] = '\0';
    if (osfrb(fp, G_cmap_id, 4)) {
        osfcls(fp);
        return 3;
    }

    // load the long description
    if (osfrb(fp, lenbuf, 2)
        || (len = osrp2(lenbuf)) > sizeof(G_cmap_ldesc)
        || osfrb(fp, G_cmap_ldesc, len)) {
        osfcls(fp);
        return 4;
    }

    // load the two tables - input, then output
    if (osfrb(fp, G_cmap_input, sizeof(G_cmap_input))
        || osfrb(fp, G_cmap_output, sizeof(G_cmap_output))) {
        osfcls(fp);
        return 5;
    }

    // read the next section header
    if (osfrb(fp, buf, 4)) {
        osfcls(fp);
        return 6;
    }

    // if it's "SYSI", read the system information string
    if (!memcmp(buf, "SYSI", 4)) {
        // read the length prefix, then the string
        if (osfrb(fp, lenbuf, 2)
            || (len = osrp2(lenbuf)) > sizeof(buf)
            || osfrb(fp, buf, len)) {
            osfcls(fp);
            return 7;
        }

        // we have a system information block
        sysblk = true;
    } else {
        // there's no system information block
        sysblk = false;
    }

    /*
     * call the OS code, so that it can do any system-dependent
     * initialization for the new character mapping 
     */
    os_advise_load_charmap(G_cmap_id, G_cmap_ldesc, sysblk ? buf : "");

    // read the next section header
    if (sysblk && osfrb(fp, buf, 4)) {
        osfcls(fp);
        return 8;
    }

    // see if we have an entity list
    if (!memcmp(buf, "ENTY", 4)) {
        // read the entities
        for (;;) {
            unsigned int cval;
            char expansion[CMAP_MAX_ENTITY_EXPANSION];
            
            // read the next item's length and character value
            if (osfrb(fp, buf, 4)) {
                osfcls(fp);
                return 9; 
            }

            // decode the values
            len = osrp2(buf);
            cval = osrp2(buf+2);

            // if we've reached the zero marker, we're done
            if (len == 0 && cval == 0)
                break;

            // read the string
            if (len > CMAP_MAX_ENTITY_EXPANSION
                || osfrb(fp, expansion, len)) {
                osfcls(fp);
                return 10;
            }

            // tell the output code about the expansion
            tio_set_html_expansion(cval, expansion, len);
        }
    }

    /* 
     * ignore anything else we find - if the file format is updated to
     * include extra information in the future, and this old code tries
     * to load an updated file, we'll just ignore the new information,
     * which should always be placed after the "SYSI" block (if present)
     * to ensure compatibility with past versions (such as this code)
     */
    // no problems - close the file and return success
    osfcls(fp);
    return 0;
}

int TADS2::cmap_load(const char *filename) {
    int err;
    
    // try loading the file
    if ((err = cmap_load_internal(filename)) != 0)
        return err;

    /*
     * note that we've explicitly loaded a character set, if they named
     * a character set (if not, this simply establishes the default
     * setting, so we haven't explicitly loaded anything) 
     */
    if (filename != nullptr)
        S_cmap_loaded = true;

    // success
    return 0;
}

void TADS2::cmap_override() {
    // apply the default mapping
    cmap_init_default();

    /* 
     * pretend we have a character map loaded, so that we don't try to
     * load another one if the game specifies a character set
     */
    S_cmap_loaded = true;
}

void TADS2::cmap_set_game_charset(errcxdef *ec, const char *internal_id,
		const char *internal_ldesc, const char *argv0) {
    char filename[OSFNMAX];
    
    /* 
     * If a character set is already explicitly loaded, ignore the
     * game's character set - the player asked us to use a particular
     * mapping, so ignore what the game wants.  (This will probably
     * result in incorrect display of non-ASCII character values, but
     * the player is most likely to use this to avoid errors when an
     * appropriate mapping file for the game is not available.  In this
     * case, the player informs us by setting the option that he or she
     * knows and accepts that the game will not look exactly right.)  
     */
    if (S_cmap_loaded)
        return;

    /*
     * ask the operating system to name the mapping file -- this routine
     * will determine, if possible, the current native character set,
     * and apply a system-specific naming convention to tell us what
     * mapping file we should open 
     */
    os_gen_charmap_filename(filename, internal_id, argv0);

    // try loading the mapping file
    if (cmap_load_internal(filename))
        errsig2(ec, ERR_CHRNOFILE,
                ERRTSTR, errstr(ec, filename, strlen(filename)),
                ERRTSTR, errstr(ec, internal_ldesc, strlen(internal_ldesc)));

    /**
     * We were successful - the game's internal character set is now
     * mapped to the current native character set.  Even though we
     * loaded an ldesc from the mapping file, forget that and store the
     * internal ldesc that the game specified.  The reason we do this is
     * that it's possible that the player will dynamically switch native
     * character sets in the future, at which point we'll need to
     * re-load the mapping table, which could raise an error if a
     * mapping file for the new character set isn't available.  So, we
     * may need to provide the same explanation later that we needed to
     * provide here.  Save the game's character set ldesc for that
     * eventuality, since it describes exactly what the *game* wanted.  
     */
    strcpy(G_cmap_ldesc, internal_ldesc);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
