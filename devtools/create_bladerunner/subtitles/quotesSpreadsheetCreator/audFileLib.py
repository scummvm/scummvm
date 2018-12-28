#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-
#
import os, sys

shutilLibFound = False
waveLibFound = False
ctypesLibFound = False
structLibFound = False

try:
	import shutil 
except ImportError:
	print "Error:: Shutil python library is required to be installed!" 
else:
	shutilLibFound = True
	
try:
	import wave
except ImportError:
	print "Error:: Wave python library is required to be installed!" 
else:
	waveLibFound = True

try:
	import ctypes 
except ImportError:
	print "Error:: ctypes python library is required to be installed!" 
else:
	ctypesLibFound = True

try:
	import struct 
except ImportError:
	print "Error:: struct python library is required to be installed!" 
else:
	structLibFound = True

if 	(not shutilLibFound) or (not waveLibFound) or (not ctypesLibFound) or (not structLibFound):
	sys.stdout.write("Error:: Errors were found when trying to import required python libraries\n")
	sys.exit(1)	

from struct import *
from audFileDecode import *

my_module_version = "0.50"
my_module_name = "audFileLib"

#constants
aud_chunk_id = 0x0000deaf
SIZE_OF_AUD_HEADER_IN_BYTES = 12
SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES = 8

class AudHeader:
	samplerate = -1	 #// Frequency // int16_t  // TODO should be unsigned (?)
	size_in = -1	 #// Size of file (without header) // int32_t	// TODO should be unsigned (?)
	size_out = -1	 #// Size of output data // int32_t	  // TODO should be unsigned (?)
	flags = -1		 #// bit 0=stereo, bit 1=16bit // int8_t 
	compression = -1 #// 1=WW compressed, 99=IMA ADPCM (0x63) // int8_t 

	def __init__(self):
		return
	
	
#//The rest of the AUD files is divided in chunks. These are usually 512
#//bytes long, except for the last one.
class AudChunkHeader:
	size_in = -1		#// Size of compressed data // int16_t	// TODO should be unsigned (?)
	size_out = -1		#// Size of output data 	// int16_t	// TODO should be unsigned (?)
	id = 0x0000FFFF		#// Always 0x0000DEAF		// int32_t
	
	def __init__(self):
		return
	
#
#
#
class audFile:
	m_header = AudHeader()
	def __init__(self):
		return
	
	# std::fstream& fs, AudFileNS::pos_type startAudFilepos, AudFileNS::pos_type endAudFilepos, const std::string& filename
	def extract_as_wav(self, audBytesBuff, filename):
		print "Saving to wav: " + filename

		cvirtualBinaryD = self.decode(audBytesBuff)
#		TODO DEBUG REMOVED	FOR NOW. TODO RESTORE THIS!!!
#		if (not cvirtualBinaryD):
#			return 1
		
		cb_sample = self.get_cb_sample()
		cs_remaining = self.get_c_samples()
		
		waveWritFile = wave.open(filename, 'wb')
		waveWritFile.setnchannels(self.get_c_channels())
		waveWritFile.setsampwidth(cb_sample)
		waveWritFile.setframerate(self.get_samplerate())
		waveWritFile.setnframes(cs_remaining)
		#waveWritFile.setcomptype(None, '')
		waveWritFile.writeframesraw(cvirtualBinaryD)
		waveWritFile.close()
		
#		t_wav_header header;
#		memset(&header, 0, sizeof(t_wav_header));
#		header.file_header.id = wav_file_id; // # "RIFF"
#		header.file_header.size = sizeof(header) - sizeof(header.file_header) + (cs_remaining << 1);
#		header.form_type = wav_form_id;		// # "WAVE"
#		header.format_chunk.header.id = wav_format_id;	// #"fmt "
#		header.format_chunk.header.size = sizeof(header.format_chunk) - sizeof(header.format_chunk.header);
#		header.format_chunk.formattag = 1;
#		header.format_chunk.c_channels = 1;
#		header.format_chunk.samplerate = get_samplerate();
#		header.format_chunk.byterate =	cb_sample * get_samplerate();
#		header.format_chunk.blockalign = cb_sample;
#		header.format_chunk.cbits_sample = cb_sample << 3;
#		header.data_chunk_header.id = wav_data_id;		# "data"
#		header.data_chunk_header.size = cb_sample * cs_remaining;
#		error = f.write(&header, sizeof(t_wav_header));
#		return error ? error : f.write(d);	
		return 0	# TODO fix
		
		
	def loadAudFile(self, audBytesBuff, maxLength):
		offsInAudFile = 0
		tmpTuple = struct.unpack_from('h', audBytesBuff, offsInAudFile)
		self.header().samplerate = tmpTuple[0]
		offsInAudFile += 2
		tmpTuple = struct.unpack_from('i', audBytesBuff, offsInAudFile)
		self.header().size_in = tmpTuple[0]
		offsInAudFile += 4
		tmpTuple = struct.unpack_from('i', audBytesBuff, offsInAudFile)
		self.header().size_out = tmpTuple[0]
		offsInAudFile += 4
		tmpTuple = struct.unpack_from('b', audBytesBuff, offsInAudFile)
		self.header().flags = tmpTuple[0]
		offsInAudFile += 1
		tmpTuple = struct.unpack_from('b', audBytesBuff, offsInAudFile)
		self.header().compression = tmpTuple[0]
		offsInAudFile += 1
		print "samplerate: %d\tsizeIn: %d\tsizeOut: %d\tflags: %d\tcompression: %d" % (self.get_samplerate(), self.header().size_in, self.header().size_out, self.header().flags, self.header().compression)
		
		if self.get_samplerate() < 8000 or self.get_samplerate() > 48000 or self.header().size_in > (maxLength - SIZE_OF_AUD_HEADER_IN_BYTES ):
			print "AUD HEADER SIZE ERROR::2"
			return False
		else:
			if self.header().compression == 1:
				if (self.header().flags != 0):
					return False
			elif self.header().compression == 0x63:
				if (self.header().flags != 2):
					return False
		return True

	# int AudFile::get_chunk_header(int i, std::fstream& fs, AudFileNS::pos_type startAudFilepos, AudFileNS::pos_type endAudFilepos, AudChunkHeader& outAudChunkHeader)	
	def get_chunk_header(self, chunkIdx, inAudFileBytesBuffer, inAudFileSize ):
		#fs.seekg(int(startAudFilepos) + int(SIZE_OF_AUD_HEADER_IN_BYTES), fs.beg);
		#AudFileNS::pos_type rAudPos;
		#rAudPos = fs.tellg();
		outAudChunkHeader =  AudChunkHeader()
		rAudPos = SIZE_OF_AUD_HEADER_IN_BYTES
		
		#AudChunkHeader  tmpInremediateChunkheader;
		tmpInremediateChunkheader = AudChunkHeader()
		#while (i--)			# value of i is decreased after checked by while loop
		while(chunkIdx > 0):
			chunkIdx -= 1
			if (rAudPos + SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES > inAudFileSize):
				return (-1, rAudPos, None)
				
			tmpAudFileOffset = rAudPos
			tmpTuple = struct.unpack_from('h', inAudFileBytesBuffer, tmpAudFileOffset)
			tmpInremediateChunkheader.size_in = tmpTuple[0]
			tmpAudFileOffset += 2
			tmpTuple = struct.unpack_from('h', inAudFileBytesBuffer, tmpAudFileOffset)
			tmpInremediateChunkheader.size_out = tmpTuple[0]
			tmpAudFileOffset += 2
			tmpTuple = struct.unpack_from('i', inAudFileBytesBuffer, tmpAudFileOffset)
			tmpInremediateChunkheader.id  = tmpTuple[0]
			tmpAudFileOffset += 4	
			#fs.read((char*)&tmpInremediateChunkheader, SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES);
			rAudPos +=  SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES + tmpInremediateChunkheader.size_in
			#fs.seekg(int(rAudPos), fs.beg);
			
		if (rAudPos + SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES > inAudFileSize ):
			return (-1, rAudPos, None)
		# write to FINAL output chunk header
		tmpAudFileOffset = rAudPos
		tmpTuple = struct.unpack_from('h', inAudFileBytesBuffer, tmpAudFileOffset)
		outAudChunkHeader.size_in = tmpTuple[0]
		tmpAudFileOffset += 2
		tmpTuple = struct.unpack_from('h', inAudFileBytesBuffer, tmpAudFileOffset)
		outAudChunkHeader.size_out = tmpTuple[0]
		tmpAudFileOffset += 2
		tmpTuple = struct.unpack_from('i', inAudFileBytesBuffer, tmpAudFileOffset)
		outAudChunkHeader.id  = tmpTuple[0]
		tmpAudFileOffset += 4
		#fs.read((char*)&outAudChunkHeader, SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES);
		if (rAudPos + SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES + outAudChunkHeader.size_in > inAudFileSize):
			return (-1, rAudPos, None)
		rAudPos += SIZE_OF_AUD_CHUNK_HEADER_IN_BYTES
		return (0, rAudPos, outAudChunkHeader) # //reinterpret_cast<const AudChunkHeader*>(r);

	# int AudFile::get_chunk_data(int i, std::fstream& fs, int sizeToRead, AudFileNS::byte* byteChunkDataPtr)
	def get_chunk_data(self, inAudFileBytesBuffer, startOffs, sizeToRead):
		#fs.read((char*)byteChunkDataPtr, sizeToRead)
		outChunkDataLst = []
		#print "startOffs %d, sizeToRead %d" % (startOffs, sizeToRead)
		for i in range(startOffs, startOffs + sizeToRead):
			#outChunkDataLst.append(ctypes.c_char(inAudFileBytesBuffer[i]).value)
			#outChunkDataLst.append(ctypes.c_byte(inAudFileBytesBuffer[i]).value)
			tmpTuple = struct.unpack_from('b', inAudFileBytesBuffer, i)
			outChunkDataLst.append(tmpTuple[0])
		#byteChunkDataOut = struct.pack('b'*len(outChunkDataLst), *outChunkDataLst)
		#return (0, byteChunkDataOut)
		return (0, outChunkDataLst)
		
		
	# std::fstream& fs, AudFileNS::pos_type startAudFilepos, AudFileNS::pos_type endAudFilepos
	# returned Cvirtual_binary
	def decode(self, audBytesBuff):
		# The * operator unpacks an argument list. It allows you to call a function with the list items as individual arguments.
		# binDataOut = struct.pack('i'*len(data), *data)
		print "DECODING..."
#		Cvirtual_binary d;
		binaryDataOutLst = []
		binaryDataOutBuff = None
		cb_audio = self.get_cb_sample() * self.get_c_samples() 	# int cb_audio - basically this should be the size_out
		if self.header().compression == 1:
			# write_start allocates space for virtualBinary
			# AudFileNS::byte* w = d.write_start(cb_audio);
			errGetChunk = 0 									# int errGetChunk
			#for (int chunk_i = 0; w != d.data_end(); chunk_i++)
			chunk_i = 0
			wIndex = 0
			while (wIndex < cb_audio):
				#AudChunkHeader out_chunk_header;																	
				#out_chunk_header = AudChunkHeader()
				(errGetChunk, bufferDataPos, out_chunk_header) = self.get_chunk_header(chunk_i, audBytesBuff, len(audBytesBuff)) 	
				if errGetChunk != 0:
#					print "Error OR End file case while getting uncompressed chunk header!"
					break
				#print "Get uncompressed chunk header returned:: %d " % (out_chunk_header.id)
				#Cvirtual_binary out_chunk_data;
				#AudFileNS::byte* byteChunkDataPtr = out_chunk_data.write_start(out_chunk_header.size_in);
				(errorGCD, byteChunkDataLst) = self.get_chunk_data(audBytesBuff, bufferDataPos, out_chunk_header.size_in)
				# export decoded chunk to w (output) buffer (of CHARS) at the point where we're currently at (so append there)
				decodedAudioChunkAsLst = aud_decode_ws_chunk(byteChunkDataLst, out_chunk_header.size_in, out_chunk_header.size_out)
				binaryDataOutLst.extend(decodedAudioChunkAsLst)
				wIndex += out_chunk_header.size_out
				chunk_i += 1
			binaryDataOutBuff = struct.pack('b'*len(binaryDataOutLst), *binaryDataOutLst)
		elif self.header().compression == 0x63:
			decodeInstance = audFileDecode();
			#decodeInstance.init();
			#AudFileNS::byte* w = d.write_start(cb_audio);
			errGetChunk = 0											# int errGetChunk
			# for (int chunk_i = 0; w != d.data_end(); chunk_i++)
			chunk_i = 0
			wIndex = 0
			while (wIndex < cb_audio):
				#print("chunkI: %d\t Windex: %d\t cb_audio: %d") % (chunk_i,wIndex,cb_audio)
				#AudChunkHeader out_chunk_header;																
				#out_chunk_header = AudChunkHeader()
				#errGetChunk = self.get_chunk_header(chunk_i, fs, startAudFilepos, endAudFilepos, out_chunk_header);
				(errGetChunk, bufferDataPos, out_chunk_header) = self.get_chunk_header(chunk_i, audBytesBuff, len(audBytesBuff))
				if errGetChunk != 0:
					print "Error OR End file case while getting COMPRESSED chunk header!"
					break
				#print "Get COMPRESSED chunk header returned:: headerInSize: %d headerOutSize: %d id: %d" % (out_chunk_header.size_in, out_chunk_header.size_out,  out_chunk_header.id) 
				#Cvirtual_binary out_chunk_data;
				#AudFileNS::byte* byteChunkDataPtr = out_chunk_data.write_start(out_chunk_header.size_in);
				(errorGCD, byteChunkDataLst) = self.get_chunk_data(audBytesBuff, bufferDataPos, out_chunk_header.size_in)
				# export decoded chunk to w (output) buffer (of SHORTS) at the point where we're currently at (so append there)
				#print "byteChunkDataLst len: %d, size_in was: %d" % (len(byteChunkDataLst), out_chunk_header.size_in)
				decodedAudioChunkAsLst = decodeInstance.decode_chunk(byteChunkDataLst, out_chunk_header.size_out / self.get_cb_sample());
				binaryDataOutLst.extend(decodedAudioChunkAsLst)
				wIndex += out_chunk_header.size_out
				#print("new Windex: %d\t cb_audio: %d") % (wIndex,cb_audio)
				chunk_i += 1
			binaryDataOutBuff = struct.pack('h'*len(binaryDataOutLst), *binaryDataOutLst)
		return binaryDataOutBuff

	def header(self):
		return self.m_header

	def	get_c_samples(self):
		return self.m_header.size_out / self.get_cb_sample()

	def get_samplerate(self):
		return self.m_header.samplerate;

	# flag bit 0 is stereo(set) mono(clear)
	def	get_c_channels(self):
		return 2 if (self.m_header.flags & 0x01) else 1;

	# flag bit 1 is 16bit(set) 8bit (clear)	
	def get_cb_sample(self):
		return 2 if (self.m_header.flags & 0x02) else 1
#
#
#		
if __name__ == '__main__':
	#	 main()
	print "Running %s as main module" % (my_module_name)
	# assumes a file of name 000000.AUD in same directory
	inAUDFile = None
	errorFound = False
	try:
		inAUDFile = open(os.path.join('.','00000000.AUD'), 'rb')
	except:
		errorFound = True
		print "Unexpected error:", sys.exc_info()[0]
		raise
	if not errorFound:	
		allOfAudFileInBuffer = inAUDFile.read()
		audFileInstance = audFile()
		audFileInstance.loadAudFile(allOfAudFileInBuffer, len(allOfAudFileInBuffer))
		audFileInstance.extract_as_wav(allOfAudFileInBuffer, './tmp.wav')
		inAUDFile.close()
else:
	#debug
	#print "Running	 %s imported from another module" % (my_module_name)
	pass