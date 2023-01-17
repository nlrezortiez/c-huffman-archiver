#include "MetadataCollector.h"

FData* fhead = NULL;

size_t SizeOfFile(FILE* file_pointer) {
	fseek(file_pointer, 0L, SEEK_END);
	size_t filesize = ftell(file_pointer);
	fseek(file_pointer, 0L, SEEK_SET);

	return filesize;
}


void CollectMetadata(char* filename, FILE* file_pointer, const uint8_t encode_length) {
	FData* current_file = (FData*)malloc(sizeof(FData));

	current_file->encode_length = encode_length;

	current_file->file_pointer = file_pointer;

	strcpy(current_file->filename, filename);
	current_file->filename_l = strlen(current_file->filename);

	int tmp = current_file->filesize = SizeOfFile(file_pointer);
	current_file->filesize_l = 0;
	do {
		++current_file->filesize_l;
		tmp /= 10;
	} while (tmp);

	current_file->next = fhead;
	fhead = current_file;
}

void DeallocateFileList() {
	FData* tmp;
	while (fhead != NULL) {
		tmp = fhead;
		fhead = fhead->next;
		free(tmp);
	}
}