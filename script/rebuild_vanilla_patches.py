#!/usr/bin/python

import glob,filecmp,shutil,os,subprocess

toolpath = '/Users/tpfaff/code/residualvm-tools'
origpath = '/Users/tpfaff/grim/ex_orig'
modpath = '/Users/tpfaff/grim/ex'
patches = '/Users/tpfaff/code/residualvm/dists/engine-data/patches/grim'
deltas = '/Users/tpfaff/code/residualvm/dists/engine-data/patches/grim_pnc_delta'
dest = '/Users/tpfaff/grimex_orig/vanilla'
patchpath = '/tmp/grim_pnc'

# init empty dir
if os.path.exists(dest):
	for i in glob.glob(dest+'/*'):
		os.remove(i)
else:
	os.makedirs(dest)

# compare patches
for f in glob.glob(patches+'/*'):
	filename = os.path.basename(f)
	datafile = filename.replace('.patchr','')
	destfile = deltas+'/'+filename
	if filename.endswith('.lua.patchr') and not filename.startswith('_system'):
		if not os.path.exists(destfile) and not os.path.exists(patchpath+'/'+filename):
			print "!!!! WARNING: " + filename + " doesn't exist !!!!"
		
		subprocess.call([toolpath+'/patchr',origpath+'/'+datafile,'/tmp/a.lua',f],shell=False)
		subprocess.call('%s/delua /tmp/a.lua > %s/%s' % (toolpath,dest,datafile),shell=True)
		if not filecmp.cmp(dest+'/'+datafile,modpath+'_lua/'+datafile):
			print datafile
			#subprocess.call(['/usr/local/bin/meld',dest+'/'+datafile,origpath+'_lua/'+datafile,modpath+'_lua/'+datafile])
		else:
			print datafile + ' (match)'
	else:
		if not os.path.exists(destfile):
			print "!!!! WARNING: " + filename + " doesn't exist !!!!"
		elif not filecmp.cmp(f,destfile,False):
			print "!!!! WARNING: " + filename + " different !!!!"
