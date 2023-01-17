#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "MetadataCollector.h"
#include "../Compressor/Compressor.h"

#ifndef BYTE_LENGTH
	#define BYTE_LENGTH 8
#endif

#define STRINGBUF 1024
//end of header
#define EOH 0x00
//default BUFSIZE
#define BUFSIZE 6000

void CutoffBuffer(uint32_t* buffer, const int encode_length);

void CutoffSingle(uint32_t* buffer, size_t capacity);

UnitNode* SeekUnit(uint32_t _target);

void BufferHeader(char buffer[BUFSIZ], FILE* ifile_pointer, const uint8_t encode_length);

void WriteHeader(FILE* ofile_pointer, char header[BUFSIZ], int files_amount);

void BufferCodetable(FILE* ofile_pointer, char* buffer);

int WriteBody_p(FILE* ofile, const char* ifilename, uint8_t encode_length);

int WriteBody_c(FILE* ofile, const char* ifilename, uint8_t encode_length);

void WriteFile(const char* output_file, char header[BUFSIZ], char* filenames[], int files_amount, uint8_t encode_length);