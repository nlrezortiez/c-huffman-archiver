#include "Compressor.h"

Heap* heap = NULL;

UnitNode* NewNode(uint32_t content, size_t frequency) {
    UnitNode* node = (UnitNode*) malloc(sizeof(UnitNode));

    node->left = node->right = NULL;
    node->content = content;
    node->frequency = frequency;

    return node;
}

Heap* CreateMinH(uint32_t capacity) {
    Heap* minHeap = (Heap*) malloc(sizeof(Heap));

    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->pUnitArray = (UnitNode**) malloc(minHeap->capacity * sizeof(UnitNode*));

    return minHeap;
}

void SwapNode(UnitNode** a, UnitNode** b) {
    UnitNode* t = *a;
    *a = *b;
    *b = t;
}

void MinHeapify(Heap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->pUnitArray[left]->frequency < minHeap->pUnitArray[smallest]->frequency)
        smallest = left;

    if (right < minHeap->size && minHeap->pUnitArray[right]->frequency < minHeap->pUnitArray[smallest]->frequency)
        smallest = right;

    if (smallest != idx) {
        SwapNode(&minHeap->pUnitArray[smallest], &minHeap->pUnitArray[idx]);
        MinHeapify(minHeap, smallest);
    }
}

bool IsSizeEqualsOne(Heap* minHeap) {
    return (minHeap->size == 1);
}

UnitNode* ExtractMinNode(Heap* minHeap) {
    UnitNode* node = minHeap->pUnitArray[0];
    minHeap->pUnitArray[0] = minHeap->pUnitArray[minHeap->size - 1];

    --minHeap->size;
    MinHeapify(minHeap, 0);

    return node;
}

void InsertMinHeap(Heap* minHeap, UnitNode* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && minHeapNode->frequency < minHeap->pUnitArray[(i - 1) / 2]->frequency) {
        minHeap->pUnitArray[i] = minHeap->pUnitArray[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->pUnitArray[i] = minHeapNode;
}

void BuildMinHeap(Heap* minHeap) {
    int n = minHeap->size - 1;

    for (int i = (n - 1) / 2; i >= 0; --i) {
        MinHeapify(minHeap, i);
    }      
}

bool IsLeaf(UnitNode* root) {
    return !(root->left) && !(root->right);
}

Heap* CreateAndBuildMinHeap(UnitNode* head, size_t size) {
    Heap* minHeap = CreateMinH(size);

    int i;
    UnitNode* current;
    for (current = head, i = 0; current != NULL; current = current->next, ++i) {
        minHeap->pUnitArray[i] = current;
    }

    minHeap->size = size;
    BuildMinHeap(minHeap);

    return minHeap;
}

UnitNode* BuildHuffmanTree(UnitNode* head, size_t size) {
    UnitNode* left, *right, *top;
    heap = CreateAndBuildMinHeap(head, size);

    while (!IsSizeEqualsOne(heap)) {
        left = ExtractMinNode(heap);
        right = ExtractMinNode(heap);

        top = NewNode('$', left->frequency + right->frequency);

        top->left = left;
        top->right = right;

        InsertMinHeap(heap, top);
    }
    return ExtractMinNode(heap);
}

void SetArray(const uint8_t arr[], uint8_t n, UnitNode* EncodeUnit) {
    EncodeUnit->bin_repr_string = (char*) malloc(n + 1);
    EncodeUnit->bin_repr_array = (uint8_t*) malloc(n);
    EncodeUnit->bin_length = n;

    char tmp[MAX_HEIGHT] = {0}, code_string[MAX_HEIGHT] = {0};
    for (int i = 0; i < n; ++i) {
        sprintf(tmp, "%d", arr[i]);
        strcat(code_string, tmp);
        EncodeUnit->bin_repr_array[i] = arr[i];
    }
    strcpy(EncodeUnit->bin_repr_string, code_string);
}

void SetHCodes(UnitNode* root, uint8_t arr[], uint8_t top) {
    if (root->left) {
        arr[top] = 0;
        SetHCodes(root->left, arr, top + 1);
    }
    if (root->right) {
        arr[top] = 1;

        SetHCodes(root->right, arr, top + 1);
    }
    if (IsLeaf(root)) {
        SetArray(arr, top, root);
    }
}

void DeallocateHeap() {
    free(heap->pUnitArray);
    free(heap);
}

void Compress(UnitNode* head, size_t list_size) {
    UnitNode* root = BuildHuffmanTree(head, list_size);

    uint8_t arr[MAX_HEIGHT];
    uint8_t top = 0;
    SetHCodes(root, arr, top);
}