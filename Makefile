SRCS = command.c mpsse.c tap.c main.c
TARGET = jtag
INCLUDES = -I/usr/include/libftdi1
CFLAGS = -O2 -g -Wall
LIBS = -lusb -lftdi1
LDFLAGS =

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

-include $(SRCS:.c=.d)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LIBS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS) *.d $(TARGET)

