# ScummVM - Graphic Adventure Engine
# Copyright (C) 2020 Stryzhniou Fiodar

# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.


from __future__ import with_statement
import os, subprocess, Queue, threading, errno
import multiprocessing as mp
from prj_generator import SafeWriteFile

from common_names import *

prj_template = "PRJ_MMPFILES\n%s"
prj_path = "paralell_build"


def tread_func(q):
   while True:
      fileName = q.get()
      if fileName is None:  # EOF?
         return
      pth = os.path.join(prj_path, fileName)
      try:
         os.makedirs(pth)
      except OSError as e:
         if e.errno == errno.EEXIST:
            pass
         else:
            raise

      fname = os.path.join(mmps, fileName)
      fname = os.path.join("..", fname)
      fname = os.path.join("..", fname) # point to mmp file in mmp folder
      tmp = os.path.join(pth, "bld.inf")
      SafeWriteFile(tmp, prj_template %fname)

      cmd = subprocess.Popen('bldmake bldfiles', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=pth, shell=True)
      out, err = cmd.communicate()
      print "err: %s\n\n" %err
      cmd = subprocess.Popen('abld build gcce urel', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=pth, shell=True)
      out1, err1 = cmd.communicate()
      # I hope it correctly store logs in parallel tasks
      SafeWriteFile(build_log, out + out1, mode = 'a')
      SafeWriteFile(build_err, err + err1, mode = 'a')


def build_mmp():
   q = Queue.Queue()
   fileNames = os.listdir(mmps)
   fileNames = [x for x in fileNames if ".mmp" in x]

   for fileName in fileNames:
      q.put(fileName)
   print q.qsize()
   threads = [ threading.Thread(target=tread_func, args=(q, )) for i in range(mp.cpu_count()) ]
   for thread in threads:
      thread.start()
      q.put(None)  # one EOF marker for each thread


if __name__ == "__main__":
   build_mmp()