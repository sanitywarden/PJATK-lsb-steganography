#ifndef BINARY_UTILITIES_HPP
#define BINARY_UTILITIES_HPP

#include <bitset>
#include <cmath>
#include <ranges>

#define BITS_PER_BYTE 8
using byte = unsigned char;

size_t number_from_binary_string(const std::string& binary_string) {
    size_t number { 0 };
    for(int i = 0; i < binary_string.length(); i++) {
        auto value = binary_string[i] == '1';
        number += value * std::pow(2, binary_string.length() - (i + 1));
    }

    return number;
}

std::string binary_string_from_number(size_t number) {
    std::string binary_representation;
    while(number != 0) {
        const auto remainder = std::to_string(number % 2);
        binary_representation += remainder;
        number = number / 2;
    }

    std::ranges::reverse(binary_representation);
    return binary_representation;
}

// Get the state of a bit at certain position <0,8) from a byte.
// The position is counted from the rightmost bit.
bool get_bit_state(const size_t& number, int position) {
    return number & (1 << position);
}

byte set_bit_state(byte number, int position, bool value) {
    std::bitset <BITS_PER_BYTE> byte { number };
    byte.set(position, value);
    return byte.to_ulong();
}

byte change_lsb_bit(byte number, bool value) {
    return set_bit_state(number, 0, value);
}

bool read_lsb_bit(const byte& number) {
    return number % 2;
}

#endif
