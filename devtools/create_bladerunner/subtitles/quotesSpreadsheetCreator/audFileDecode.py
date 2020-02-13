#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
ctypesLibFound = False
structLibFound = False

try:
	import ctypes
except ImportError:
	print "[Error] ctypes python library is required to be installed!"
else:
	ctypesLibFound = True

if 	(not ctypesLibFound):
	sys.stdout.write("[Error] Errors were found when trying to import required python libraries\n")
	sys.exit(1)
	
from struct import *

MY_MODULE_VERSION = "0.60"
MY_MODULE_NAME = "audFileDecode"

aud_ima_index_adjust_table = [-1, -1, -1, -1, 2, 4, 6, 8]

# aud_ima_step_table has 89 entries
aud_ima_step_table = [
	7,	   8,	  9,	 10,	11,	   12,	   13,	  14,	 16,
	17,	   19,	  21,	 23,	25,	   28,	   31,	  34,	 37,
	41,	   45,	  50,	 55,	60,	   66,	   73,	  80,	 88,
	97,	   107,	  118,	 130,	143,   157,	   173,	  190,	 209,
	230,   253,	  279,	 307,	337,   371,	   408,	  449,	 494,
	544,   598,	  658,	 724,	796,   876,	   963,	  1060,	 1166,
	1282,  1411,  1552,	 1707,	1878,  2066,   2272,  2499,	 2749,
	3024,  3327,  3660,	 4026,	4428,  4871,   5358,  5894,	 6484,
	7132,  7845,  8630,	 9493,	10442, 11487,  12635, 13899, 15289,
	16818, 18500, 20350, 22385, 24623, 27086,  29794, 32767 ]

aud_ws_step_table2 = [-2, -1, 0, 1]

aud_ws_step_table4 = [
	-9, -8, -6, -5, -4, -3, -2, -1,
	 0,	 1,	 2,	 3,	 4,	 5,	 6,	 8
]

# (const xccTHA::byte* audio_in, short* audio_out, int& index, int& sample, int cs_chunk)
# index and sample are passed by reference and changed here...
# audio_out is definitely affected!
def aud_decode_ima_chunk(audioBufferIn, index, sample, cs_chunk):
	code = -1
	delta = -1
	step = -1
	
	audioBufferOut = []
	#for i in range(0, len(audioBufferIn)):
	#if self.m_traceModeEnabled:
	#	print '[Debug] %d: %d'%(i, int(audioBufferIn[i]))
	
	for sample_index in range (0, cs_chunk):
		try:
			code = audioBufferIn[sample_index >> 1]
		except:
			code = 0xa9 # dummy workaround because the c code is accessing an out of bounds index sometimes due to this shift here
		#print "[Debug] cs_chunk: %d, sample_index: %d, shifted: %d, code: %d" % (cs_chunk, sample_index, sample_index >> 1, int(audioBufferIn[sample_index >> 1]))
		#print "[Debug] cs_chunk: %s, sample_index: %s, shifted: %s, code: %s" % \
		#		(''.join('{:04X}'.format(cs_chunk)), ''.join('{:02X}'.format(sample_index)), ''.join('{:02X}'.format(sample_index >> 1)), ''.join('{:04X}'.format(int(code))))
		code = code >> 4  if (sample_index & 1) else code & 0xf
		step = aud_ima_step_table[index]
		delta = step >> 3
		if (code & 1):
			delta += step >> 2
		if (code & 2):
			delta += step >> 1
		if (code & 4):
			delta += step
		if (code & 8):
			sample -= delta
			if (sample < -32768):
				sample = -32768
		else:
			sample += delta
			if (sample > 32767):
				sample = 32767
		audioBufferOut.append(ctypes.c_short( sample ).value )
		#audioBufferOut.append(sample) # it's not different from above... ctypes.c_short( sample ).value
		#if self.m_traceModeEnabled:
		#	print "[Debug] audio_out[%s]: %s" % (''.join('{:02X}'.format(sample_index)), ''.join('{:02X}'.format(audioBufferOut[sample_index])));
		index += aud_ima_index_adjust_table[code & 7]
		if (index < 0):
			index = 0
		elif (index > 88):
			index = 88
	## output buffer of shorts
	#binDataOut = struct.pack('h'*len(audioBufferOut), *audioBufferOut)
	#return (binDataOut, index, sample)
	return (audioBufferOut, index, sample)
#
#
#
def aud_decode_clip8(v):
	if (v < 0):
		return 0
	return 0xff if (v > 0xff) else v
#
#
#

# (const xccTHA::byte* r, char* w, int cb_s, int cb_d)
def aud_decode_ws_chunk(inputChunkBuffer, cb_s, cb_d):
	outputChunkBuffer = []
	inpChBuffIter = 0
	outChBuffIter = 0

	if (cb_s == cb_d):
		# outputChunkBuffer = inputChunkBuffer[:cb_s] #	memcpy(w, r, cb_s)  #  FIX
		for mcp in range(0, cb_s):
			outputChunkBuffer.append(ctypes.c_char(inputChunkBuffer[inpChBuffIter + mcp]).value)
		#binDataOut = struct.pack('b'*len(outputChunkBuffer), *outputChunkBuffer)
		#return binDataOut
		return outputChunkBuffer
		
#	const xccTHA::byte* s_end = inputChunkBuffer + cb_s; # FIX

	s_end = inpChBuffIter + cb_s
	sample = ctypes.c_int(0x80).value #int sample
	while (inpChBuffIter < s_end):
		inpChBuffIter += 1
		count = ctypes.c_char(inputChunkBuffer[inpChBuffIter] & 0x3f).value # char count
		switchKey = inputChunkBuffer[inpChBuffIter - 1] >> 6                # inputChunkBuffer[-1] # b[-1] is  *(b - 1)
		if switchKey == 0:
			count += 1
			for iter0 in range (count, 0, -1):
				inpChBuffIter += 1
				code = ctypes.c_int(inputChunkBuffer[inpChBuffIter]).value  # int code
				# assignment in C was right to left so:
				# *(outputChunkBuffer++) = sample = clip8(sample + aud_ws_step_table2[code & 3])
				# is:
				# *(outputChunkBuffer++) = (sample = clip8(sample + aud_ws_step_table2[code & 3]))
				# which is equivalent to these two commands:
				# sample = clip8(sample + aud_ws_step_table2[code & 3])
				# *(outputChunkBuffer++) = sample
				# SO:
				sample = aud_decode_clip8(sample + aud_ws_step_table2[code & 3])
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
				sample = aud_decode_clip8(sample + aud_ws_step_table2[code >> 2 & 3])
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
				sample = aud_decode_clip8(sample + aud_ws_step_table2[code >> 4 & 3])
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
				sample = aud_decode_clip8(sample + aud_ws_step_table2[code >> 6])
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
		elif switchKey == 1:
			count += 1
			for iter0 in range (count, 0, -1):
				inpChBuffIter += 1
				code = inputChunkBuffer[inpChBuffIter] # int code
				sample += aud_ws_step_table4[code & 0xf]
				sample = aud_decode_clip8(sample)
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
				sample += aud_ws_step_table4[code >> 4]
				sample = aud_decode_clip8(sample)
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
		elif switchKey == 2:
			if (count & 0x20):
				#sample += static_cast<char>(count << 3) >> 3
				#*(outputChunkBuffer++) = sample
				sample += ((count & 0xFF) << 3 ) >> 3
				outputChunkBuffer.append(ctypes.c_char(sample).value)
				outChBuffIter += 1
			else:
				count += 1
				# memcpy(outputChunkBuffer, inputChunkBuffer, count) # FIX
				for mcp in range(0, count):
					outputChunkBuffer.append(ctypes.c_char(inputChunkBuffer[inpChBuffIter + mcp]).value)
				inpChBuffIter += count
				outChBuffIter += count
				sample = inputChunkBuffer[inpChBuffIter - 1]
		else:
			count += 1
			# memset(outputChunkBuffer, sample, ++count)
			for mst in range(0, count):
				outputChunkBuffer.append(ctypes.c_char(sample).value)
			outChBuffIter += count;
	# output buffer of chars
	#binDataOut = struct.pack('b'*len(outputChunkBuffer), *outputChunkBuffer)
	#return binDataOut
	return outputChunkBuffer

#
#
#
class audFileDecode(object):
	m_index = -1
	m_sample = -1
	m_traceModeEnabled = False
	
	# traceModeEnabled is bool to enable more printed debug messages
	def __init__(self, traceModeEnabled = True, index = 0, sample = 0):
		self.m_traceModeEnabled = traceModeEnabled
		self.m_index = index;
		self.m_sample = sample;
		return
	
	def index(self):
		return self.m_index
		
	# (const xccTHA::byte* audio_in, short* audio_out, int cs_chunk)
	def decode_chunk(self, audio_in, cs_chunk):
		(audio_Out, outIndex, outSample) = aud_decode_ima_chunk(audio_in, self.m_index, self.m_sample, cs_chunk)
		self.m_index = outIndex
		self.m_sample = outSample
		return audio_Out

if __name__ == '__main__':
	#	 main()
	decodeInstance = audFileDecode()
	if decodeInstance.m_traceModeEnabled:
		print "[Debug] Running %s (%s) as main module" % (MY_MODULE_NAME, MY_MODULE_VERSION)
		
else:
	#debug
	#print "[Debug] Running %s (%s) imported from another module" % (MY_MODULE_NAME, MY_MODULE_VERSION)
	pass
