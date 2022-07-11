#!/usr/bin/env python3
import sys
import itertools
import io

hfs_block_size = 512
window_size = 65536

class DecompressionError(Exception):
	pass

def block_copy(dest, dest_offset, src, src_offset, size):
	if size == 0:
		return
	dest[dest_offset:dest_offset+size] = src[src_offset:src_offset+size]


# Inserts bytes into sliding window ring buffer, returns new window position
def insert_sl(sl, sl_pos, bytes_to_insert, insert_src_offset, size):
	available = window_size - sl_pos
	if available < size:
		block_copy(sl, sl_pos, bytes_to_insert, insert_src_offset, available)
		sl_pos = 0
		sl_pos = insert_sl(sl, sl_pos, bytes_to_insert, insert_src_offset + available, size - available)
	else:
		block_copy(sl, sl_pos, bytes_to_insert, insert_src_offset, size)
		sl_pos = sl_pos + size

	return sl_pos

# Reads bytes from sliding window ring buffer
def read_sl(sl, sl_pos, out_buf, out_buf_pos, size):
	available = window_size - sl_pos
	if available < size:
		block_copy(out_buf, out_buf_pos, sl, sl_pos, available)
		read_sl(sl, 0, out_buf, out_buf_pos + available, size - available)
	else:
		block_copy(out_buf, out_buf_pos, sl, sl_pos, size)

def read_lz(sl, sl_pos, out_buf, out_buf_pos, coded_offset, length):
	actual_offset = coded_offset + 1

	read_pos = (sl_pos + window_size - actual_offset) % window_size;

	while actual_offset < length:
		# Repeating sequence
		read_sl(sl, read_pos, out_buf, out_buf_pos, actual_offset)
		out_buf_pos += actual_offset
		length -= actual_offset

	# Copy
	read_sl(sl, read_pos, out_buf, out_buf_pos, length)

def decompress(in_f, out_f, compressed_data_size):
	sl = bytearray(window_size)
	lz_bytes = bytearray(128)
	sl_pos = 0
	chunk_size = 0
	output_data = 0

	while compressed_data_size > 0:
		code_byte_0 = in_f.read(1)[0]

		compressed_data_size -= 1

		if (code_byte_0 & 0x80):
			# Literal
			chunk_size = (code_byte_0 & 0x7f) + 1

			if chunk_size > compressed_data_size:
				raise DecompressionError()

			output_data = in_f.read(chunk_size)

			compressed_data_size -= chunk_size
		elif (code_byte_0 & 0x40):
			# Large offset
			if compressed_data_size < 2:
				raise DecompressionError()

			code_bytes_12 = in_f.read(2)

			compressed_data_size -= 2

			chunk_size = (code_byte_0 & 0x3f) + 4
			coded_offset = (code_bytes_12[0] << 8) + code_bytes_12[1]

			read_lz(sl, sl_pos, lz_bytes, 0, coded_offset, chunk_size)
			output_data = lz_bytes
		else:
			# Small offset
			if compressed_data_size < 1:
				raise DecompressionError()

			code_byte_1 = in_f.read(1)[0]

			compressed_data_size -= 1

			chunk_size = ((code_byte_0 & 0x3c) >> 2) + 3
			coded_offset = ((code_byte_0 & 0x3) << 8) + code_byte_1

			read_lz(sl, sl_pos, lz_bytes, 0, coded_offset, chunk_size)
			output_data = lz_bytes

		out_f.write(output_data[0:chunk_size])
		new_offset = out_f.tell()

		sl_pos = insert_sl(sl, sl_pos, output_data, 0, chunk_size)

def bulk_copy(in_f, out_f, amount):
	chunk_size = 4096
	while (amount > 0):
		block_amount = chunk_size
		if (block_amount > amount):
			block_amount = amount

		block = in_f.read(block_amount)
		out_f.write(block)

		amount -= block_amount



def main(argv):
	if (len(argv) != 3):
		print("decompress-image.py: Converts an ADC-compressed Disk Copy 6 image to an uncompressed image")
		print("Usage: decompress-image.py <input> <output>")
		return -1

	in_f = io.open(argv[1], "rb")

	in_f.seek(-hfs_block_size, 2)

	alt_mdb_loc = in_f.tell()

	block_buffer = in_f.read(hfs_block_size)

	if (block_buffer[0] != 66 or block_buffer[1] != 68):
		print("The specified file doesn't look like a disk image")
		return -1

	num_allocation_blocks = (block_buffer[18] << 8) + block_buffer[19]
	allocation_block_size = (block_buffer[20] << 24) + (block_buffer[21] << 16) + (block_buffer[22] << 8) + block_buffer[23]
	first_allocation_block = (block_buffer[28] << 8) + block_buffer[29]

	compressed_data_start = first_allocation_block * allocation_block_size
	compressed_data_end = alt_mdb_loc	# ???

	in_f.seek(0)

	out_f = io.open(argv[2], "wb")

	bulk_copy(in_f, out_f, compressed_data_start)

	compressed_amount = compressed_data_end - compressed_data_start

	decompress(in_f, out_f, compressed_amount)

	in_f.seek(alt_mdb_loc)
	bulk_copy(in_f, out_f, hfs_block_size)

	in_f.close()
	out_f.close()


main(sys.argv)
