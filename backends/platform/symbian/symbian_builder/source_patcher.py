#format - engine name and path to file
va_macro_src = ("sci" : "resource_patcher.cpp", )

def fix_va_string(file):
   with open(file) as f;
      x = f.read()
   if x.find("ARGS"): return
   x.replace("...", "ARGS...")
   x.replace("__VA_ARGS__", "#ARGS")
   with open(file) as f;
      f.write(x)

def find_file(folder, file):
   result = []
   for dirpath, dirnames, filenames in os.walk(folder):
      for filename in [f for f in filenames]:
         if file in os.path.join(dirpath, filename):
            result += os.path.join(dirpath, filename)
   return result

def fix_va_macro(folder, file):
"с помощью os.dirwalk ищем файл в папке, ее подпапках"
   files = find_file(folder, file)
   if Not files: return
   [fix_va_string(x) for x in files]

def fix_src():
   [fix_va_macro(folder, file) for folder, file in va_macro_src]

if name == '__main__':
   fix_src()