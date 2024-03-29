# GAR-Archiver
Простой архиватор, использующий lossless сжатие данных, основанное на алгоритме Хаффмена (на представлении контента в виде бинарных кодов переменной длины). Используется только стандартная библиотека языка С + makefile для сборки проекта, gdb, valgrind, gcc.

Для сборки проекта:
```make
make
```
## <b>Часть 0. API.</b>
Запуск подпрограммы кодирования, сжатия и архивирования:

```
./gar --pack -i <input_file1> -o <output_file1> -el=<encode_length>
```

Где <b>-el=<encode_length></b> - есть длина кодировки в соответствии с заданием (8/12/16/20 бит)

Для подпрограммы разархивирования:
```make
./gar --unpack <path_to_archive>.gar <directory_name>/ 
```
	
## <b>Часть 1. Считывание.</b>
Считывание данных из файла, хранение отдельных юнитов (8/12/16/20 бит) в виде связного списка. Список имеет вид: 

```C
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
```

Здесь:

```c
uint32_t content - считанный юнит

size_t freqiency - его встречаемость в файле (в файлах (связный список строится один на все архивируемые файлы))

uint8_t* bin_repr_array - бинарное представление юнита после его кодирования

char* bin_repr_string - бинарное представление после кодирования в виде строки

int8_t bin_length - длина бинарного кода данного юнита

struct UnitNode* left, *right - указатели, использующиеся для построения дерева Хаффмена

struct UnitNode* next - указатель на следующий элемент в связном списке
```

Логика считывания для точных кодировок (8/16 бит) предельно проста:
	
*Для 16 бит - смещение на 8 бит влево каждую нечетную итерацию*
	
``` c
while(!feof(file_pointer)) {
	size_t read = fread(buffer, 1, BUFSIZE, file_pointer);
	for (int i = 0; i < read; ++i) {
		// buffer[i] - current byte
		// построение связного списка
	}
}
```

Считывание неточных кодировок устроено сложнее: 

``` c
while (!feof(file_pointer)) {
	size_t read = fread(in_buffer, 1, BUFSIZE, file_pointer);
	size_t last_byte = 0;
	for (int i = 0; i + variable_size <= read; i += variable_size, last_byte = i) {
		// in_buffer[i] - текущий байт
	}
```
Сложности по сравнению с точныи кодировками возникли в следующем:
	
1. В какой то момент необходимо сдвигать указатель в потоке ```file_pointer``` на один байт назад, иначе потеряется часть данных.
	
2. Также на определенных итерациях необходимо "отрезать" часть буффера для того чтобы не записать лишних данных, поэтому появляется нужда использовать что-то типа такого: 
	
``` c
void CutoffBuffer(uint32_t* buffer, const int ENCODE_LENGTH) {
	(*buffer) <<= 32 - ENCODE_LENGTH;
	(*buffer) >>= 32 - ENCODE_LENGTH;
}

void CutoffSingle(uint32_t* buffer, size_t capacity) {
	(*buffer) <<= 32 - (capacity - 4);
	(*buffer) >>= 32 - (capacity - 4);
}
```

По завершении этого этапа заполнены поля ```content``` и ```frequency``` в связном списке, что пригодится далее при построении дерева.

Вывод этой статистики на консоль можно осуществить добавив флаг ```--print-stat=true``` (false by default)

## <b>Часть 2. Кодирование.</b>
Для реализации алгоритма Хаффмена используется структура данных Heap (по сути дерево, удовлетворяющее свойствам кучи). 

![heap](https://user-images.githubusercontent.com/74617877/209480283-1ddd73bd-ddb3-4714-a96b-17648190cbee.PNG)
	
На основании построенного в ```Части 1``` алфавита, а именно на основании полей ```size_t frequency``` и ```uint32_t content``` структуры связного списка для данных входных файлов (дерево строится одно на все файлы), происходит построение вышеупомянутого дерева с помощью алгоритма Хаффмена.
	
Алгоритм Хаффмена реализован в модуле ```Compressor``` в файле ```Compressor.c```. Здесь заполняются оставшиеся поля структуры ```UnitNode```, а именно ```bin_repr_array``` - массив ```1``` и ```0``` (для записи побитово в файл), а также ```bin_repr_string``` для удобного вывода на консоль и для дебага в целом.
Здесь различий в работе с точными и неточными кодировками нет, поскольку под юнит отведен 32-битный ```uint32_t```, который точно вместит в себя [8, 20] - битные кодировки, и массивы для бинарных представлений создаются в хипе, то есть память под них выделяется динамически уже после того как известна длина бинарной последовательности перекодированного символа.

``` c
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
```

Функция ```PrintHuffmanCodes()``` выводит на консоль соответсвия юнитов их кодам в формате ```{unit : binary_code}```, было полезно для дебага, но в целом не нужно.
	
## <b>Часть 3. Архивирование.</b>
За архивацию отвечает модуль ```FileRoutine```, в котором ```Write.c``` производит запись закодированного текста в архив, а ```MetadataCollector.c``` собирает метаданные о файле, необходимые для записи хедера в архив (размер файлов, их имена, длина кодировки, выбранная пользователем, хедер заканчивается байтом 0x00).
Перед кодированием непосредственно тела файла буферизируется и записывается хедер, это реализовано в том же модуле в файле ```Write.c``` внутри функции ```void BufferHeader()```. Основная проблема - хранение кодовой таблицы, поскольку на больших файлах с коидировкой типа 12/20 бит дерево очень широко разрастается, из-за чего таблица начинает занимать очень много места в хедере и в архиве в целом. В архив она записывается как обычный текст, на больших файлах появляется ```buffer overflow```((.
	
Запись при точной кодировке (8/16 бит) производится в функции ```WriteBody_p()```, при неточной - ```WriteBody_c()```. Разнесено по разным функциям поскольку запись неточно закодированных байтов также как и при построении алфавита оказалась сложнее, чем точных, а именно на определенных итерациях было необходимо использовать сдвиги для того чтобы не потерять/не записать лишнего. Функции записи не сильно отличаются от функций построения алфавита, сигнатура схожая, по сути просто производится второй проход по исходному кодируемому файлу.

Логика архивирования следующая: существует ```uint8_t out_buffer[BUFSIZE] = {}``` в который побайтово набиваются закодированные данные. Если байт остается незаполненным, то данные сдвигаются к левому краю, так проще декодировать:
	
``` c
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
```
После этих операций в переменной ```unpacked``` нужный нам контент. 
	
Сдвиг влево: 
	
``` c
out_buffer[cur_bit >> 3] <<= 8 - (cur_bit % 8);  //cur_bit >> 3 -- last index
```
	
то есть сдвиг на свободное слева кол-во бит. 
	
## <b>Часть 4. Разархивирование.</b>
*разархивирование нескольких файлов находится в разработке, архивация доступна, но необратима*

Так как подпрограммы архивирования и разархивирования обособленны и могут работать автономно, то для декодирования необходимо построить новое дерево, которое мы получаем из информации в хедере архива, а именно из кодовой таблицы. За логику разархивирования отвечает модуль ```Unpacker```. В функции ```Unpack()``` файла ```Unpacker.c``` происходит парс хедера, реализовано в функции ```ParseHeader()```, которая в свою очередь вызывает ```ParseCodetable```, в которой динамически выделяется память под хранение кодовой таблицы: 
	
``` c
typedef struct CodeTableUnit {
    uint32_t content;
    char binary_repr_string[MAX_HEIGHT];
} CodeTable;
```
	
``` c
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
```

Для оптимального использования памяти используется ```realloc()```, дабы не выделять буффер заведомо большего размера, но, кажется не оптимально по производительности. Массив с юнитами кодовой таблицы расширяется постепенно, с тем как происходит считывание кодовой таблицы из хедера архива. В конце вызывается ```BuildCodetableTree()```, где происходит восстановление дерева кодов по информации, полученной из хедера архива.
