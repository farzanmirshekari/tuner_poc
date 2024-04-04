#include "wav_hdr.h"

#include <fftw3.h>

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[]) {
    wav_hdr wav_header;
    int header_size = sizeof(wav_hdr);

    if (argc <= 1)
    {
        std::cerr << "Usage: " << argv[0] << " <input.wav>" << std::endl;
        return 1;
    }
    char* const input_file = argv[1];

    FILE* wav_file = std::fopen(input_file, "rb");
    if (wav_file == nullptr)
    {
        std::cerr << "Unable to open wave file: " << input_file << std::endl;
        return 1;
    }

    std::fread(&wav_header, 1, header_size, wav_file);

    std::fseek(wav_file, 0, SEEK_END);
    long file_size = std::ftell(wav_file);
    long data_size = file_size - header_size;
    std::fseek(wav_file, header_size, SEEK_SET);

    std::unique_ptr<int8_t[]> data_buffer = std::make_unique<int8_t[]>(data_size);
    fread(data_buffer.get(), sizeof(int8_t), data_size, wav_file);

    int num_samples = data_size / (wav_header.bits_per_sample / 8);
    double* in = reinterpret_cast<double*>(fftw_malloc(sizeof(double) * num_samples));
    fftw_complex* out = reinterpret_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * (num_samples / 2 + 1)));
    fftw_plan plan = fftw_plan_dft_r2c_1d(num_samples, in, out, FFTW_ESTIMATE);
    
    for (int i = 0; i < num_samples; ++i)
    {
        short sample = reinterpret_cast<int16_t*>(data_buffer.get())[i];
        in[i] = static_cast<double>(sample) / 32768.0;
    }

    fftw_execute(plan);

    double max_magnitude = 0.0;
    int max_index = 0;
    for (int i = 0; i < (num_samples / 2) + 1; ++i)
    {
        double magnitude = out[i][0] * out[i][0] + out[i][1] * out[i][1];
        if (magnitude > max_magnitude)
        {
            max_magnitude = magnitude;
            max_index = i;
        }
    }

    double dominant_frequency = max_index * static_cast<double>(wav_header.samples_per_sec) / num_samples;
    std::cout << "Dominant frequency: " << dominant_frequency << "Hz" << std::endl;

    std::string const notes[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    int note_index = static_cast<int>(round(12 * log2(dominant_frequency / 440.0))) + 9;
    std::cout << "Note: " << notes[note_index] << std::endl;

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    std::fclose(wav_file);

    return 0;
}