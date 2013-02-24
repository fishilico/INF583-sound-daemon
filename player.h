#ifndef PLAYER_H
#define PLAYER_H

// Std ints
#include <stdint.h>

// Music file handle
typedef struct
{
    FILE *file;
    uint_fast32_t oss_format;
    uint_fast32_t channels;
    uint_fast32_t sample_rate;
    uint_fast32_t bits_per_sample;
    uint_fast32_t data_size;
} music_file_t;

int wave_opener(music_file_t * file_info);
int au_opener(music_file_t * file_info);
int dsp_configuration(int const fd_dsp, music_file_t const * audio_file);
int play_file(const char *file_name);
int player_main(int argc, char ** argv);

#endif /* PLAYER_H */
