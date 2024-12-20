#include "irix_audio.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Initialize audio system
    int devices = irix_audio_initialize();
    if (devices < 0) {
        fprintf(stderr, "Failed to initialize audio: %s\n", irix_audio_get_last_error());
        return EXIT_FAILURE;
    }

    printf("\nFound %d device(s) ...\n", devices);

    // Iterate through devices and print information
    for (int i = 0; i < devices; i++) {
        IrixAudioDeviceInfo info;
        
        // Get device info
        if (irix_audio_get_device_info(i, &info) != 0) {
            fprintf(stderr, "Failed to get info for device %d: %s\n", 
                    i, irix_audio_get_last_error());
            continue;
        }

        printf("\nDevice %d:\n", i);
        
        // Print output channel information
        if (info.max_output_channels > 0) {
            printf("Output Channels: %d to %d\n", 
                   info.min_output_channels, 
                   info.max_output_channels);
        }

        // Print input channel information
        if (info.max_input_channels > 0) {
            printf("Input Channels: %d to %d\n", 
                   info.min_input_channels, 
                   info.max_input_channels);
        }

        // Print supported sample rates
        printf("Supported Sample Rates:");
        for (int j = 0; j < info.sample_rates_count; j++) {
            printf(" %d", info.sample_rates[j]);
        }
        printf("\n");

        // Print native formats
        printf("Supported Formats:\n");
        if (info.native_formats & IRIX_AUDIO_SINT8)
            printf("  8-bit signed integer\n");
        if (info.native_formats & IRIX_AUDIO_SINT16)
            printf("  16-bit signed integer\n");
        if (info.native_formats & IRIX_AUDIO_SINT24)
            printf("  24-bit signed integer\n");
        if (info.native_formats & IRIX_AUDIO_SINT32)
            printf("  32-bit signed integer\n");
        if (info.native_formats & IRIX_AUDIO_FLOAT32)
            printf("  32-bit float\n");
        if (info.native_formats & IRIX_AUDIO_FLOAT64)
            printf("  64-bit float\n");
    }

    // Cleanup
    irix_audio_cleanup();

    return EXIT_SUCCESS;
}
