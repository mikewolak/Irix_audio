# IRIX Audio Library (libirixaudio) Manual

## Overview

The IRIX Audio Library is a lightweight, C-based audio I/O library designed for Silicon Graphics IRIX systems. It provides a simple, efficient interface for audio input, output, and device management, built on top of the IRIX Audio Library (AL).

## Library Components

### Key Features
- Support for audio input and output streams
- Multiple audio data formats
- Device discovery and information retrieval
- Low-level audio device abstraction

### Supported Audio Formats
- 8-bit signed integer
- 16-bit signed integer
- 24-bit signed integer
- 32-bit signed integer
- 32-bit float
- 64-bit float

## Data Structures

### Audio Modes
```c
typedef enum {
    IRIX_AUDIO_INPUT,    // Input-only mode
    IRIX_AUDIO_OUTPUT,   // Output-only mode
    IRIX_AUDIO_DUPLEX    // Simultaneous input and output
} IrixAudioMode;
```

### Audio Formats
```c
typedef enum {
    IRIX_AUDIO_SINT8 = 0x1,     // 8-bit signed integer
    IRIX_AUDIO_SINT16 = 0x2,    // 16-bit signed integer
    IRIX_AUDIO_SINT24 = 0x4,    // 24-bit signed integer
    IRIX_AUDIO_SINT32 = 0x8,    // 32-bit signed integer
    IRIX_AUDIO_FLOAT32 = 0x10,  // 32-bit float
    IRIX_AUDIO_FLOAT64 = 0x20   // 64-bit float
} IrixAudioFormat;
```

### Device Information Structure
```c
typedef struct {
    int max_output_channels;    // Maximum output channels
    int min_output_channels;    // Minimum output channels
    int max_input_channels;     // Maximum input channels
    int min_input_channels;     // Minimum input channels
    int* sample_rates;          // Supported sample rates
    int sample_rates_count;     // Number of supported sample rates
    unsigned int native_formats;// Supported audio formats
} IrixAudioDeviceInfo;
```

### Stream Parameters
```c
typedef struct {
    IrixAudioMode mode;     // Audio stream mode
    int channels;           // Number of audio channels
    int sample_rate;        // Sampling rate
    int buffer_size;        // Audio buffer size
} IrixAudioStreamParams;
```

## Function Reference

### Initialization and Device Management

#### `int irix_audio_initialize()`
- Discovers and initializes audio devices
- Returns number of devices found or -1 on error

#### `int irix_audio_get_device_count()`
- Retrieves the number of available audio devices

#### `int irix_audio_get_device_info(int device_index, IrixAudioDeviceInfo* info)`
- Retrieves detailed information about a specific audio device
- Fills the `IrixAudioDeviceInfo` structure
- Returns 0 on success, -1 on error

#### `void irix_audio_cleanup()`
- Frees resources allocated during device initialization
- Should be called at the end of audio operations

### Stream Management

#### `IrixAudioStream* irix_audio_open_stream(IrixAudioStreamParams* params)`
- Creates and configures an audio stream
- Returns a pointer to the created stream or NULL on error

#### `void irix_audio_close_stream(IrixAudioStream* stream)`
- Closes an open audio stream
- Releases associated resources

### Audio I/O Operations

#### `int irix_audio_write_frames(IrixAudioStream* stream, void* buffer, int frames)`
- Writes audio frames to an output stream
- Returns number of frames written or -1 on error

#### `int irix_audio_read_frames(IrixAudioStream* stream, void* buffer, int frames)`
- Reads audio frames from an input stream
- Returns number of frames read or -1 on error

### Error Handling

#### `const char* irix_audio_get_last_error()`
- Retrieves the last error message
- Useful for diagnosing issues during audio operations

## Usage Examples

### Simple Audio Output
```c
// Set up stream parameters
IrixAudioStreamParams params = {
    .mode = IRIX_AUDIO_OUTPUT,
    .channels = 2,
    .sample_rate = 44100,
    .buffer_size = 256
};

// Open stream
IrixAudioStream* stream = irix_audio_open_stream(&params);
if (!stream) {
    printf("Error opening stream: %s\n", irix_audio_get_last_error());
    return;
}

// Write audio data
float buffer[256];
// ... fill buffer with audio data ...
irix_audio_write_frames(stream, buffer, 256);

// Cleanup
irix_audio_close_stream(stream);
```

### Audio Device Enumeration
```c
// Initialize audio system
int device_count = irix_audio_initialize();
printf("Found %d audio devices\n", device_count);

// Print device information
for (int i = 0; i < device_count; i++) {
    IrixAudioDeviceInfo info;
    if (irix_audio_get_device_info(i, &info) == 0) {
        printf("Device %d: Input Channels %d, Output Channels %d\n", 
               i, info.max_input_channels, info.max_output_channels);
    }
}

// Cleanup
irix_audio_cleanup();
```

## Compilation and Linking

### Compiler Flags
- Use `-lirixaudio` to link against the library
- Requires `-lAL` for IRIX Audio Library support
- Use `-lm` for math functions (e.g., sine wave generation)

### Example Makefile Compilation
```makefile
CC = cc
CFLAGS = -32 -mips3 -O2
LIBS = -lAL -lm -lirixaudio

my_audio_program: my_audio_program.c
    $(CC) $(CFLAGS) -o $@ $< $(LIBS)
```

## Limitations and Considerations
- Specifically designed for IRIX 6.5 systems
- Depends on the IRIX Audio Library (AL)
- Limited to the audio capabilities of SGI hardware
- No built-in audio file format conversion

## Version and Compatibility
- Extracted from RtAudio library
- Original Copyright (c) 2001-2005 Gary P. Scavone
- Converted from C++ to C for broader compatibility

## Troubleshooting
- Always check `irix_audio_get_last_error()` after operations
- Ensure proper initialization and cleanup
- Verify device capabilities before opening streams
- Match buffer formats with device native formats

## License
Refer to the original RtAudio library licensing terms

## Contributing
Contributions and improvements are welcome. Please submit pull requests or issue reports to the project repository.
