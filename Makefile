# Makefile for IRIX Audio Library
# Compiled for IRIX 6.5 with MIPSpro Compiler

# Compiler
CC = cc

# Compiler flags
# -32 for 32-bit compilation, -mips3 for MIPS III instruction set
# -O2 for optimization, -g for debugging symbols
CFLAGS = -32 -mips3 -O2 -Wall

# Linker flags
LDFLAGS = -shared -32

# Library name
LIB_NAME = libirixaudio.so
STATIC_LIB_NAME = libirixaudio.a

# Source files
SRCS = irix_audio.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = irix_audio.h

# Include paths
INCLUDES = -I/usr/include/audio -I.

# Libraries
LIBS = -lAL -lm

# Example programs
EXAMPLES = irix_audio_info irix_two_streams sine_tone_generator audio_recorder audio_loopback

# Targets
all: $(LIB_NAME) $(STATIC_LIB_NAME) examples

# Examples
examples: $(EXAMPLES)

# Shared library
$(LIB_NAME): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Static library
$(STATIC_LIB_NAME): $(OBJS)
	ar crs $@ $^

# Compile source files into object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Example program compilation rules
irix_audio_info: irix_audio_info.c $(LIB_NAME)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -lirixaudio $(LIBS)

irix_two_streams: irix_two_streams.c $(LIB_NAME)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -lirixaudio $(LIBS)

sine_tone_generator: sine_tone_generator.c $(LIB_NAME)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -lirixaudio $(LIBS)

audio_recorder: audio_recorder.c $(LIB_NAME)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -lirixaudio $(LIBS)

audio_loopback: audio_loopback.c $(LIB_NAME)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -lirixaudio $(LIBS)

# Clean up build files
clean:
	rm -f $(OBJS) $(LIB_NAME) $(STATIC_LIB_NAME) $(EXAMPLES)

# Install library (requires root/sudo)
install: $(LIB_NAME) $(STATIC_LIB_NAME)
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include
	cp $(LIB_NAME) /usr/local/lib/
	cp $(STATIC_LIB_NAME) /usr/local/lib/
	cp irix_audio.h /usr/local/include/

# Uninstall library (requires root/sudo)
uninstall:
	rm -f /usr/local/lib/$(LIB_NAME)
	rm -f /usr/local/lib/$(STATIC_LIB_NAME)
	rm -f /usr/local/include/irix_audio.h

.PHONY: all clean install uninstall examples
