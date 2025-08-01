#!/usr/bin/env python3
# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import os
import json
import sys
from pathlib import Path

sym_links = {}
ignore_files = ['.git', 'index.json']

def rd_sync(dpath, tree, name):
    """Recursively scan directory and build file tree structure."""
    try:
        files = os.listdir(dpath)
    except (OSError, PermissionError):
        return tree
    
    for file in files:
        # ignore non-essential directories / files
        if file in ignore_files or file.startswith('.'):
            continue
            
        fpath = os.path.join(dpath, file)
        
        try:
            # Avoid infinite loops with symbolic links
            lstat = os.lstat(fpath)
            if os.path.islink(fpath):
                dev = lstat.st_dev
                ino = lstat.st_ino
                
                if dev not in sym_links:
                    sym_links[dev] = {}
                
                # Ignore if we've seen it before
                if ino in sym_links[dev]:
                    continue
                    
                sym_links[dev][ino] = True
            
            if os.path.isdir(fpath):
                child = {}
                tree[file] = child
                rd_sync(fpath, child, file)
                
                # Write index.json for this directory
                fs_listing = json.dumps(child)
                fname = os.path.join(fpath, "index.json")
                with open(fname, 'w', encoding='utf-8') as f:
                    f.write(fs_listing)
                
                # Reset tree entry to empty dict after writing index
                tree[file] = {}
            else:
                # Store file size
                stat = os.stat(fpath)
                tree[file] = stat.st_size
                
        except (OSError, PermissionError):
            # Ignore and move on
            continue
    
    return tree

def main():
    if len(sys.argv) == 2:
        root_folder = sys.argv[1]
        fs_listing = json.dumps(rd_sync(root_folder, {}, '/'))
        fname = os.path.join(root_folder, "index.json")
        with open(fname, 'w', encoding='utf-8') as f:
            f.write(fs_listing)
    else:
        root_folder = os.getcwd()
        fs_listing = json.dumps(rd_sync(root_folder, {}, '/'))
        print(fs_listing)

if __name__ == "__main__":
    main()
