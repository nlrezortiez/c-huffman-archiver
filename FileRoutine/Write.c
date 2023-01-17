#include "Write.h"

extern FData* fhead;
extern UnitNode* head;

void CutoffBuffer(uint32_t* buffer, const int encode_length) {
	(*buffer) <<= 32 - encode_length;
	(*buffer) >>= 32 - encode_length;
}

void CutoffSingle(uint32_t* buffer, size_t capacity) {
	(*buffer) <<= 32 - (capacity - 4);
	(*buffer) >>= 32 - (capacity - 4);
}


UnitNode* SeekUnit(uint32_t _target) {
	for (UnitNode* current = head; current != NULL; current = current->next) {
		if (current->content == _target) {
			return current;
		}
	}
	return NULL;
}

void BufferHeader(char buffer[BUFSIZ], FILE* ifile_pointer, const uint8_t encode_length) {
	for (FData* current = fhead; current != NULL; current = current->next) {
		if (current->file_pointer == ifile_pointer) {

			char* string_encode_length = (char*) malloc(2);
			char string_filename_size[STRINGBUF];

			sprintf(string_encode_length, "%hhd", current->encode_length);
			sprintf(string_filename_size, "%ld", current->filename_l);
			strcat(buffer, string_encode_length);
			strcat(buffer, "|");
			strcat(buffer, string_filename_size);
			strcat(buffer, "|");
			strcat(buffer, current->filename);
			strcat(buffer, "|");

			char* string_filesize = (char*) malloc(current->filesize_l + 1);
			sprintf(string_filesize, "%ld", current->filesize);
			strcat(buffer, string_filesize);

			strcat(buffer, "|");
			break;
		}
	}
}

void WriteHeader(FILE* ofile_pointer, char header[BUFSIZ], int files_amount) {
	char header_size_string[STRINGBUF] = {}, files_amount_size_string[STRINGBUF] = {};
	sprintf(header_size_string, "%ld", strlen(header));
	sprintf(files_amount_size_string, "%d", files_amount);
	fprintf(ofile_pointer, "%d|%s|%ld%c", files_amount, header, strlen(header) + 3 + strlen(header_size_string) + strlen(files_amount_size_string), EOH);
}

void BufferCodetable(FILE* ofile_pointer, char* buffer) {
	strcat(buffer, "[");
	for (UnitNode* current = head; current != NULL; current = current->next) {
		char current_unit[STRINGBUF] = {};
		sprintf(current_unit, "{%d:%s};", current->content, current->bin_repr_string);
		strcat(buffer, current_unit);
	}
	strcat(buffer, "]");
}

int WriteBody_p(FILE* ofile_pointer, const char* ifilename, uint8_t encode_length) {
	FILE* ifile_pointer = fopen(ifilename, "rb");
	if (ifile_pointer == NULL) {
		return -1;
	}
	
	uint8_t in_buffer[BUFSIZ];
	uint8_t out_buffer[BUFSIZ] = {};
	size_t cur_bit = 0;
	size_t variable_size = encode_length / 8;
	uint8_t variable[variable_size];

	while (!feof(ifile_pointer)) {
		size_t read = fread(in_buffer, 1, BUFSIZ, ifile_pointer);
		size_t last_byte = 0;
		for (size_t i = 0; i + variable_size <= read; i += variable_size, last_byte = i) {
			memcpy(variable, in_buffer + i, variable_size);
			uint32_t unpacked = 0;
			for (size_t j = 0; j < variable_size; j++) {
				unpacked += (variable[j] << ((variable_size - j - 1) * 8));
			}
			UnitNode* unit = SeekUnit(unpacked);
			if (unit == NULL) {
				exit(printf("Error occurred while archiving. Abort.\n"));
			}
			uint8_t* bits = unit->bin_repr_array;
			size_t length = unit->bin_length;
			for (size_t j = 0; j < length; j++) {
				out_buffer[cur_bit >> 3] <<= 1;
				out_buffer[cur_bit >> 3] += bits[j];
				if (++cur_bit == BUFSIZ * 8) {
					fwrite(out_buffer, 1, BUFSIZ, ofile_pointer);
					memset(out_buffer, 0, BUFSIZ);
					cur_bit = 0;
				}
			}
		}
		
		if (last_byte != read) {
			memcpy(variable, in_buffer + last_byte, read - last_byte);
			uint32_t unpacked = 0;
			for (int j = 0; j < read - last_byte; j++) {
				unpacked += (variable[j] << (j * 8));
			}
			unpacked <<= 8;
			UnitNode* unit = SeekUnit(unpacked);
			uint8_t* bits = unit->bin_repr_array;
			size_t length = unit->bin_length;
			for (size_t j = 0; j < length; j++) {
				out_buffer[cur_bit >> 3] <<= 1;
				out_buffer[cur_bit >> 3] += bits[j];
				if (++cur_bit == BUFSIZ * 8) {
					fwrite(out_buffer, 1, BUFSIZ, ofile_pointer);
					memset(out_buffer, 0, BUFSIZ);
					cur_bit = 0;
				}
			}
		}
	}
	
	if (cur_bit != 0) {
		out_buffer[cur_bit >> 3] <<= 8 - (cur_bit % 8);
		fwrite(out_buffer, 1, (cur_bit + 7) >> 3, ofile_pointer);
	}

	fclose(ifile_pointer);
	return 0;
}

int WriteBody_c(FILE* ofile_pointer, const char* ifilename, uint8_t encode_length) {
	FILE* ifile_pointer = fopen(ifilename, "rb");
	if (ifile_pointer == NULL) {
		return -1;
	}

	uint8_t in_buffer[BUFSIZE], out_buffer[BUFSIZE] = {};
	size_t variable_size = (encode_length + 7) / 8;
	uint8_t variable[variable_size];
	size_t iter_counter = 0, cur_bit = 0;

	while (!feof(ifile_pointer)) {
		size_t read = fread(in_buffer, 1, BUFSIZE, ifile_pointer);
		size_t last_byte = 0;
		for (int i = 0; i + variable_size <= read; i += variable_size, last_byte = i) {
			memcpy(variable, in_buffer + i, variable_size);
			uint32_t unpacked = 0;
			for (size_t j = 0; j < variable_size; ++j) {
				unpacked += (variable[j] << ((variable_size -j - 1) * 8));
			}
			if (iter_counter % 2 == 0) {
				unpacked >>= 4;
				--i;
			} else {
				CutoffBuffer(&unpacked, encode_length);
			}
            ++iter_counter;
			UnitNode* unit = SeekUnit(unpacked);
			if (unit == NULL) {
				exit(printf("Error occurred while archiving. Abort.\n"));
			}
			uint8_t* bits = unit->bin_repr_array;
			size_t length = unit->bin_length;
			for (size_t j = 0; j < length; ++j) {
				out_buffer[cur_bit >> 3] <<= 1;
				out_buffer[cur_bit >> 3] += bits[j];
				if (++cur_bit == BUFSIZE * 8) {
					fwrite(out_buffer, 1, BUFSIZE, ofile_pointer);
					memset(out_buffer, 0, BUFSIZE);
					cur_bit = 0;
				}
			}
		}

		if (last_byte != read) {
			memset(variable, 0, variable_size);
			memcpy(variable, in_buffer + last_byte, read - last_byte);
			uint32_t unpacked = 0;
			for (size_t j = 0; j < read - last_byte; ++j) {
				unpacked <<= 8;
				unpacked += variable[j];
			}
			if (iter_counter % 2 == 0 && read - last_byte > 2) {
				unpacked >>= 4;
			} else if (iter_counter % 2 != 0){
				CutoffSingle(&unpacked, (read - last_byte) * 8);
				unpacked <<= encode_length - ((read - last_byte) * 8) + 4; 	// godlike decision here
			} else if (iter_counter % 2 == 0 && read - last_byte <= 2) {
				unpacked <<= encode_length - ((read - last_byte) * 8);		// and here
			}
			UnitNode* unit = SeekUnit(unpacked);
			if (unit == NULL) {
				exit(printf("Error occurred while archiving. Abort.\n"));
			}
			uint8_t* bits = unit->bin_repr_array;
			size_t length = unit->bin_length;
			for (size_t j = 0; j < length; ++j) {
				out_buffer[cur_bit >> 3] <<= 1;
				out_buffer[cur_bit >> 3] += bits[j];
				if (++cur_bit == BUFSIZE * 8) {
					fwrite(out_buffer, 1, BUFSIZE, ofile_pointer);
					memset(out_buffer, 0, BUFSIZE);
					cur_bit = 0;
				}
			}
		}
	}	

	if (cur_bit != 0) {
		out_buffer[cur_bit >> 3] <<= 8 - (cur_bit % 8);
		fwrite(out_buffer, 1, (cur_bit + 7) >> 3, ofile_pointer);
	}

	fclose(ifile_pointer);
	return 0;
}

void WriteFile(const char* output_file,
			   char header[BUFSIZ],
			   char* filenames[],
			   int files_amount,
			   uint8_t encode_length) {
	FILE* ofile_pointer = fopen(output_file, "wb+");

	BufferCodetable(ofile_pointer, header);
	WriteHeader(ofile_pointer, header, files_amount);
	
	if (encode_length % BYTE_LENGTH == 0) {
		for (int i = 0; i < files_amount; ++i) {
			WriteBody_p(ofile_pointer, filenames[i], encode_length);
		}
	} else {
		for (int i = 0; i < files_amount; ++i) {
			WriteBody_c(ofile_pointer, filenames[i], encode_length);
		}
	}

	fclose(ofile_pointer);
}