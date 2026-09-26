CC	:= gcc
CFLAGS	:= -Oz -s -Wall -Wextra -Wpedantic
LDFLAGS	:= -static -s
LDLIBS	:= -lsystemd -lcap

TARGET	:= systemd-if
SRCS	:= $(wildcard *.c)
OBJS	:= $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
