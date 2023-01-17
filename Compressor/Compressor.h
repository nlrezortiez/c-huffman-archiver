#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef MAX_HEIGHT
	#define MAX_HEIGHT 64
#endif

typedef struct UnitNode {
	uint32_t content;
	size_t frequency;
	
	uint8_t* bin_repr_array;
	char* bin_repr_string;
	int8_t bin_length;

	struct UnitNode* left;
	struct UnitNode* right;
	struct UnitNode* next;
} UnitNode;

typedef struct Heap {
	uint32_t size;
	uint32_t capacity;

	struct UnitNode** pUnitArray;
} Heap;


UnitNode* NewNode(uint32_t content, size_t frequency);

Heap* CreateMinH(uint32_t capacity);

void SwapNode(UnitNode** a, UnitNode** b);

void MinHeapify(Heap* minHeap, int idx);

bool IsSizeEqualsOne(Heap* minHeap);

UnitNode* ExtractMinNode(Heap* minHeap);

void InsertMinHeap(Heap* minHeap, UnitNode* minHeapNode);

void BuildMinHeap(Heap* minHeap);

bool IsLeaf(UnitNode* root);

Heap* CreateAndBuildMinHeap(UnitNode* head, size_t size);

UnitNode* BuildHuffmanTree(UnitNode* head, size_t size);

void SetArray(const uint8_t arr[], uint8_t n, UnitNode* EncodeUnit);

void SetHCodes(UnitNode* root, uint8_t arr[], uint8_t top);

void Compress(UnitNode* head, size_t list_size);

void DeallocateHeap();