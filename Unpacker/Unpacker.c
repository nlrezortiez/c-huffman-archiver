#include "Unpacker.h"

CodeTable** codetable_array = NULL;
UFile** file_array = NULL;
Tree* root = NULL;

void Unpack(const char* archive_name, char* directory_name) {
	FILE* archive_pointer = fopen(archive_name, "r");
	uint8_t files_amount;
	size_t codetable_s;
	if (archive_pointer == NULL) {
		exit(printf("Unable to unarchive %s\n", archive_name));
	}

	size_t header_size = GetHeaderSize(archive_pointer);
	ParseHeader(archive_pointer, header_size, &files_amount, &codetable_s);
	AddBinRepresentation(root);
	
	for (int i = 0; i < files_amount; ++i) {
		char output_filename[FILENAME_MAX] = {};
		strcat(output_filename, directory_name);
		strcat(output_filename, file_array[i]->filename);
		FILE* ofile_pointer = fopen(output_filename, "w+");
		if (ofile_pointer == NULL) {
			exit(printf("An error occurred while decoding\n"));
		}

		DecodeBody(archive_pointer, ofile_pointer, file_array[i]->filesize);

		fclose(ofile_pointer);
	}

	FreeMemory(files_amount, codetable_s);
}

size_t GetHeaderSize(FILE* archive) {
	char ch;
	while ((ch = fgetc(archive)) != EOH) {
		continue;
	}
	size_t header_size = ftell(archive);
	rewind(archive);
	return header_size;
}

void ParseHeader(FILE* archive, size_t header_size, uint8_t* files_amount, size_t* codetable_s) {
	char header_buffer[header_size + 1];
	char* delim_pointer;
	char* delimiter = "|";
	uint8_t idx = 0;
	size_t read = fread(header_buffer, 1, header_size, archive);
	if (read < header_size) {
		exit(printf("Error occurred while parsing header.\n"));
	}
	
	delim_pointer = strtok(header_buffer, delimiter);
	*files_amount = atoi(delim_pointer);
	file_array = (UFile**)malloc(sizeof(UFile*) * (*files_amount));
	for (int i = 0; i < *files_amount; ++i) {
		file_array[i] = (UFile*)malloc(sizeof(UFile));
	}

	while (delim_pointer != NULL) {
		delim_pointer = strtok(NULL, delimiter);
		if (delim_pointer != NULL) {
			file_array[idx]->encode_length = atoi(delim_pointer);
			delim_pointer = strtok(NULL, delimiter);
			file_array[idx]->filename_length = atoi(delim_pointer);
			//delim_pointer = strtok(NULL, delimiter);
			delim_pointer = strtok(NULL, delimiter);
			strcpy(file_array[idx]->filename, delim_pointer);
			delim_pointer = strtok(NULL, delimiter);
			file_array[idx]->filesize = atoi(delim_pointer);
		}

		//delim_pointer points to codetable now
		if (idx == (*files_amount) - 1) {
			delim_pointer = strtok(NULL, delimiter);
			ParseCodetable(delim_pointer, codetable_s);
			break;
		} else {
			++idx;
		}
	}
}

void ParseCodetable(char* codetable, size_t* codetable_s) {
	codetable_array = (CodeTable**)malloc(sizeof(CodeTable*)); 
	size_t codetable_idx = 0;
	char* tmp;
	size_t codetable_length = strlen(codetable);
	for (int i = 0; i < codetable_length; ++i) {
		if (codetable[i] == '{') {
			tmp = strtok(codetable + i + 1, ":");
			uint32_t content = atoi(tmp); 
			tmp = strtok(NULL, "}");
			char* string_repr = tmp;

			codetable_array[codetable_idx] = (CodeTable*)malloc(sizeof(CodeTable));
			codetable_array[codetable_idx]->content = content;
			strcpy(codetable_array[codetable_idx]->binary_repr_string, string_repr);
			++codetable_idx;
			codetable_array = realloc(codetable_array, codetable_idx * sizeof(CodeTable));
		}
	}
	*codetable_s = codetable_idx;
	BuildCodeTableTree(codetable_idx);
}


void BuildCodeTableTree(size_t size) {
	root = InsertNode(0);
	for (size_t i = 0; i < size; ++i) {
		Tree* current = root;
		size_t codelen = strlen(codetable_array[i]->binary_repr_string);
		char* bin_string = codetable_array[i]->binary_repr_string;
		for (size_t j = 0; j < codelen; ++j) {
			if (bin_string[j] == '1' && j != codelen - 1) {
				if (current->right != NULL) {
					current = current->right;
				} else {
					current->right = InsertNode(RIGHT);
					current = current->right;
				}
			} else if (bin_string[j] == '0' && j != codelen - 1) {
				if (current->left != NULL) {
					current = current->left;
				} else {
					current->left = InsertNode(LEFT);
					current = current->left;
				}
			} else if (bin_string[j] == '1' && j == codelen - 1) {
				current->right = InsertNode(codetable_array[i]->content);
			} else if (bin_string[j] == '0' && j == codelen - 1) {
				current->left = InsertNode(codetable_array[i]->content);
			}
		}
	}
}

void DecodeBody(FILE* bodystream, FILE* output_stream, size_t filesize) {
	uint8_t in_buffer[BUFSIZE];
	uint8_t out_buffer[5] = {};
	size_t read_bit = 0;
	Tree* current = root;
	size_t cur_bit = 0, total_bits = 0;

	while (!feof(bodystream) && read_bit < filesize << 3) {
		size_t read = fread(in_buffer, 1, BUFSIZE, bodystream);
		for (size_t i = 0; i < read; ++i) {
			uint8_t cur_byte = in_buffer[i];
			int8_t shift = 8;
			while (shift) {
				--shift;
				bool digit = (cur_byte >> shift) & 1;
				current = TraverseTree(current, digit);
				if (current == NULL) {
					exit(printf("An error occurred while decoding\n"));
				}
				if (Leaf(current)) {
					total_bits += file_array[0]->encode_length;
					int64_t to_read = (filesize << 3) - total_bits;
					to_read = (to_read < 0) ? (-to_read) : 0;
					for (int j = 0; j < file_array[0]->encode_length - to_read; ++j) {
						out_buffer[cur_bit >> 3] <<= 1;
						out_buffer[cur_bit >> 3] += current->binary_representation[j];
						++cur_bit;
					}
					
					if (cur_bit % 8 == 0) {
						fwrite(out_buffer, 1, cur_bit >> 3, output_stream);
						memset(out_buffer, 0, 5);
						cur_bit = 0;
					}
					current = root;
				}
			}
		}
	}
}

Tree* InsertNode(uint32_t content) {
	Tree* node = (Tree*)malloc(sizeof(Tree));
	node->content = content;
	node->left = node->right = NULL;

	return node;
}

bool Leaf(Tree* node) {
	return (node->left == NULL && node->right == NULL);
}

void AddBinRepresentation(Tree* node) {
	if (node != NULL) {
		if (Leaf(node)) {
			uint32_t tmp = node->content;
			for (int i = 0; i < file_array[0]->encode_length; ++i) {
				node->binary_representation[file_array[0]->encode_length - i - 1] = (tmp >> i) & 1;
			}
		}
		AddBinRepresentation(node->left);
		AddBinRepresentation(node->right);
	}
}

Tree* TraverseTree(Tree* node, uint8_t value) {
	if (value == LEFT) {
		return node->left;
	} else if (value == RIGHT) {
		return node->right;
	}
	return NULL;
}

void FreeMemory(uint8_t files_amount, size_t codetable_s) {
	//free file_array
	for (int i = 0; i < files_amount; ++i) {
		free(file_array[i]);
	}
	free(file_array);
	//free Tree
	Deallocate(root);

	//free codetable array
	for (int i = 0; i < codetable_s; ++i) {
		free(codetable_array[i]);
	}
	free(codetable_array);
}

void Deallocate(Tree* node) {
	if (node == NULL) {
		return;
	}
	Deallocate(node->right);
	Deallocate(node->left);

	free(node);
}