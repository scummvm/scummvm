#!/usr/bin/python

import glob,filecmp,shutil,os,subprocess

toolpath = '/Users/tpfaff/code/residualvm-tools'
modpath = '/Users/tpfaff/grim/ex'
origpath = '/Users/tpfaff/grim/ex_orig'
patchpath = '/tmp/grim_pnc'

neq = 0
tot = 0
for new_file in glob.glob(modpath+'/*.lua'):
	if not "_system" in new_file and not "mouse" in new_file:
		old_file = new_file.replace(modpath,origpath)
		patch_file = patchpath + '/' + os.path.basename(new_file) + '.patchr'
		if not filecmp.cmp(old_file,new_file,False):
			neq += 1
			subprocess.call([toolpath+'/diffr','-n','-m',old_file,new_file,patch_file],shell=False)
		tot += 1
print ('%d/%d files copied' % (neq,tot))
