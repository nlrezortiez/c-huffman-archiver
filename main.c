#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "Packer/Packer.h"
#include "Unpacker/Unpacker.h"

#ifndef BYTE_LENGTH
	#define BYTE_LENGTH 8
#endif

#ifndef DEF_EL
	#define DEF_EL 8 // default encode length
#endif

bool MODE_PACK = false;
bool MODE_UNPACK = false;
bool PRINT = false;
bool IPRINT = false;
uint8_t ENCODE_LENGTH = DEF_EL;

int ParseCLI(int argc, char* argv[], char* input_filenames[], char** output_filename) {
	uint8_t container_index = 0;
	if (argc == 1) {
		exit(printf("Usage:\n-p -i <filename>.<ext> [<filename>.<ext> <filename>.<ext>] -o <output_filename>.<txt> for pack or \n-u <arhcive_name.gar> <destination> for unpack\n"));
	}
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pack") == 0) {
				MODE_PACK = true;
				if (MODE_UNPACK) {
					exit(printf("You cant use --pack(-p) with --unpack(-u)\nUsage: \
						-p -i <filename>.<ext> [<filename>.<ext> <filename>.<ext>] -o <output_filename>.<txt> for pack or \
						-u <destination> for unpack\n"));
				}
			} else if (strcmp(argv[i], "-u") == 0  || strcmp(argv[i], "--unpack") == 0) {
				MODE_UNPACK = true;
				if (MODE_PACK) {
					exit(printf("You cant use --pack(-p) with --unpack(-u)\nUsage: \
						-p -i <filename>.<ext> [<filename>.<ext> <filename>.<ext>] -o <output_filename>.<txt> for pack or \
						-u -i <archive_name.gar> <destination> for unpack\n"));
				}
				return 0;
			} else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
				if (i == argc - 1) {
					exit(printf("Wrong usage. Use -p -i <filename>.<ext> [<filename>.<ext> <filename>.<ext>] -o <output_filename>.<txt>\n"));
				}
			} else if (strcmp(argv[i], "-o") == 0  || strcmp(argv[i], "--output") == 0) {
				if (i == argc - 1) {
					exit(printf("Wrong usage. Use -p -i <filename>.<ext> [<filename>.<ext> <filename>.<ext>] -o <output_filename>.<txt>\n"));
				} else {
					*output_filename = (char*)malloc(strlen(argv[i+1] + 1));
					strcpy(*output_filename, argv[i+1]);
				}
			} else if (strstr(argv[i], "-el=")) {
				ENCODE_LENGTH = atoi(strstr(argv[i], "=") + 1);
			} else if (strstr(argv[i], "--print-stat=true")) {
				PRINT = true;
				IPRINT = true;
			} else if (strstr(argv[i], "--print-stat=false")) {
				PRINT = false;
				IPRINT = true;
			}
		} else if (argv[i][0] != '-' && strcmp(argv[i-1], "-o")){
			input_filenames[container_index] = (char*)malloc(strlen(argv[i]) + 1);
			strcpy(input_filenames[container_index], argv[i]);
			++container_index;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	
	char* input_filenames[argc > 6 ? argc - 6 : 1];
	char* output_filename;
	ParseCLI(argc, argv, input_filenames, &output_filename);

	if (MODE_PACK) {
		Pack(input_filenames, output_filename, (argc-6) - IPRINT, ENCODE_LENGTH, PRINT);
		for (int i = 0; i < (argc - 6) - IPRINT; ++i) {
			free(input_filenames[i]);
		}
	} else if (MODE_UNPACK) {
		char* archive_name = argv[argc - 2];
		char* directory_name = argv[argc - 1];
		Unpack(archive_name, directory_name);
	} else {
		exit(printf("Wrong mode. Use --pack(-p) or --unpack(-u)"));
	}

	return 0;
}