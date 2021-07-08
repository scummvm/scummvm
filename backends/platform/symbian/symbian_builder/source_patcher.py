from __future__ import with_statement
import os
from common_names import *
from collections import defaultdict as defdict


#format - engine name : list of source files
va_macro_src = [("engines\\sci", "resource_patcher.cpp"), ("engines\\ags", "managed_object_pool.h"),
("graphics\\macgui", "mactext.cpp")]

pt = '..\..\..'

def fix_va_string(file):
   print "File to process: %s" %file
   with open(file) as d:
      x = d.read()
   if "#ARGS" in x: #Already patched
      print "Already patched"
      return
   x = x.replace("...", "ARGS...")
   x = x.replace("__VA_ARGS__", "#ARGS")
   with open(file, 'w') as f:
      f.write(x)

def find_file(folder, files):
   result = []
   directory = os.path.join(pt, folder)
   for dirpath, dirnames, filenames in os.walk(directory):
      for filename in filenames:
         if filename in files:
            result.append(os.path.join(dirpath, filename))
   return result

def fix_va_macro(folder, file):
   files = find_file(folder, file)
   if not files:
      return
   [fix_va_string(x) for x in files]

def fix_src():
   d = defdict(list)
   for k, v in va_macro_src:
      d[k].append(v)
   [fix_va_macro(folder, file) for folder, file in d.iteritems()]

if __name__ == '__main__':
   fix_src()