#define _POSIX_C_SOURCE 1

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <netinet/in.h>
#include "player.h"

// Length of the buffer in seconds
#define BUF_SEC 2


// Convenient macros for system calls

#define MY_READ(FILE,ADDR,SIZE) \
    do \
    { \
        ret = fread (ADDR, 1, SIZE, FILE); \
        if (ret == 0 && ferror(FILE)) \
        { \
            fprintf (stderr, "fread failed\n"); \
            return 2; \
        } \
    } \
    while (0)

#define MY_IOCTL(FD,ID,ARG) \
    do \
    { \
        ret = ioctl (FD, ID, ARG); \
        if (ret == -1) \
        { \
            perror ("ioctl"); \
            return 2; \
        } \
    } \
    while (0)

// Endianness
#include <sys/types.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define U32_TO_LE(x) (x)
#define U16_TO_LE(x) (x)
#elif BYTE_ORDER == BIG_ENDIAN
#define  U32_TO_LE(x) \
    ((x) >> 24 | ((x) >> 8) & 0x0000FF00 | \
     ((x) << 8) & 0x00FF0000 | (x) << 24)
#define U16_TO_LE (x) ((x) >> 8 | (x) << 8)
#else
#error Machine endianness not detected or not supported!
#endif



/**
 * Read info in WAV files.
 */
int wave_opener(music_file_t * file_info)
{
    // Apart from the magic numbers, the RIFF spec says that everything
    // is stored in little-endian.

    int ret = 0;
    FILE * file = file_info -> file;

    uint32_t file_size;
    MY_READ(file, & file_size, sizeof(file_size));
    file_size = U32_TO_LE(file_size);
    file_size += 8;
    printf("[WAV] File size: %u.\n", file_size);

    uint32_t magic_number;
    MY_READ(file, & magic_number, sizeof(magic_number));
    magic_number = ntohl (magic_number);
    if (magic_number != 0x57415645) {
        fprintf(stderr, "This RIFF file not a WAVE file.\n");
        return 1;
    }

    uint32_t magic_number_header;
    MY_READ(file, & magic_number_header, sizeof(magic_number_header));
    magic_number_header = ntohl (magic_number_header);
    if (magic_number_header != 0x666d7420) {
        fprintf(stderr, "WAVE file ERROR: no header!!\n");
        return 1;
    }

    uint32_t header_size;
    MY_READ(file, & header_size, sizeof(header_size));
    header_size = U32_TO_LE (header_size);
    printf("[WAV] Header size: %u.\n", header_size);

    uint16_t encoding;
    MY_READ(file, & encoding, sizeof(encoding));
    encoding = U16_TO_LE (encoding);
    printf("[WAV] WAVE encoding format: %u.\n", encoding);
    if (encoding != 1) {
        fprintf(stderr, "Encoding not supported. Sorry...\n");
        return 1;
    }

    uint16_t channels;
    MY_READ(file, & channels, sizeof(channels));
    channels = U16_TO_LE (channels);
    printf("[WAV] Nb of channels: %u.\n", channels);
    file_info -> channels = channels;

    uint32_t sample_rate;
    MY_READ(file, & sample_rate, sizeof(sample_rate));
    sample_rate = U32_TO_LE (sample_rate);
    printf("[WAV] Sample rate: %u.\n", sample_rate);
    file_info -> sample_rate = sample_rate;

    uint32_t byte_rate;
    MY_READ(file, & byte_rate, sizeof(byte_rate));
    byte_rate = U32_TO_LE (byte_rate);
    printf("[WAV] Byte rate: %u.\n", byte_rate);

    uint16_t block_align;
    MY_READ(file, & block_align, sizeof(block_align));
    block_align = U16_TO_LE (block_align);
    printf("[WAV] Block size: %u.\n", block_align);

    uint16_t bits_per_sample;
    MY_READ(file, & bits_per_sample, sizeof(bits_per_sample));
    bits_per_sample = U16_TO_LE (bits_per_sample);
    printf("[WAV] Bits per sample: %u.\n", bits_per_sample);
    file_info -> bits_per_sample = bits_per_sample;

    uint_fast32_t oss_format;
    switch (bits_per_sample) {
        case 8:
            oss_format = AFMT_U8;
            break;

        case 16:
            oss_format = AFMT_S16_LE;
            break;

        default:
            fprintf(stderr, "This value of bits per sample not supported. Sorry...\n");
            return 1;
    }
    file_info -> oss_format = oss_format;

    // Header sanity checks
    if (block_align != channels * bits_per_sample / 8 ||
        (uint_fast32_t) block_align * sample_rate != byte_rate) {
        fprintf(stderr, "WAVE file error: Internal inconsistency in the header.\n");
        return 1;
    }

    uint32_t magic_number_data;
    MY_READ(file, & magic_number_data, sizeof(magic_number_data));
    magic_number_data = ntohl (magic_number_data);
    if (magic_number_data != 0x64617461) {
        fprintf(stderr, "WAVE file ERROR: no data.\n");
        return 1;
    }

    uint32_t data_size;
    MY_READ(file, & data_size, sizeof(data_size));
    data_size = U32_TO_LE (data_size);
    printf("[WAV] Data size: %u.\n",data_size);
    file_info -> data_size = data_size;

    return 0;
}


/**
 * Read informations in AU files
 */
int au_opener(music_file_t * file_info)
{
    int ret;
    FILE * file = file_info -> file;

    uint32_t header_size;
    MY_READ(file, & header_size, sizeof(header_size));
    header_size = ntohl (header_size);
    printf("[AU] Header size: %u.\n", header_size);

    uint32_t data_size;
    MY_READ(file, & data_size, sizeof(data_size));
    data_size = ntohl (data_size);
    printf("[AU] Data size: %u.\n",data_size);
    file_info -> data_size = data_size;

    uint32_t encoding;
    MY_READ(file, & encoding, sizeof(encoding));
    encoding = ntohl (encoding);

    unsigned oss_format;
    uint_fast32_t bits_per_sample;
    switch (encoding) {
        case 2:
            bits_per_sample = 8;
            oss_format = AFMT_S8;
            printf("[AU] Encoding format: Signed 8-bit.\n");
            break;

        case 3:
            bits_per_sample = 16;
            oss_format = AFMT_S16_BE;
            printf("[AU] Encoding format: Signed 16-bit.\n");
            break;

        default:
            printf("[AU] Encoding format: %u.\n", encoding);
            fprintf(stderr, "Encoding not supported. Sorry...\n");
            return 1;
    }
    file_info -> bits_per_sample = bits_per_sample;
    file_info -> oss_format = oss_format;

    uint32_t sample_rate;
    MY_READ(file, & sample_rate, sizeof(sample_rate));
    sample_rate = ntohl (sample_rate);
    printf("[AU] Sample rate: %u.\n", sample_rate);
    file_info -> sample_rate = sample_rate;

    uint32_t channels;
    MY_READ(file, & channels, sizeof(channels));
    channels = ntohl (channels);
    printf("[AU] Nb of channels: %u.\n", channels);
    file_info -> channels = channels;

    // Position the cursor to the beginning of the data section
    if (fseek(file, header_size, SEEK_SET) == -1) {
        perror("fseek");
        return 2;
    }

    return 0;
}


/**
 * Open a music file in AU or WAVE format
 * Please call fclose(file_info->file) to free the file descriptor
 */
int open_music_file(const char *file_name, music_file_t *file_info)
{
    // Open file
    file_info->file = fopen (file_name, "rb");
    if (file_info->file == NULL) {
        fprintf(stderr, "Couldn't open the file!\n");
        return 2;
    }

    // Look for magic number to determine file type
    int ret;
    uint32_t magic_number;
    MY_READ(file_info->file, & magic_number, sizeof(magic_number));
    magic_number = ntohl (magic_number);

    if (magic_number == 0x52494646) {
        // Seems to be a RIFF file. Try to see if it's a WAVE one.
        ret = wave_opener(file_info);
    } else if (magic_number == 0x2e736e64) {
        // Decode file header
        ret = au_opener(file_info);
    } else {
        fprintf(stderr, "File format not recognized.\n");
        fclose(file_info->file);
        return 2;
    }

    if (ret) {
        fprintf(stderr, "Header parsing failed! File may have been corrupted.\n");
        fclose(file_info->file);
        return 2;
    }

    return 0;
}


/**
 * Set the parameters of the dsp device
 */
int dsp_configuration(int const fd_dsp, music_file_t const * audio_file)
{
    unsigned arg;
    int ret;

    // Nb of channels
    arg = audio_file -> channels;
    MY_IOCTL (fd_dsp, SNDCTL_DSP_CHANNELS, & arg);
    if (arg != audio_file -> channels) {
        fprintf(stderr, "This number of channels not supported by OSS! Sorry...\n");
        return 1;
    }

    // Sample format
    arg = audio_file -> oss_format;
    MY_IOCTL (fd_dsp, SNDCTL_DSP_SETFMT, & arg);
    if (arg != audio_file -> oss_format) {
        fprintf(stderr, "Unable to set OSS sample format.\n");
        return 1;
    }

    // Sample rate
    arg = audio_file -> sample_rate;
    MY_IOCTL (fd_dsp, SNDCTL_DSP_SPEED, & arg);
    if (arg != audio_file -> sample_rate) {
        fprintf(stderr, "This sample rate is not supported by OSS... Sorry!\n");
        return 1;
    }

    return 0;
}


/**
 * Prepare to play a file
 */
int open_music_buffer(const char *file_name, music_buffer_t *music_buf)
{
    int ret;

    // Zero music buffer
    memset(music_buf, 0, sizeof(*music_buf));
    music_buf->fd_dsp = -1;
    
    // Open the music file
    ret = open_music_file(file_name, &(music_buf->info));
    if (ret) {
        return ret;
    }

    // Compute and print file duration
    unsigned const oct_per_sec =
        music_buf->info.bits_per_sample *
        music_buf->info.sample_rate *
        music_buf->info.channels / 8;
    float const duration = music_buf->info.data_size / oct_per_sec;
    printf("File duration: %g seconds.\n", duration);
    
    // Open sound device
    music_buf->fd_dsp = open("/dev/dsp", O_WRONLY);
    if (music_buf->fd_dsp == -1) {
        perror("open: /dev/dsp");
        close_music_buffer(music_buf);
        return 2;
    }

    // Configure sound device
    ret = dsp_configuration(music_buf->fd_dsp, &(music_buf->info));
    if (ret) {
        fprintf(stderr, "Configuration of sound device failed... :-(\n");
        close_music_buffer(music_buf);
        return 2;
    }

    // Alloc the playing buffer
    music_buf->buf = malloc(music_buf->buf_size = BUF_SEC * oct_per_sec);

    if (!music_buf->buf) {
        fprintf(stderr, "Couldn't allocate the buffer to play the file.\n");
        close_music_buffer(music_buf);
        return 2;
    }

    return 0;
}


/**
 * Free every resources associated with music_buf
 */
int close_music_buffer(music_buffer_t *music_buf)
{
    if (music_buf->fd_dsp != -1) {
        close(music_buf->fd_dsp);
        music_buf->fd_dsp = -1;
    }
    if (music_buf->info.file != NULL) {
        fclose(music_buf->info.file);
        music_buf->info.file = NULL;
    }
    if (music_buf->buf != NULL) {
        free(music_buf->buf);
        music_buf->buf = NULL;
    }
    return 0;
}


/**
 * Play one step of the buffer
 */
int play_step_music_buffer(music_buffer_t *music_buf)
{
    size_t ret = 0;
    MY_READ(music_buf->info.file, music_buf->buf, music_buf->buf_size);
    size_t bytes = ret;
    if (bytes == 0) {
        if (!feof(music_buf->info.file)) {
            return 0;
        } else {
            fprintf(stderr, "An error occured while reading the file.\n");
            return 1;
        }
    }
    ret = write(music_buf->fd_dsp,  music_buf->buf, bytes);
    if (ret != bytes) {
        fprintf(stderr, "Writing to the sound device failed.\n");
        return 2;
    }
    return 0;
}


/**
 * Test end of file
 */
int eof_music_buffer(music_buffer_t *music_buf)
{
    return (music_buf == NULL) ||
        (music_buf->info.file == NULL) ||
        feof(music_buf->info.file);
}


/**
 * Play a file
 */
int play_file(const char *file_name)
{
    music_buffer_t music_buf;
    int ret = open_music_buffer(file_name, &music_buf);
    if (ret) return ret;
    fflush(stdout);

    // Play the file
    while (!eof_music_buffer(&music_buf)) {
        ret = play_step_music_buffer(&music_buf);
        if (ret) break;
    }
    close_music_buffer(&music_buf);
    return ret;
}

/**
 * Main function of a simple player
 */
int player_main(int argc, char ** argv)
{
    // Args
    if (argc != 2) {
        fprintf(stderr, "Wrong number of args.\n");
        return 1;
    }

    // File name
    char const * file_name = argv[1];
    printf("File name: '%s'.\n", file_name);
    return play_file(file_name);
}
