#ifndef DECODE_HPP
#define DECODE_HPP

#include <iostream>
#include "file_utilities.hpp"
#include "binary_utilities.hpp"

std::string decode(const std::string& input_filename) {
    sf::Image input_image;
    if(!input_image.loadFromFile(input_filename)) {
        std::cout << "Could not load input image\n";
        return {};
    }

    const auto format = file_format_from_filename(input_filename);
    auto BYTES_PER_PIXEL { 0 };
    switch(format) {
        default: throw std::runtime_error("Unsupported file format\n");
        case FILE_FORMAT::BMP: {
            BYTES_PER_PIXEL = BYTES_PER_PIXEL_BMP;
            break;
        }
        case FILE_FORMAT::PNG: {
            BYTES_PER_PIXEL = BYTES_PER_PIXEL_PNG;
            break;
        }
    }

    std::string decrypted_message {};
    const auto pixel_data = input_image.getPixelsPtr();
    const auto read_until_byte_index = end_of_message_character_index(input_filename);
    if(read_until_byte_index == 0)
        return decrypted_message;

    for(size_t byte_index = 0; byte_index < read_until_byte_index; byte_index++) {
        std::string letter_in_binary {};

        for(size_t bit_index = 0; bit_index < BITS_PER_BYTE; bit_index++) {
            const auto current_byte_index = byte_index * BITS_PER_BYTE + bit_index;
            const auto current_byte = pixel_data[current_byte_index];
            const auto lsb_value = read_lsb_bit(current_byte);
            letter_in_binary.append(1, lsb_value ? '1' : '0');
        }

        const auto decrypted_letter = number_from_binary_string(letter_in_binary);
        decrypted_message.append(1, decrypted_letter);
    }

    return decrypted_message;
}

#endif
