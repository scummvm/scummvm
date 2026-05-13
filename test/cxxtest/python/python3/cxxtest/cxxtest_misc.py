#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

import sys
import os

def abort( problem ):
    '''Print error message and exit'''
    sys.stderr.write( '\n' )
    sys.stderr.write( problem )
    sys.stderr.write( '\n\n' )
    sys.exit(2)

if sys.version_info < (2,6):    #pragma: no cover
    def resolve_symlinks(orig_path):
        drive,tmp = os.path.splitdrive(os.path.normpath(orig_path))
        if not drive:
            drive = os.path.sep
        parts = tmp.split(os.path.sep)
        actual_path = [drive]
        while parts:
            actual_path.append(parts.pop(0))
            if not os.path.islink(os.path.join(*actual_path)):
                continue
            actual_path[-1] = os.readlink(os.path.join(*actual_path))
            tmp_drive, tmp_path = os.path.splitdrive(
                dereference_path(os.path.join(*actual_path)) )
            if tmp_drive:
                drive = tmp_drive
            actual_path = [drive] + tmp_path.split(os.path.sep)
        return os.path.join(*actual_path)

    def relpath(path, start=None):
        """Return a relative version of a path.
        (provides compatibility with Python < 2.6)"""
        # Some notes on implementation:
        #   - We rely on resolve_symlinks to correctly resolve any symbolic
        #     links that may be present in the paths
        #   - The explicit handling od the drive name is critical for proper
        #     function on Windows (because os.path.join('c:','foo') yields
        #     "c:foo"!).
        if not start:
            start = os.getcwd()
        ref_drive, ref_path = os.path.splitdrive(
            resolve_symlinks(os.path.abspath(start)) )
        if not ref_drive:
            ref_drive = os.path.sep
        start = [ref_drive] + ref_path.split(os.path.sep)
        while '' in start:
            start.remove('')

        pth_drive, pth_path = os.path.splitdrive(
            resolve_symlinks(os.path.abspath(path)) )
        if not pth_drive:
            pth_drive = os.path.sep
        path = [pth_drive] + pth_path.split(os.path.sep)
        while '' in path:
            path.remove('')

        i = 0
        max = min(len(path), len(start))
        while i < max and path[i] == start[i]:
            i += 1

        if i < 2:
            return os.path.join(*path)
        else:
            rel = ['..']*(len(start)-i) + path[i:]
            if rel:
                return os.path.join(*rel)
            else:
                return '.'
