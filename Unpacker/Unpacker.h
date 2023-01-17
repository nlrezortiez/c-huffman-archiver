#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "../Compressor/Compressor.h"

#ifndef EOH
    #define EOH 0x00
#endif

#ifndef BYTE_LENGTH
	#define BYTE_LENGTH 8
#endif

#define DIR_DELIMITER "/"

#ifdef __WIN32__
    #define DIR_DELIMITER "\\"
#endif 

#ifndef MAX_HEIGHT
	#define MAX_HEIGHT 64
#endif

#ifndef STRINGBUF
    #define STRINGBUF 1024
#endif

//tree defines
#define LEFT 0
#define RIGHT 1

#ifndef BUFSIZE
    #define BUFSIZE 6000
#endif

typedef struct UFile {
    char filename[FILENAME_MAX];
    size_t filesize;
    size_t filename_length;
    uint8_t encode_length;
} UFile;

typedef struct CodeTableUnit {
    uint32_t content;
    char binary_repr_string[MAX_HEIGHT];
} CodeTable;

typedef struct CodeTableTree {
    uint32_t content;
    uint8_t binary_representation[BYTE_LENGTH * sizeof(uint32_t)];

    struct CodeTableTree* left;
    struct CodeTableTree* right;
} Tree;

void Unpack(const char* archive_name, char* directory_name);

size_t GetHeaderSize(FILE* archive);

void ParseHeader(FILE* archive, size_t header_size, uint8_t* files_amount, size_t* codetable_s);

void ParseCodetable(char* codetable, size_t* codetable_s);

void BuildCodeTableTree(size_t size);

Tree* InsertNode(uint32_t content);

bool Leaf(Tree* root);

void AddBinRepresentation(Tree* root);

void DecodeBody(FILE* bodystream, FILE* output_stream, size_t filesize);

Tree* TraverseTree(Tree* node, uint8_t value);

void FreeMemory(uint8_t files_amount, size_t codetable_s);

void Deallocate(Tree* node);