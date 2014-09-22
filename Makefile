GCC = gcc
CFLAGS = -c -Wall -g -Os
LD = $(GCC)
LDFLAGS =

TARGET = Dogstatsd

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(GCC) $(CFLAGS) $^ -o $@

cl:
	rm $(OBJECTS)

clean:
	rm $(TARGET) $(OBJECTS)