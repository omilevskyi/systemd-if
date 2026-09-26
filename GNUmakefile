CC	:= gcc
CFLAGS	:= -Oz -flto -s -Wall -Wextra -Wpedantic

ifneq ($(strip $(VERSION)),)
CFLAGS	+= -DVERSION=\"$(VERSION)\"
endif

LDFLAGS	:= -Oz -flto -s -static
LDFLAGS	+= -Wl,-O3
LDFLAGS	+= -Wl,-flto
LDFLAGS	+= -Wl,--gc-sections
LDFLAGS	+= -Wl,--as-needed
LDFLAGS	+= -Wl,--sort-common
LDFLAGS	+= -Wl,-z,pack-relative-relocs
LDFLAGS	+= -Wl,-z,defs

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
