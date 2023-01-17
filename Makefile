CC = gcc
CFLAGS = -Wall -g -O0 -march=native
LDFLAGS = 
OBJFILES = main.o Packer/Packer.o Unpacker/Unpacker.o Compressor/Compressor.o FileRoutine/MetadataCollector.o FileRoutine/Write.o
TARGET = gar

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~