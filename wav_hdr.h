#include <cstdint>

typedef struct
{
    uint8_t RIFF[4];
    uint32_t file_size;
    uint8_t WAVE[4];
    uint8_t fmt[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;  
    uint16_t num_channels;    
    uint32_t samples_per_sec;
    uint32_t bytes_per_sec;  
    uint16_t block_align;   
    uint16_t bits_per_sample;
    uint8_t subchunk2_id[4];
    uint32_t subchunk2_size;
} wav_hdr;