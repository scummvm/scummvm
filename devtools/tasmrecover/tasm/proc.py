import re
import op

class proc:
	last_addr = 0xc000
	
	def __init__(self, name):
		self.name = name
		self.calls = []
		self.stmts = []
		self.labels = set()
		self.__label_re = re.compile(r'^(\S+):(.*)$')
		self.offset = proc.last_addr
		proc.last_addr += 4

	def add_label(self, label):
		self.stmts.append(op.label(label))
		self.labels.add(label)
	
	def optimize(self):
		print "optimizing..."
		#trivial simplifications, removing last ret
		while len(self.stmts) and isinstance(self.stmts[-1], op.label):
			print "stripping last label"
			self.stmts.pop()
		#if isinstance(self.stmts[-1], op._ret) and (len(self.stmts) < 2 or not isinstance(self.stmts[-2], op.label)):
		#	print "stripping last ret"
		#	self.stmts.pop()
		#merging push ax pop bx constructs
		i = 0
		while i + 1 < len(self.stmts):
			a, b = self.stmts[i], self.stmts[i + 1]
			if isinstance(a, op._push) and isinstance(b, op._pop):
				ar, br = a.regs, b.regs
				movs = []
				while len(ar) and len(br):
					src = ar.pop()
					dst = br.pop(0)
					movs.append(op._mov2(dst, src))
				if len(br) == 0:
					self.stmts.pop(i + 1)
				print "merging %d push-pops into movs" %(len(movs))
				for m in movs:
					print "\t%s <- %s" %(m.dst, m.src)
				self.stmts[i + 1:i + 1] = movs
				if len(ar) == 0:
					self.stmts.pop(i)
			else:
				i += 1
		#fixme: add local? 
	
	def add(self, stmt):
		#print stmt
		comment = stmt.rfind(';')
		if comment >= 0:
			stmt = stmt[:comment]
		stmt = stmt.strip()

		r = self.__label_re.search(stmt)
		if r is not None:
			#label
			self.add_label(r.group(1).lower())
			#print "remains: %s" %r.group(2)
			stmt = r.group(2).strip()

		if len(stmt) == 0:
			return
		
		s = stmt.split(None)
		cmd = s[0]
		cl = getattr(op, '_' + cmd)
		arg = " ".join(s[1:]) if len(s) > 1 else str()
		o = cl(arg)
		self.stmts.append(o)
	
	def __str__(self):
		r = []
		for i in self.stmts:
			r.append(i.__str__())
		return "\n".join(r)

	def visit(self, visitor, skip = 0):
		for i in xrange(skip, len(self.stmts)):
			self.stmts[i].visit(visitor)
