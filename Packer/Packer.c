#include "Packer.h"

UnitNode* head = NULL; 

void Pack(char* input_files[], const char* output_file, int files_amount, const uint8_t encode_length, bool is_print_option) {
	//needs to be big enough for large codetables
	//extremly unefficient btw
	char header_buffer[BUFSIZ << 8] = {};

	for (int i = 0; i < files_amount; ++i) {
		FILE* ifile_pointer = fopen(input_files[i], "rb");

		if (ifile_pointer == NULL) {
			printf("Unable to open file %s. Skipping to the next file.\n", input_files[i]);
			continue;
		}

		CollectMetadata(input_files[i], ifile_pointer, encode_length);
		BufferHeader(header_buffer, ifile_pointer, encode_length);

		if (encode_length % BYTE_LENGTH == 0) {
			BuildPreciseAlphabet(ifile_pointer, encode_length);
		} else {
			BuildComplexAlphabet(ifile_pointer, encode_length);
		}
		
		fclose(ifile_pointer);
	}

	size_t list_size;
	if ((list_size = GetListSize()) == 0) {
		exit(printf("Please, note that file(s) is empty, nothing to work with.\n"));
	}

	Compress(head, list_size);
	WriteFile(output_file, header_buffer, input_files, files_amount, encode_length);

	if (is_print_option) {
		PrintUnitFrequencies();
	}

	DeallocateHeap();
	DeallocateFileList();
	DeallocateUnitList();
}

void BuildPreciseAlphabet(FILE* file_pointer, const int ENCODE_LENGTH) {
	uint16_t r_buffer = 0;
	uint8_t buffer[BUFSIZE];
	uint8_t BYTES = ENCODE_LENGTH / BYTE_LENGTH;
	size_t rbyte_counter = 0;

	while (!feof(file_pointer)) {
		size_t read = fread(buffer, 1, BUFSIZE, file_pointer);
		for (int i = 0; i < read; ++i) {
			r_buffer += buffer[i];
			++rbyte_counter;
			if (rbyte_counter % BYTES == 0) {
				UnitNode* found = SeekUnit((uint32_t)r_buffer);
				if (found == NULL) {
					head = PushUnitStructNode(&head, (uint32_t)r_buffer);
				} else {
					++found->frequency;
				}
				r_buffer = 0;
			} else {
				r_buffer <<= BYTE_LENGTH;
			}
		}
	}

	if (r_buffer != 0) {
		UnitNode* found = SeekUnit((uint32_t)r_buffer);
		if (found == NULL) {
			head = PushUnitStructNode(&head, (uint32_t)r_buffer);
		} else {
			++found->frequency;
		}
	}
}

void BuildComplexAlphabet(FILE* file_pointer, const int ENCODE_LENGTH) {
	uint8_t in_buffer[BUFSIZE];
	size_t variable_size = (ENCODE_LENGTH + 7) / 8;
	uint8_t variable[4];
	size_t iter_counter = 0;

	while (!feof(file_pointer)) {
		size_t read = fread(in_buffer, 1, BUFSIZE, file_pointer);
		size_t last_byte = 0;
		for (int i = 0; i + variable_size <= read; i += variable_size, last_byte = i) {
			memcpy(variable, in_buffer + i, variable_size);
			uint32_t unpacked = 0;
			for (size_t j = 0; j < variable_size; ++j) {
				unpacked += (variable[j] << ((variable_size - j - 1) * 8));
			}
			if (iter_counter % 2 == 0) {
				unpacked >>= 4;
				--i;
			} else {
				CutoffBuffer(&unpacked, ENCODE_LENGTH);
			}
			++iter_counter;
			UnitNode* unit = SeekUnit(unpacked);
			if (unit == NULL) {
				head = PushUnitStructNode(&head, unpacked);
			} else {
				++unit->frequency;
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
			} else if (iter_counter % 2 != 0) {
				CutoffSingle(&unpacked, (read - last_byte) * 8);
				unpacked <<= ENCODE_LENGTH - ((read - last_byte) * 8) + 4;
			} else if (iter_counter % 2 == 0 && read - last_byte <= 2) {
				unpacked <<= ENCODE_LENGTH - ((read - last_byte) * 8);
			}
			++iter_counter;
			UnitNode* unit = SeekUnit(unpacked);
			if (unit == NULL) {
				head = PushUnitStructNode(&head, unpacked);
			} else {
				++unit->frequency;
			}
		}
	}
}

UnitNode* PushUnitStructNode(UnitNode** head_reference, uint32_t content) {
    UnitNode* node = (UnitNode*)malloc(sizeof(UnitNode));

    node->content = content;
    node->frequency = 1;
    node->next = (*head_reference);
    node->left = node->right = NULL;

    (*head_reference) = node;
    return node;
}

void PrintHuffmanCodes() {
	for (UnitNode* current = head; current != NULL; current = current->next) {
		printf("{%d : %s}\n", current->content, current->bin_repr_string);
	}
}

void PrintUnitFrequencies() {
	for (UnitNode* current = head; current != NULL; current = current->next) {
		printf("{%d : %ld}\n", current->content, current->frequency);
	}
}

size_t GetListSize() {
	size_t counter = 0;
	for (UnitNode* current = head; current != NULL; current = current->next) {
		++counter;
	}
	return counter;
}

void DeallocateUnitList() {
	UnitNode* tmp;
	while (head->next != NULL) {
		tmp = head;
		head = head->next;
		free(tmp->bin_repr_array);
		free(tmp->bin_repr_string);
		free(tmp);
	}
}