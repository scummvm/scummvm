#!/usr/bin/python
import glob,struct,sys,os

srcpath = '.'
patchpath = '/tmp/grim_pnc'

class Dataset:
	pass

def writestring(fp,str):
	fp.write(struct.pack('i',len(str)))
	for c in str:
		fp.write(struct.pack('c',c))

dset = []
sname = []
for file in glob.glob(srcpath+'/hs/*.set.hot'):
	set = os.path.basename(file).replace('.hot','')
	with open(file,'r') as fin:
		data = []
		for line in fin:
			line = line.strip()
			if not line:
				continue
			d = Dataset()
			[startp,_,interm] = line.partition('"')
			[d.name,_,endp] = interm.partition('"')
			[setup,_,d.id] = startp.strip().partition(' ')
			d.setup = int(setup)
			tok = endp.strip().split(' ')
			d.type = int(tok[0])
			d.numpos = int(tok[1])
			npos = d.numpos
			d.pos = []
			for i in range(npos*3):
				d.pos.append(float(tok[2+i]))
			d.num = int(tok[2+npos*3])
			d.poly=[]
			for i in range(d.num*2):
				d.poly.append(int(tok[3+npos*3+i]))
			data.append(d)
		dset.append(data)
		sname.append(set)

if not os.path.exists(patchpath):
	os.makedirs(patchpath)

with open(patchpath+'/set.bin','wb') as fout:
	fout.write(struct.pack('i',len(dset)))
	for i in range(len(dset)):
		writestring(fout,sname[i])
		fout.write(struct.pack('i',len(dset[i])))
		for d in dset[i]:
			writestring(fout,d.id)
			writestring(fout,d.name)
			fout.write(struct.pack('iii',d.setup,d.type,d.numpos))
			for i in d.pos:
				fout.write(struct.pack('f',i))
			fout.write(struct.pack('i',d.num))
			for i in d.poly:
				fout.write(struct.pack('i',i))
