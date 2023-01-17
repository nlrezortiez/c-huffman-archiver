#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifndef FILENAME_MAX
	#define FILENAME_MAX 256
#endif

typedef struct FileMetadata {
	char filename[FILENAME_MAX]; // offset: 0
	size_t filesize; 			 // offset: 256(4096)

	FILE* file_pointer;			 // offset: 320(4160)
	uint8_t encode_length;		 // offset: 328(4168)

	size_t filename_l;			 // offset: 336(4176)
	size_t filesize_l;			 // offset: 400(4240)

	struct FileMetadata* next;	 // offset: 464(4304)
} FData;

size_t SizeOfFile(FILE* file_pointer);

void CollectMetadata(char* filename, FILE* file, const uint8_t encode_length);

void DeallocateFileList();