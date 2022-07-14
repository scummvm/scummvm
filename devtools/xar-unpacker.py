import io
import sys
import struct
import os
import zlib
import xml
import xml.etree.ElementTree as ET

class XarError(Exception):
	pass

class XarHeader:
	def __init__(self, checksum_algo, toc_data, heap_start):
		self.checksum_algo = checksum_algo
		self.toc_data = toc_data
		self.heap_start = heap_start

class XarDirectory:
	def __init__(self, name, children):
		self.name = name
		self.children = children

	def find(self, name):
		for ch in self.children:
			if ch.name == name:
				return ch
		return None


class XarFile:
	def __init__(self, name, compression, compressed_size, uncompressed_size, offset):
		self.name = name
		self.compression = compression
		self.compressed_size = compressed_size
		self.uncompressed_size = uncompressed_size
		self.offset = offset

def read_xar_header(f):
	header_bytes = f.read(28)
	(magic, header_size, version, toc_size_compressed, toc_size_uncompressed, checksum_algo) = struct.unpack(">IHHQQI", header_bytes)
	
	if header_size < 28:
		raise XarError("Header size field was too small")
	f.seek(header_size - 28, os.SEEK_CUR)
	toc_data = f.read(toc_size_compressed)

	return XarHeader(checksum_algo, zlib.decompress(toc_data), header_size + toc_size_compressed)


def parse_xar_abstract_file(f, force_directory):
	children = []
	type_element = None
	name_element = None
	data_element = None
	
	for ch in f:
		if isinstance(ch, ET.Element):
			if ch.tag == "file":
				children.append(parse_xar_abstract_file(ch, False))
			elif ch.tag == "type":
				type_element = ch
			elif ch.tag == "name":
				name_element = ch
			elif ch.tag == "data":
				data_element = ch

	if force_directory:
		return XarDirectory(".", children)

	if type_element is None:
		raise XarError("Unknown element type")

	f_type = type_element.text
	f_name = name_element.text
	if f_type == "file":
		if data_element is None:
			raise XarError("No data element for file")

		encoding_element = None
		length_element = None
		size_element = None
		offset_element = None
		for ch in data_element:
			if isinstance(ch, ET.Element):
				if ch.tag == "encoding":
					encoding_element = ch
				elif ch.tag == "length":
					length_element = ch
				elif ch.tag == "size":
					size_element = ch
				elif ch.tag == "offset":
					offset_element = ch
	
		f_encoding = "zlib"
		if encoding_element is not None:
			enc_mime = encoding_element.attrib["style"]
			if enc_mime == "application/x-lzma":
				f_encoding = "lzma"
			elif enc_mime == "application/octet-stream":
				f_encoding = "none"
			elif enc_mime == "application/x-bzip2":
				f_encoding = "bzip2"
			else:
				raise XarError("Unknown encoding " + str(enc_mime))

		return XarFile(f_name, f_encoding, int(length_element.text), int(size_element.text), int(offset_element.text))
	elif f_type == "directory":
		return XarDirectory(f_name, children)
	else:
		raise XarError("Unknown element type")

def parse_xar_toc(toc_data):
	tree = ET.fromstring(toc_data.decode("utf-8"))
	toc = tree.find("toc")
	return parse_xar_abstract_file(toc, True)

def print_xar_tree(f, indent=''):
	if isinstance(f, XarDirectory):
		indent = indent + "    "
		for ch in f.children:
			print_xar_tree(ch, indent)
	elif isinstance(f, XarFile):
		print(indent + f.name + "   Size " + str(f.compressed_size) + "->" + str(f.uncompressed_size) + "  Pos " + str(f.offset) + "  Compr " + f.compression)
	else:
		raise XarError("print_xar_tree called with a value of invalid type")

def decompress_copy(in_f, comp_size, uncomp_size, out_f):
	if comp_size != uncomp_size:
		raise XarError("Uncompressed file had different compressed and uncompressed sizes")

	chunk_size = 32768
	remaining = uncomp_size
	while remaining > 0:
		if remaining < chunk_size:
			chunk_size = remaining
		chunk = in_f.read(chunk_size)
		out_f.write(chunk)
		remaining -= chunk_size

def extract_xar_file(xar_f, xar_header, xar_file, out_f):
	xar_f.seek(xar_file.offset + xar_header.heap_start, os.SEEK_SET)

	compressed_size = xar_file.compressed_size
	uncompressed_size = xar_file.uncompressed_size

	decompress_func = None
	if xar_file.compression == "none":
		decompress_func = decompress_copy

	if decompress_func is None:
		raise XarError("No codec available for encoding " + xar_file.compression)

	decompress_func(xar_f, compressed_size, uncompressed_size, out_f)

def print_usage():
	print("XAR unpacker (c)2022 Eric Lasota")
	print("Usage: xar-unpacker.py unpack <input.pkg> <path> <output>")
	print("                       Unpacks one file from a XAR archive")
	print("       xar-unpacker.py list <input.pkg>")
	print("                       Prints the contents of a XAR archive")
	print("       xar-unpacker.py dumptoc <input.pkg> <output>")
	print("                       Dumps the TOC of a XAR archive")

def main(argv):
	if len(argv) < 3:
		print_usage()
		return -1

	operation = argv[1]
	if operation == "unpack":
		if len(argv) != 5:
			print_usage()
			return -1
	elif operation == "list":
		if len(argv) != 3:
			print_usage()
			return -1
	elif operation == "dumptoc":
		if len(argv) != 4:
			print_usage()
			return -1

	in_f_name = argv[2]

	xar_f = io.open(in_f_name, "rb")
	xar_header = read_xar_header(xar_f)

	toc = parse_xar_toc(xar_header.toc_data)

	if operation == "unpack":
		arc_path = argv[3]
		out_f_name = argv[4]
		target_file = toc
		for sub_path in arc_path.split("/"):
			if not isinstance(target_file, XarDirectory):
				raise XarError("Subpath parent is not a directory")

			target_file = target_file.find(sub_path)
			if target_file is None:
				raise XarError("Subpath " + sub_path + " wasn't found")

		print("Extracting " + arc_path + " to " + out_f_name)
		
		out_f = io.open(out_f_name, "wb")

		extract_xar_file(xar_f, xar_header, target_file, out_f)

		out_f.close()
	elif operation == "list":
		print("Dumping TOC from " + str(toc))
		print_xar_tree(toc)
	elif operation == "dumptoc":
		out_f_name = argv[3]
		toc_f = io.open(out_f_name, "wb")
		toc_f.write(xar_header.toc_data)
		toc_f.close()

	xar_f.close()



main(sys.argv)
