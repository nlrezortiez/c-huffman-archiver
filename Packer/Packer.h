#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "../Compressor/Compressor.h"
#include "../FileRoutine/Write.h"

#ifndef BYTE_LENGTH
	#define BYTE_LENGTH 8
#endif

void Pack(char* input_files[], const char* output_file, int files_amount, const uint8_t encode_length, bool is_print_option);

void BuildPreciseAlphabet(FILE* file_pointer, const int encode_length);

void BuildComplexAlphabet(FILE* file_pointer, const int encode_length);

UnitNode* PushUnitStructNode(UnitNode** head_reference, uint32_t content);

void PrintHuffmanCodes();

void PrintUnitFrequencies();

size_t GetListSize();

void DeallocateUnitList();