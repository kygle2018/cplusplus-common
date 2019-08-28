OBJECT = json_reader.o json_writer.o json_value.o
TARGET = ./libjson.a
SAMPLE = sample

INC_CLIB = -I../

CC = g++
AR = ar cr

CFLAGS = -g -fPIC

all:  $(OBJECT) $(TARGET) $(SAMPLE)

$(TARGET): $(OBJECT)
	@$(AR) $(TARGET) $(OBJECT)
	@ranlib $(TARGET)
	@echo $@

.cpp.o:
	@$(CC) $(CFLAGS)  -c -Wall $< $(INC_CLIB)
	@echo $@
	
sample: sample.cpp
	@$(CC) $(CFLAGS) -o $(SAMPLE) $(SAMPLE).cpp  $(INC_CLIB)  -L./  -ljson
	@echo $(SAMPLE)

	
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f $(SAMPLE)

