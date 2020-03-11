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

#include "ultima/ultima4/tile.h"
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/image.h"
#include "ultima/ultima4/imagemgr.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/tileanim.h"
#include "ultima/ultima4/tilemap.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

TileId Tile::nextId = 0;

Tile::Tile(Tileset *tileset)
    : id(nextId++)
    , name()
    , tileset(tileset)
    , w(0)
    , h(0)
    , frames(0)
    , scale(1)
    , anim(NULL)
    , opaque(false)
    , foreground()
    , waterForeground()
    , rule(NULL)
    , imageName()
    , looks_like()
    , image(NULL)
    , tiledInDungeon(false)
    , directions()
    , animationRule("") {
}

/**
 * Loads tile information.
 */
void Tile::loadProperties(const ConfigElement &conf) {
    if (conf.getName() != "tile")
        return;
            
    name = conf.getString("name"); /* get the name of the tile */

    /* get the animation for the tile, if one is specified */
    if (conf.exists("animation")) {
        animationRule = conf.getString("animation");
    }

    /* see if the tile is opaque */
    opaque = conf.getBool("opaque"); 

    foreground = conf.getBool("usesReplacementTileAsBackground");
    waterForeground = conf.getBool("usesWaterReplacementTileAsBackground");

    /* find the rule that applies to the current tile, if there is one.
       if there is no rule specified, it defaults to the "default" rule */
    if (conf.exists("rule")) {
        rule = TileRule::findByName(conf.getString("rule"));
        if (rule == NULL)
            rule = TileRule::findByName("default");
    }
    else rule = TileRule::findByName("default");

    /* get the number of frames the tile has */    
    frames = conf.getInt("frames", 1);

    /* get the name of the image that belongs to this tile */
    if (conf.exists("image"))
        imageName = conf.getString("image");
    else 
        imageName = Common::String("tile_") + name;

    tiledInDungeon = conf.getBool("tiledInDungeon");

    if (conf.exists("directions")) {
        Common::String dirs = conf.getString("directions");
        if (dirs.size() != (unsigned) frames)
            errorFatal("Error: %ld directions for tile but only %d frames", (long) dirs.size(), frames);
        for (unsigned i = 0; i < dirs.size(); i++) {
            if (dirs[i] == 'w')
                directions.push_back(DIR_WEST);
            else if (dirs[i] == 'n')
                directions.push_back(DIR_NORTH);
            else if (dirs[i] == 'e')
                directions.push_back(DIR_EAST);
            else if (dirs[i] == 's')
                directions.push_back(DIR_SOUTH);
            else
                errorFatal("Error: unknown direction specified by %c", dirs[i]);
        }
    }
}

Image *Tile::getImage() {
    if (!image)
        loadImage();
    return image;
}

/**
 * Loads the tile image
 */ 
void Tile::loadImage() {
    if (!image) {
        scale = settings.scale;

    	SubImage *subimage = NULL;

        ImageInfo *info = imageMgr->get(imageName);
        if (!info) {
            subimage = imageMgr->getSubImage(imageName);
            if (subimage)            
                info = imageMgr->get(subimage->srcImageName);            
        }
        if (!info) //IF still no info loaded
        {
            errorWarning("Error: couldn't load image for tile '%s'", name.c_str());
            return;
        }

        /* FIXME: This is a hack to address the fact that there are 4
           frames for the guard in VGA mode, but only 2 in EGA. Is there
           a better way to handle this? */
        if (name == "guard")
        {
        	if (settings.videoType == "EGA")
        		frames = 2;
        	else
        		frames = 4;
        }


        if (info->image)
        	info->image->alphaOff();

        if (info) {
            w = (subimage ? subimage->width * scale : info->width * scale / info->prescale);
            h = (subimage ? (subimage->height * scale) / frames : (info->height * scale / info->prescale) / frames);
            image = Image::create(w, h * frames, false, Image::HARDWARE);


            //info->image->alphaOff();

            /* draw the tile from the image we found to our tile image */
            if (subimage) {
                Image *tiles = info->image;
                tiles->drawSubRectOn(image, 0, 0, subimage->x * scale, subimage->y * scale, subimage->width * scale, subimage->height * scale);
            }
            else info->image->drawOn(image, 0, 0);
        }

        if (animationRule.size() > 0) {
            extern TileAnimSet *tileanims;

            anim = NULL;
            if (tileanims)
                anim = tileanims->getByName(animationRule);
            if (anim == NULL)
                errorWarning("Warning: animation style '%s' not found", animationRule.c_str());
        }

        /* if we have animations, we always used 'animated' to draw from */
        //if (anim)
        //    image->alphaOff();


    }
}

void Tile::deleteImage()
{
    if(image) {
        delete image;
        image = NULL;
    }
    scale = settings.scale;
}

/**
 * MapTile Class Implementation
 */
Direction MapTile::getDirection() const {
    return getTileType()->directionForFrame(frame);
}

bool MapTile::setDirection(Direction d) {
    /* if we're already pointing the right direction, do nothing! */
    if (getDirection() == d)
        return false;

    const Tile *type = getTileType();

    int new_frame = type->frameForDirection(d);
    if (new_frame != -1) {
        frame = new_frame;
        return true;
    }
    return false;
}

bool Tile::isDungeonFloor() const {
    Tile *floor = tileset->getByName("brick_floor");
    if (id == floor->id)
        return true;
    return false;
}

bool Tile::isOpaque() const {
    extern Context *c;
    return c->opacity ? opaque : false;
}

/**
 * Is tile a foreground tile (i.e. has transparent parts).
 * Deprecated? Never used in XML. Other mechanisms exist, though this could help?
 */
bool Tile::isForeground() const {
    return (rule->mask & MASK_FOREGROUND);
}

Direction Tile::directionForFrame(int frame) const {
    if (static_cast<unsigned>(frame) >= directions.size())
        return DIR_NONE;
    else
        return directions[frame];
}

int Tile::frameForDirection(Direction d) const {
    for (int i = 0; (unsigned) i < directions.size() && i < frames; i++) {
        if (directions[i] == d)
            return i;
    }
    return -1;
}


const Tile *MapTile::getTileType() const {
    return Tileset::findTileById(id);
}

} // End of namespace Ultima4
} // End of namespace Ultima
