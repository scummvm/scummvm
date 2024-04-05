
#ifndef __ZIP_HEADERS_H__
#define __ZIP_HEADERS_H__

typedef unsigned char byte;
typedef long longint;
typedef unsigned short word;

typedef longint       signature_type;

#define local_file_header_signature  0x04034b50L

struct local_file_header { 
	word         version_needed_to_extract; 
	word         general_purpose_bit_flag; 
	word         compression_method; 
	word         last_mod_file_time; 
	word         last_mod_file_date; 
	longint      crc32; 
	longint      compressed_size; 
	longint      uncompressed_size; 
	word         filename_length; 
	word         extra_field_length; 
}; 

#define central_file_header_signature  0x02014b50L

struct central_directory_file_header { 
	word         version_made_by; 
	word         version_needed_to_extract; 
	word         general_purpose_bit_flag; 
	word         compression_method; 
	word         last_mod_file_time; 
	word         last_mod_file_date; 
	longint      crc32; 
	longint      compressed_size; 
	longint      uncompressed_size; 
	word         filename_length; 
	word         extra_field_length; 
	word         file_comment_length; 
	word         disk_number_start; 
	word         internal_file_attributes; 
	longint      external_file_attributes; 
	longint      relative_offset_local_header; 
}; 

#define end_central_dir_signature  0x06054b50L

struct end_central_dir_record { 
	word         number_this_disk; 
	word         number_disk_with_start_central_directory; 
	word         total_entries_central_dir_on_this_disk; 
	word         total_entries_central_dir; 
	longint      size_central_directory; 
	longint      offset_start_central_directory; 
	word         zipfile_comment_length; 
}; 

#endif // __ZIP_HEADERS_H__
