from __future__ import with_statement
import os
from common_names import *
from collections import defaultdict as defdict


#format - engine name : list of source files
va_macro_src = [("engines\\sci", "resource_patcher.cpp"), ("engines\\ags", "managed_object_pool.h"),
("graphics\\macgui", "mactext.cpp")]

#Format: "file to edit" = "parent folder", src, dst
oneline_patch_src = [
("ptr.h", ["common", r'#include <cstddef>', r'//#include <cstddef>']),

("object.cpp", ["gui", '''error("Unable to load widget position for \'%s\'. Please check your theme files", _name.c_str());''', '''warning("Unable to load widget position for \'%s\'. Please check your theme files", _name.c_str());''']),
# ("", ["", "", ""]),
]

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
   one_line_patch()

def apply_patch(file, val):
   print "File to process: %s\n" %file
   with open(file) as d:
      x = d.read()
   if val[-1] in x: #Already patched
      print "Already patched"
      return
   x = x.replace(val[1], val[-1])
   with open(file, 'w') as f:
      f.write(x)

def patch_line(file, val):
   val = val[0] #get nested list
   folder = val[0]
   if not folder:
      raise "Prent folder for file %s not set!" %file
   files = find_file(folder, file)
   if not files:
      return
   [apply_patch(x, val) for x in files]

def one_line_patch():
   d = defdict(list)
   for k, v in oneline_patch_src:
      d[k].append(v)
   [patch_line(file, val) for file, val in d.iteritems()]

if __name__ == '__main__':
   fix_src()
