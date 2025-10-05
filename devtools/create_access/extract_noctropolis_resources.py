#!/usr/bin/env python3
import sys
import struct

"""
The Noctropolis resources were recompiled with 32 bit ints, so we can't just
extract the data and read it with our code as-is.  Instead, we pull out the
individual bits and print them out to go into CPP files.
"""


def read_c_string(f, offset=None):
	"""Read a null-terminated string from file starting at offset."""
	if offset is not None:
		f.seek(offset)
	chars = []
	while True:
		b = f.read(1)
		if not b or b == b"\x00":
			break
		chars.append(b.decode("latin1"))  # latin1 avoids decode errors
	return "".join(chars)


def read_one_pointer(f):
	data = f.read(8)
	if len(data) < 8:
		return 0
	(value,) = struct.unpack("<Q", data)
	# strip off the segment
	value &= 0xFFFFFF
	return value


def read_pointer_list(f, count=None):
	"""
	Read a list of pointers (64-bit) from the current file offset.
	Read count pointers (if set) or until a null pointer is hit otherwise.
	"""
	offsets = []
	# Read 64-bit little-endian integers until 0 or count
	while True:
		value = read_one_pointer(f)
		if count is None and value == 0:
			break
		offsets.append(value)
		if count is not None:
			count -= 1
			if count == 0:
				break
	return offsets


# read a file/subfile pair and as shorts
def file_ints_to_shorts(f):
	"""
	Read the file/subfile pair of int32s and recode them as int16
	"""
	data = f.read(8)
	file, subfile = struct.unpack("<ii", data)
	return struct.pack("<hh", file, subfile)


def read_room_cell_data(f):
	alldata = b""
	while True:
		data = f.read(4)
		alldata += data[0:1]  # convert int32 -> byte for cell num
		(cell,) = struct.unpack("<I", data)
		if cell == 0xFF:
			return alldata
		alldata += file_ints_to_shorts(f)


def read_room_vid_data(f):
	"""
	Read the video data table for the room data
	"""
	offsets = read_pointer_list(f)

	if not offsets:
		return b"\0"

	# Collect strings from those offsets
	strings = [read_c_string(f, off) for off in offsets]
	asbytes = [x.encode("utf-8") for x in strings]

	result = b"\0".join(asbytes) + b"\0\0"
	return result


def read_room_sound_data(f):
	"""
	Read the sound list for room data.  Each sound entry is 24 bytes:
	 * file/subfile/priority int32s and 12 bytes of null
	 * if the file is -1, it's the end of the list
	 * if the priority is 0xff, the last 8 bytes can be a pointer to a file name.

	 convert that to a more compact and similar-to-others format:
	 * if file is 0xff -> read a file name string, else read subfile
	 * write ints as int16
	"""
	alldata = b""
	while True:
		data = f.read(8)
		file, subfile = struct.unpack("<ii", data)
		# print(f"{file}, {subfile}")
		if file == -1:
			alldata += struct.pack("<h", file)
			return alldata

		priodata = f.read(4)
		(priority,) = struct.unpack("<I", priodata)

		_dummy = f.read(4)
		if priority == 0xFF:
			strptr = read_one_pointer(f)
			if strptr != 0:
				curpos = f.tell()
				filename = read_c_string(f, strptr)
				alldata += struct.pack("<H", 0xFF)
				alldata += filename.encode("utf-8")
				alldata += b"\0"
				# print(f"Read string {filename} from 0x%x" % strptr)
				f.seek(curpos)
			else:
				alldata += struct.pack("<HH", file, subfile)
		else:
			f.read(8)  # null str ptr
			alldata += struct.pack("<HH", file, subfile)

		alldata += struct.pack("<H", priority)


def print_data_as_c_array(data):
	for o in range((len(data) + 15) // 16):
		subdata = data[16 * o : 16 * (o + 1)]
		subdata_str = ["0x%02x," % v for v in subdata]
		result = " ".join(subdata_str)
		print("\t" + result)


def read_room_data(f, i: int, offset: int):
	f.seek(offset)
	data = f.read(2)  # roomflag and pal intensity
	f.read(2)  # unused
	data += file_ints_to_shorts(f)  # music file/subfile
	data += f.read(3)  # scale vals
	f.read(1)  # unused
	data += file_ints_to_shorts(f)  # playfield file/subfile
	cellptr = read_one_pointer(f)
	if cellptr == 0:
		data += bytes([0xFF])
	else:
		curpos = f.tell()
		f.seek(cellptr)
		data += read_room_cell_data(f)
		f.seek(curpos)

	data += file_ints_to_shorts(f)  # script file/subfile
	data += file_ints_to_shorts(f)  # anim file/subfile
	data += f.read(1)  # scaleI
	f.read(1)  # unused
	(scrollThreshold,) = struct.unpack("<h", f.read(2))
	assert scrollThreshold <= 0xFF
	data += struct.pack("<B", scrollThreshold)

	f.read(12)  # Skip palette file nums (unused) and 4 bytes after

	videoptr = read_one_pointer(f)
	if videoptr:
		curpos = f.tell()
		f.seek(videoptr)
		data += read_room_vid_data(f)
		f.seek(curpos)
	else:
		data += b"\0"

	soundptr = read_one_pointer(f)
	if soundptr:
		curpos = f.tell()
		f.seek(soundptr)
		# print("Read sounds from 0x%x" % soundptr)
		data += read_room_sound_data(f)
		f.seek(curpos)
	else:
		data += struct.pack("<h", -1)

	print("static const byte ROOMDATA_%02d[] = {" % i)
	print_data_as_c_array(data)
	print("};")


def read_rooms(f):
	f.seek(0x9F400)
	offsets = read_pointer_list(f, count=100)

	for i, offset in enumerate(offsets):
		if offset != 0:
			read_room_data(f, i, offset)

	print("")
	print("")
	for i, offset in enumerate(offsets):
		if offset == 0:
			print(
				'\tROOMTBL.push_back(RoomEntry({"", Common::Point(), Common::Array<byte>()}));'
			)
		else:
			print(
				'\tROOMTBL.push_back(RoomEntry({"", Common::Point(), Common::Array<byte>(ROOMDATA_%02d, ARRAYSIZE(ROOMDATA_%02d))}));'
				% (i, i)
			)


def read_filenames(f):
	base_offset = 0x94830
	f.seek(base_offset)

	offsets = read_pointer_list(f)

	# Collect strings from those offsets
	strings = [read_c_string(f, off) for off in offsets]

	print("static const char *FILENAMES[] = {")
	for s in strings:
		escaped = s.replace("\\", "\\\\").replace('"', '\\"')
		print(f'\t"{escaped}",')
	print("};")


def read_mouse_cursor(f):
	hotdata = f.read(4)
	hotx, hoty = struct.unpack("<HH", hotdata)
	width, height = struct.unpack("<HH", f.read(4))
	gfxptr = read_one_pointer(f)
	_texptr = read_one_pointer(f)

	f.seek(gfxptr)
	gfxdata = f.read(width * height)

	print(
		"\t0x%02x, 0x%02x, 0x%02x, 0x%02x,   // hot x,y"
		% (hotdata[0], hotdata[1], hotdata[2], hotdata[3])
	)
	print_data_as_c_array(gfxdata)
	pass


def read_mouse_cursors(f):
	base_offset = 0x95030
	f.seek(base_offset)

	offsets = read_pointer_list(f)

	for i, pointer in enumerate(offsets):
		print("static const byte CURSOR_%d_DATA[] = {" % i)
		f.seek(pointer)
		read_mouse_cursor(f)
		print("};")


def main():
	if len(sys.argv) < 2:
		print(f"Usage: {sys.argv[0]} Macos-Noctropolis-Executable")
		return
	filename = sys.argv[1]

	with open(filename, "rb") as f:
		# read_filenames(f)
		read_rooms(f)
		#read_mouse_cursors(f)


if __name__ == "__main__":
	main()
