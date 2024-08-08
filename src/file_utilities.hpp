#ifndef FILE_UTILITIES_HPP
#define FILE_UTILITIES_HPP

#include <string>
#include <cassert>
#include <stdexcept>
#include <fstream>

#include "binary_utilities.hpp"

#define END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT 4
// A special symbol is appended N times to the message, and is encrypted with the message so that it is possible to read the message later on.
// Probability of a certain combination of bits which would result in N END_OF_MESSAGE_CHARACTERs in a row is low. It gets more and more impossible the larger N is.
// With this approach we can confidently say if a message exists in the input image and the decoder will not return unreadable gibberish.

#define END_OF_MESSAGE_CHARACTER '~'
// The character should be rare.

#define ARGC_ENCRYPT 5
// Program arguments should equal to at most 5
// First one is the executable filename
// Second one is the execution mode (-e)
// Third one is the input filename
// Fourth one is the output filename
// Fifth one is the message

#define ARGC_DECRYPT 3
// Program arguments should equal to at most 3
// First one is the executable filename
// Second one is the execution mode (-d)
// Third one is the input filename

#define ARGC_HELP 2
// Program arguments should equal to at most 2
// First one is the executable filename
// Second one is the execution mode (-h)

#define ARGC_INFO 3
// First one is the executable filename
// Second one is the execution mode (-i)
// Third one is the input filename

#define ARGC_CHECK_WRITE 4
// First one is the executable filename
// Second one is the execution mode (-c)
// Third one is the input filename
// Fourth one is the message

#define ARGC_CHECK_READ 3
// First one is the executable filename
// Second one is the execution mode (-c)
// Third one is the input filename

// This program encodes ASCII characters <0, 256) into an image.
#define ONE_CHARACTER_SIZE_BITS 8

// https://en.wikipedia.org/wiki/PNG
// One pixel in a PNG image holds four properties: RGBA
#define BYTES_PER_PIXEL_PNG 4

// This program encodes a single byte from the message in the output image by changing the LSB of 8 consecutive bytes in the image.
// Depending on the image format different amount of pixels is required to encode a letter.
// One pixel (in PNG format) holds 4 bytes (for RGBA) thus 2 pixels are requried to encode a letter.
#define PIXELS_PER_BYTE_PNG (ONE_CHARACTER_SIZE_BITS / BYTES_PER_PIXEL_PNG)

// https://en.wikipedia.org/wiki/BMP_file_format
// One pixel in a BMP image holds three properties: RGB
// But it's padded to be 4 bytes.
#define BYTES_PER_PIXEL_BMP 4

// See PIXELS_PER_BYTE_PNG for explanation.
#define PIXELS_PER_BYTE_BMP (ONE_CHARACTER_SIZE_BITS / BYTES_PER_PIXEL_BMP)

enum class FILE_FORMAT {
    PNG,
    BMP,
};

std::string file_format_from_filename_str(const std::string& filename) {
    const auto extension_start_index = filename.find('.');
    const auto extension = filename.substr(extension_start_index + 1, filename.size() - extension_start_index + 1);
    return extension;
}

FILE_FORMAT file_format_from_filename(const std::string& filename) {
    const auto extension = file_format_from_filename_str(filename);
    if(extension == "png") return FILE_FORMAT::PNG;
    else if(extension == "bmp") return FILE_FORMAT::BMP;
    else throw std::runtime_error("File extension '" + extension + "' is not supported by this program\nSee -help for supported extensions");
    throw std::runtime_error("Code execution should not have reached this point!");
}

// Make sure that the last pixel is equivalent to the last four bytes of image data.
bool check_if_file_format_is_rgba(const std::string& filename) {
    sf::Image image;
    if(!image.loadFromFile(filename)) {
        std::cout << "Could not load input image\n";
        return false;
    }

    const auto image_pixel_count = image.getSize().x * image.getSize().y;
    const auto bytes_per_pixel_rgba = 4;
    auto last_four_bytes_start_index = (image_pixel_count * bytes_per_pixel_rgba) - bytes_per_pixel_rgba;
    const auto pixel_data = image.getPixelsPtr();
    sf::Color last_pixel_from_data = {
            pixel_data[last_four_bytes_start_index++],
            pixel_data[last_four_bytes_start_index++],
            pixel_data[last_four_bytes_start_index++],
            pixel_data[last_four_bytes_start_index++]
    };

    const auto last_pixel_from_image = image.getPixel(image.getSize().x - 1, image.getSize().y - 1);
    return last_pixel_from_data == last_pixel_from_image;
}

bool is_file_format_supported(const std::string& filename) {
    try {
        const auto file_format_enum = file_format_from_filename(filename);
    } catch (const std::runtime_error &e) {
        return false;
    }
    return true;
}

bool message_fits_in_image(size_t width, size_t height, const std::string& message, FILE_FORMAT format) {
    switch(format) {
        default: throw std::runtime_error("Unimplemented file format!");

        // The unit being compared is bytes.
        case FILE_FORMAT::BMP: return message.length() + END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT < (width * height / PIXELS_PER_BYTE_BMP) / BITS_PER_BYTE;
        case FILE_FORMAT::PNG: return message.length() + END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT < (width * height / PIXELS_PER_BYTE_PNG) / BITS_PER_BYTE;
    }
    throw std::runtime_error("Code execution should not have reached this point!");
}

bool message_fits_in_image(const std::string& filename, const std::string& message) {
    if(!is_file_format_supported(filename))
        return false;

    sf::Image image;
    if(!image.loadFromFile(filename))
        return false;

    const auto format = file_format_from_filename(filename);
    return message_fits_in_image(image.getSize().x, image.getSize().y, message, format);
}

size_t end_of_message_character_index(const std::string& filename) {
    if(!is_file_format_supported(filename))
        return 0;

    sf::Image image;
    if(!image.loadFromFile(filename)) {
        std::cout << "Could not load input image\n";
        return 0;
    }

    const auto format = file_format_from_filename(filename);
    auto BYTES_PER_PIXEL { 0 };
    switch(format) {
        case FILE_FORMAT::BMP: {
            BYTES_PER_PIXEL = BYTES_PER_PIXEL_BMP;
            break;
        }

        case FILE_FORMAT::PNG: {
            BYTES_PER_PIXEL = BYTES_PER_PIXEL_PNG;
            break;
        }
    }

    const auto image_size = image.getSize().x * image.getSize().y;
    const auto pixel_data = image.getPixelsPtr();
    auto eom_characters_in_a_row { 0 };
    for(size_t byte_index = 0; byte_index < image_size * BYTES_PER_PIXEL / BITS_PER_BYTE; byte_index++) {
        std::string letter_in_binary {};
        for(size_t bit_index = 0; bit_index < BITS_PER_BYTE; bit_index++) {
            const auto current_byte_index = byte_index * BITS_PER_BYTE + bit_index;
            const auto current_byte = pixel_data[current_byte_index];
            const auto lsb_value = (size_t)current_byte % 2;
            letter_in_binary.append(1, lsb_value ? '1' : '0');
        }

        const auto decrypted_letter = number_from_binary_string(letter_in_binary);
        if(decrypted_letter != END_OF_MESSAGE_CHARACTER && eom_characters_in_a_row > 0)
            eom_characters_in_a_row = 0;

        else if(decrypted_letter == END_OF_MESSAGE_CHARACTER)
            ++eom_characters_in_a_row;

        if(eom_characters_in_a_row == END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT)
            return byte_index - (END_OF_MESSAGE_CHARACTER_EXPECTED_COUNT - 1);
    }

    return 0;
}

bool may_message_exist(const std::string& filename) {
    return end_of_message_character_index(filename) > 0;
}

#endif
