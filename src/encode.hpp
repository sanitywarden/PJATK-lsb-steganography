#ifndef STEGANOGRAPHY_ENCODE_HPP
#define STEGANOGRAPHY_ENCODE_HPP

#include <iostream>
#include <SFML/Graphics.hpp>

#include "file_utilities.hpp"
#include "binary_utilities.hpp"

bool encode(const std::string& input_filename, const std::string& output_filename, std::string message_content) {
    sf::Image input_image;
    if(!input_image.loadFromFile(input_filename)) {
        std::cout << "Could not load input image\n";
        return false;
    }

    auto PIXELS_PER_BYTE { 0 };
    auto BYTES_PER_PIXEL { 0 };
    const auto file_format_enum = file_format_from_filename(input_filename);
    switch(file_format_enum) {
        default: throw std::runtime_error("Unsupported file format\n");
        case FILE_FORMAT::BMP: {
            PIXELS_PER_BYTE = PIXELS_PER_BYTE_BMP;
            BYTES_PER_PIXEL = BYTES_PER_PIXEL_BMP;
            break;
        }
        case FILE_FORMAT::PNG: {
            PIXELS_PER_BYTE = PIXELS_PER_BYTE_PNG;
            BYTES_PER_PIXEL = BYTES_PER_PIXEL_PNG;
            break;
        }
    }

    // See END_OF_MESSAGE_CHARACTER and END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT in file_utilities.hpp
    message_content.append(END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT, END_OF_MESSAGE_CHARACTER);

    const auto message_size_bytes = message_content.length();

    // 1 byte of data will be saved in 8 bytes in the target image.
    // Thus, one byte of data requires two PNG pixels (one pixel holds four bytes) to be saved.
    const auto amount_of_pixels_to_change = message_size_bytes * PIXELS_PER_BYTE;
    const auto input_image_pixel_data = input_image.getPixelsPtr();

    // Index to the letter being written.
    auto current_letter_index { 0 };

    // Index to the bit of currently written value.
    // When a letter is being written value is between <0,8>.
    auto current_letter_bit { 0 };

    sf::Image output_image = input_image;
    for(size_t pixel_index = 0; pixel_index < amount_of_pixels_to_change; pixel_index++) {
        // Pixel position in the image.
        const auto x = pixel_index % input_image.getSize().x;
        const auto y = pixel_index / input_image.getSize().y;

        auto red = input_image_pixel_data[pixel_index + 0];
        auto green = input_image_pixel_data[pixel_index + 1];
        auto blue = input_image_pixel_data[pixel_index + 2];
        auto alpha = input_image_pixel_data[pixel_index + 3];

        for(size_t byte_index = 0; byte_index < BYTES_PER_PIXEL; byte_index++) {
            const auto current_byte = input_image_pixel_data[pixel_index * BYTES_PER_PIXEL + byte_index];
            const auto current_letter = message_content[current_letter_index];
            const auto new_last_bit_value = get_bit_state(current_letter, BITS_PER_BYTE - (current_letter_bit + 1));
            const auto changed_byte_value = change_lsb_bit(current_byte, new_last_bit_value);

            current_letter_bit = ++current_letter_bit % BITS_PER_BYTE;
            if(current_letter_bit == 0) ++current_letter_index;

            switch(byte_index) {
                case 0: red   = changed_byte_value; break;
                case 1: green = changed_byte_value; break;
                case 2: blue  = changed_byte_value; break;
                case 3: alpha = changed_byte_value; break;
            }
        }

        output_image.setPixel(x, y, { red, green, blue, alpha });
    }

    return output_image.saveToFile(output_filename);
}


#endif
