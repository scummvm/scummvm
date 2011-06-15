import op, traceback, re, proc
from copy import copy
proc_module = proc

class CrossJump(Exception):
	pass

def parse_bin(s):
	b = s.group(1)
	v = hex(int(b, 2))
	#print "BINARY: %s -> %s" %(b, v)
	return v

class cpp:
	def __init__(self, context, namespace, skip_first = 0, blacklist = []):
		self.namespace = namespace
		fname = namespace + ".cpp"
		header = namespace + ".h"
		banner = "/* PLEASE DO NOT MODIFY THIS FILE. ALL CHANGES WILL BE LOST! LOOK FOR README FOR DETAILS */"
		self.fd = open(fname, "wt")
		self.hd = open(header, "wt")
		hid = "TASMRECOVER_%s_STUBS_H__" %namespace.upper()
		self.hd.write("""#ifndef %s
#define %s

%s

""" %(hid, hid, banner))
		self.context = context
		self.data_seg = context.binary_data
		self.procs = context.proc_list
		self.skip_first = skip_first
		self.proc_queue = []
		self.proc_done = []
		self.blacklist = blacklist
		self.failed = list(blacklist)
		self.translated = []
		self.proc_addr = []
		self.forwards = []
		self.fd.write("""%s

#include \"%s\"

namespace %s {

""" %(banner, header, namespace))

	def expand_cb(self, match):
		name = match.group(0).lower()
		if len(name) == 2 and \
			((name[0] in ['a', 'b', 'c', 'd'] and name[1] in ['h', 'x', 'l']) or name in ['si', 'di', 'es', 'ds', 'cs']):
			return "context.%s" %name

		if self.indirection == -1:
			try:
				offset,p,p = self.context.get_offset(name)
				print "OFFSET = %d" %offset
				self.indirection = 0
				return str(offset)
			except:
				pass
		
		g = self.context.get_global(name)
		if isinstance(g, op.const):
			value = self.expand_equ(g.value)
			print "equ: %s -> %s" %(name, value)
		elif isinstance(g, proc.proc):
			if self.indirection != -1:
				raise Exception("invalid proc label usage")
			value = str(g.offset)
			self.indirection = 0
		else:
			size = g.size
			if size == 0:
				raise Exception("invalid var '%s' size %u" %(name, size))
			if self.indirection == 0:
				value = "context.data.%s(k%s)" %("byte" if size == 1 else "word", name.capitalize())
			elif self.indirection == -1:
				value = "%s" %g.offset
				self.indirection = 0
			else:
				raise Exception("invalid indirection %d" %self.indirection)
		return value
	
	def get_size(self, expr):
		#print 'get_size("%s")' %expr
		try:
			v = self.context.parse_int(expr)
			return 1 if v < 256 else 2
		except:
			pass

		if re.match(r'byte\s+ptr\s', expr) is not None:
			return 1

		if re.match(r'word\s+ptr\s', expr) is not None:
			return 2

		if len(expr) == 2 and expr[0] in ['a', 'b', 'c', 'd'] and expr[1] in ['h', 'l']:
			return 1
		if expr in ['ax', 'bx', 'cx', 'dx', 'si', 'di', 'sp', 'bp', 'ds', 'cs', 'es', 'fs']:
			return 2

		m = re.match(r'[a-zA-Z_]\w*', expr)
		if m is not None:
			name = m.group(0)
			try:
				g = self.context.get_global(name)
				return g.size
			except:
				pass

		return 0

	def expand_equ_cb(self, match):
		name = match.group(0).lower()
		g = self.context.get_global(name)
		if isinstance(g, op.const):
			return g.value
		return str(g.offset)

	def expand_equ(self, expr):
		n = 1
		while n > 0:
			expr, n = re.subn(r'\b[a-zA-Z_][a-zA-Z0-9_]+\b', self.expand_equ_cb, expr)
		expr = re.sub(r'\b([0-9][a-fA-F0-9]*)h', '0x\\1', expr)
		return "(%s)" %expr

	def expand(self, expr, def_size = 0):
		#print "EXPAND \"%s\"" %expr
		size = self.get_size(expr) if def_size == 0 else def_size
		indirection = 0
		seg = None
		reg = True

		m = re.match(r'seg\s+(.*?)$', expr)
		if m is not None:
			return "context.data"
		
		match_id = True
		m = re.match(r'offset\s+(.*?)$', expr)
		if m is not None:
			indirection -= 1
			expr = m.group(1).strip()

		m = re.match(r'byte\s+ptr\s+(.*?)$', expr)
		if m is not None:
			expr = m.group(1).strip()

		m = re.match(r'word\s+ptr\s+(.*?)$', expr)
		if m is not None:
			expr = m.group(1).strip()

		m = re.match(r'\[(.*)\]$', expr)
		if m is not None:
			indirection += 1
			expr = m.group(1).strip()

		m = re.match(r'(\w{2,2}):(.*)$', expr)
		if m is not None:
			seg_prefix = m.group(1)
			expr = m.group(2).strip()
			print "SEGMENT %s, remains: %s" %(seg_prefix, expr)
		else:
			seg_prefix = "ds"

		m = re.match(r'(([abcd][xhl])|si|di|bp|sp)([\+-].*)?$', expr)
		if m is not None:
			reg = m.group(1)
			plus = m.group(3)
			if plus is not None:
				plus = self.expand(plus)
			else:
				plus = ""
			match_id = False
			#print "COMMON_REG: ", reg, plus
			expr = "context.%s%s" %(reg, plus)

		expr = re.sub(r'\b([0-9][a-fA-F0-9]*)h', '0x\\1', expr)
		expr = re.sub(r'\b([0-1]+)b', parse_bin, expr)
		expr = re.sub(r'"(.)"', '\'\\1\'', expr)
		if match_id:
			#print "BEFORE: %d" %indirection
			self.indirection = indirection
			expr = re.sub(r'\b[a-zA-Z_][a-zA-Z0-9_]+\b', self.expand_cb, expr)
			indirection = self.indirection
			#print "AFTER: %d" %indirection
		
		if indirection == 1:
			if size == 1:
				expr = "context.%s.byte(%s)" %(seg_prefix, expr)
			elif size == 2:
				expr = "context.%s.word(%s)" %(seg_prefix, expr)
			else:
				expr = "@invalid size 0"
		elif indirection == 0:
			pass
		elif indirection == -1:
			expr = "&%s" %expr
		else:
			raise Exception("invalid indirection %d" %indirection)
		return expr
	
	def mangle_label(self, name):
		name = name.lower()
		return re.sub(r'\$', '_tmp', name)

	def resolve_label(self, name):
		name = name.lower()
		if not name in self.proc.labels:
			try:
				offset, proc, pos = self.context.get_offset(name)
			except:
				print "no label %s, trying procedure" %name
				proc = self.context.get_global(name)
				pos = 0
				if not isinstance(proc, proc_module.proc):
					raise CrossJump("cross-procedure jump to non label and non procedure %s" %(name))
			self.proc.labels.add(name)
			for i in xrange(0, len(self.unbounded)):
				u = self.unbounded[i]
				if u[1] == proc:
					if pos < u[2]:
						self.unbounded[i] = (name, proc, pos)
				return self.mangle_label(name)
			self.unbounded.append((name, proc, pos))
		
		return self.mangle_label(name)

	def jump_to_label(self, name):
		jump_proc = False
		if name in self.blacklist:
			jump_proc = True
		
		if self.context.has_global(name) :
			g = self.context.get_global(name)
			if isinstance(g, proc_module.proc):
				jump_proc = True
		
		if jump_proc:
			self.add_forward(name)
			return "{ %s(context); return; }" %name
		else:
			return "goto %s" %self.resolve_label(name)
	
	def _label(self, name):
		self.body += "%s:\n" %self.mangle_label(name)
	
	def schedule(self, name):
		name = name.lower()
		if name in self.proc_queue or name in self.proc_done or name in self.failed:
			return
		print "+scheduling function %s..." %name
		self.proc_queue.append(name)

	def add_forward(self, name):
		if name not in self.forwards and name not in self.failed:
			self.forwards.append(name)
	
	def _call(self, name):
		name = name.lower()
		if name == 'ax':
			self.body += "\t__dispatch_call(context, %s);\n" %self.expand('ax', 2)
			return
		self.body += "\t%s(context);\n" %name
		self.add_forward(name);
		self.schedule(name)

	def _ret(self):
		self.body += "\treturn;\n"

	def parse2(self, dst, src):
		dst_size, src_size = self.get_size(dst), self.get_size(src)
		if dst_size == 0:
			if src_size == 0:
				raise Exception("both sizes are 0")
			dst_size = src_size
		if src_size == 0:
			src_size = dst_size

		dst = self.expand(dst, dst_size)
		src = self.expand(src, src_size)
		return dst, src

	def _mov(self, dst, src):
		self.body += "\t%s = %s;\n" %self.parse2(dst, src)

	def _add(self, dst, src):
		self.body += "\tcontext._add(%s, %s);\n" %self.parse2(dst, src)

	def _sub(self, dst, src):
		self.body += "\tcontext._sub(%s, %s);\n" %self.parse2(dst, src)

	def _and(self, dst, src):
		self.body += "\tcontext._and(%s, %s);\n" %self.parse2(dst, src)

	def _or(self, dst, src):
		self.body += "\tcontext._or(%s, %s);\n" %self.parse2(dst, src)

	def _xor(self, dst, src):
		self.body += "\tcontext._xor(%s, %s);\n" %self.parse2(dst, src)

	def _neg(self, dst):
		dst = self.expand(dst)
		self.body += "\tcontext._neg(%s);\n" %(dst)

	def _cbw(self):
		self.body += "\tcontext.ax.cbw();\n"

	def _shr(self, dst, src):
		self.body += "\tcontext._shr(%s, %s);\n" %self.parse2(dst, src)

	def _shl(self, dst, src):
		self.body += "\tcontext._shl(%s, %s);\n" %self.parse2(dst, src)

	#def _sar(self, dst, src):
	#	self.body += "\tcontext._sar(%s%s);\n" %self.parse2(dst, src)

	#def _sal(self, dst, src):
	#	self.body += "\tcontext._sal(%s, %s);\n" %self.parse2(dst, src)

	#def _rcl(self, dst, src):
	#	self.body += "\tcontext._rcl(%s, %s);\n" %self.parse2(dst, src)

	#def _rcr(self, dst, src):
	#	self.body += "\tcontext._rcr(%s, %s);\n" %self.parse2(dst, src)

	def _mul(self, src):
		src = self.expand(src)
		self.body += "\tcontext._mul(%s);\n" %(src)

	def _div(self, src):
		src = self.expand(src)
		self.body += "\tcontext._div(%s);\n" %(src)

	def _inc(self, dst):
		dst = self.expand(dst)
		self.body += "\tcontext._inc(%s);\n" %(dst)

	def _dec(self, dst):
		dst = self.expand(dst)
		self.body += "\tcontext._dec(%s);\n" %(dst)

	def _cmp(self, a, b):
		self.body += "\tcontext._cmp(%s, %s);\n" %self.parse2(a, b)

	def _test(self, a, b):
		self.body += "\tcontext._test(%s, %s);\n" %self.parse2(a, b)

	def _js(self, label):
		self.body += "\tif (context.flags.s()) %s;\n" %(self.jump_to_label(label))

	def _jns(self, label):
		self.body += "\tif (!context.flags.s()) %s;\n" %(self.jump_to_label(label)) 

	def _jz(self, label):
		self.body += "\tif (context.flags.z()) %s;\n" %(self.jump_to_label(label)) 

	def _jnz(self, label):
		self.body += "\tif (!context.flags.z()) %s;\n" %(self.jump_to_label(label)) 

	def _jl(self, label):
		self.body += "\tif (context.flags.l()) %s;\n" %(self.jump_to_label(label)) 

	def _jg(self, label):
		self.body += "\tif (!context.flags.le()) %s;\n" %(self.jump_to_label(label)) 

	def _jle(self, label):
		self.body += "\tif (context.flags.le()) %s;\n" %(self.jump_to_label(label)) 

	def _jge(self, label):
		self.body += "\tif (!context.flags.l()) %s;\n" %(self.jump_to_label(label)) 

	def _jc(self, label):
		self.body += "\tif (context.flags.c()) %s;\n" %(self.jump_to_label(label)) 

	def _jnc(self, label):
		self.body += "\tif (!context.flags.c()) %s;\n" %(self.jump_to_label(label)) 
	
	def _xchg(self, dst, src):
		self.body += "\tcontext._xchg(%s, %s);\n" %self.parse2(dst, src)

	def _jmp(self, label):
		self.body += "\t%s;\n" %(self.jump_to_label(label)) 

	def _loop(self, label):
		self.body += "\tif (--context.cx) %s;\n" %self.jump_to_label(label)

	def _push(self, regs):
		p = str();
		for r in regs:
			r = self.expand(r)
			p += "\tcontext.push(%s);\n" %(r)
		self.body += p

	def _pop(self, regs):
		p = str();
		for r in regs:
			self.temps_count -= 1
			i = self.temps_count
			r = self.expand(r)
			p += "\t%s = context.pop();\n" %r
		self.body += p

	def _rep(self):
		self.body += "\twhile(context.cx--) "

	def _lodsb(self):
		self.body += "\tcontext._lodsb();\n"

	def _lodsw(self):
		self.body += "\tcontext._lodsw();\n"

	def _stosb(self, n):
		self.body += "\tcontext._stosb(%s);\n" %("" if n == 1 else n)

	def _stosw(self, n):
		self.body += "\tcontext._stosw(%s);\n" %("" if n == 1 else n)

	def _movsb(self, n):
		self.body += "\tcontext._movsb(%s);\n " %("" if n == 1 else n)

	def _movsw(self, n):
		self.body += "\tcontext._movsw(%s);\n " %("" if n == 1 else n)

	def _stc(self):
		self.body += "\tcontext.flags._c = true;\n "

	def _clc(self):
		self.body += "\tcontext.flags._c = false;\n "

	def __proc(self, name, def_skip = 0):
		try:
			skip = def_skip
			self.temps_count = 0
			self.temps_max = 0
			if self.context.has_global(name):
				self.proc = self.context.get_global(name)
			else:
				print "No procedure named %s, trying label" %name
				off, src_proc, skip = self.context.get_offset(name)
				
				self.proc = proc_module.proc(name)
				self.proc.stmts = copy(src_proc.stmts)
				self.proc.labels = copy(src_proc.labels)
				#for p in xrange(skip, len(self.proc.stmts)):
				#	s = self.proc.stmts[p]
				#	if isinstance(s, op.basejmp):
				#		o, p, s = self.context.get_offset(s.label)
				#		if p == src_proc and s < skip:
				#			skip = s
			
			
			self.proc_addr.append((name, self.proc.offset))
			self.body = str()
			self.body += "void %s(Context &context) {\n\tSTACK_CHECK(context);\n" %name;
			self.proc.optimize()
			self.unbounded = []
			self.proc.visit(self, skip)

			#adding remaining labels:
			for i in xrange(0, len(self.unbounded)):
				u = self.unbounded[i]
				print "UNBOUNDED: ", u
				proc = u[1]
				for p in xrange(u[2], len(proc.stmts)):
					s = proc.stmts[p]
					if isinstance(s, op.basejmp):
						self.resolve_label(s.label)
			
			#adding statements
			for label, proc, offset in self.unbounded:
				self.body += "/*continuing to unbounded code: %s from %s:%d-%d*/\n" %(label, proc.name, offset, len(proc.stmts))
				start = len(self.proc.stmts)
				self.proc.add_label(label)
				for s in proc.stmts[offset:]:
					if isinstance(s, op.label):
						self.proc.labels.add(s.name)
					self.proc.stmts.append(s)
				self.proc.add("ret")
				print "skipping %d instructions, todo: %d" %(start, len(self.proc.stmts) - start)
				self.proc.visit(self, start)
			self.body += "}\n";
			self.translated.insert(0, self.body)
			self.proc = None
			if self.temps_count > 0:
				raise Exception("temps count == %d at the exit of proc" %self.temps_count);
			return True
		except (CrossJump, op.Unsupported) as e:
			print "%s: ERROR: %s" %(name, e)
			self.failed.append(name)
		except:
			raise

	def get_type(self, width):
		return "uint%d_t" %(width * 8)
	
	def write_stubs(self, fname, procs):
		fd = open(fname, "wt")
		fd.write("namespace %s {\n" %self.namespace)
		for p in procs:
			fd.write("void %s(Context &context) {\n\t::error(\"%s\");\n}\n\n" %(p, p))
		fd.write("} /*namespace %s */\n" %self.namespace)
		fd.close()


	def generate(self, start):
		#print self.prologue()
		#print context
		self.proc_queue.append(start)
		while len(self.proc_queue):
			name = self.proc_queue.pop()
			if name in self.failed or name in self.proc_done:
				continue
			if len(self.proc_queue) == 0 and len(self.procs) > 0:
				print "queue's empty, adding remaining procs:"
				for p in self.procs:
					self.schedule(p)
				self.procs = []
			print "continuing on %s" %name
			self.proc_done.append(name)
			self.__proc(name)
		self.write_stubs("_stubs.cpp", self.failed)
		done, failed = len(self.proc_done), len(self.failed)

		for f in self.forwards:
			if f not in self.failed:
				self.fd.write("void %s(Context &context);\n" %f)

		self.fd.write("\n")
		self.fd.write("\n".join(self.translated))
		self.fd.write("\n\n")
		print "%d ok, %d failed of %d, %.02g%% translated" %(done, failed, done + failed, 100.0 * done / (done + failed))
		print "\n".join(self.failed)
		data_bin = self.data_seg
		data_impl = "\n\tstatic const uint8 src[] = {\n\t\t"
		n = 0
		for v in data_bin:
			data_impl += "0x%02x, " %v
			n += 1
			if (n & 0xf) == 0:
				data_impl += "\n\t\t"
		data_impl += "};\n\tcontext.ds.assign(src, src + sizeof(src));\n"
		self.hd.write(
"""\n#include "dreamweb/runtime.h"

namespace %s {

	void __dispatch_call(Context &context, unsigned addr);
	void __start(Context &context);

""" %(self.namespace))
		for f in self.failed:
			self.hd.write("\tvoid %s(Context &context);\n" %f)
		
		offsets_decl = "\n"
		offsets = []
		for k, v in self.context.get_globals().items():
			if isinstance(v, op.var):
				offsets.append((k.capitalize(), v.offset))
			elif isinstance(v, op.const):
				offsets.append((k.capitalize(), self.expand_equ(v.value))) #fixme: try to save all constants here
		
		offsets = sorted(offsets, key=lambda t: t[1])
		for o in offsets:
			offsets_decl += "\tconst static uint16 k%s = %s;\n" %o
		offsets_decl += "\n"
		self.hd.write(offsets_decl);

		self.hd.write("\n}\n\n#endif\n")
		self.hd.close()
		
		self.fd.write("\nvoid __start(Context &context) { %s%s(context); \n}\n" %(data_impl, start))
		
		self.fd.write("\nvoid __dispatch_call(Context &context, unsigned addr) {\n\tswitch(addr) {\n")
		self.proc_addr.sort(cmp = lambda x, y: x[1] - y[1])
		for name,addr in self.proc_addr:
			self.fd.write("\t\tcase 0x%04x: %s(context); break;\n" %(addr, name))
		self.fd.write("\t\tdefault: ::error(\"invalid call to %04x dispatched\", (uint16)context.ax);")
		self.fd.write("\n\t}\n}\n\n} /*namespace*/\n")

		self.fd.close()
