#include <iostream>

#include "encode.hpp"
#include "decode.hpp"

int main(int argc, char *argv[]) {
    std::string help_or_execution_mode;
    if(argc > 1) help_or_execution_mode = std::string(argv[1]);

    if((argc == 1 || argc == ARGC_HELP) && (help_or_execution_mode == "--help" || help_or_execution_mode == "-h" || help_or_execution_mode.empty())) {
        std::cout << "OPTIONS:\n";
        std::cout << "\t-i --info\tTO CHECK FORMAT SUPPORT:   \t" << " -i <input_file>\n";
        std::cout << "\t-e --encode \tTO ENCODE IMAGE:        \t" << " -e <input_file> <output_file> \"<message>\"\n";
        std::cout << "\t-d --decrypt\tTO DECODE IMAGE:        \t" << " -d <input_file>\n";
        std::cout << "\t-c --check\tTO CHECK IF MESSAGE FITS: \t" << " -c <input_file> \"<message>\"\n";
        std::cout << "\t-h --help\tTO DISPLAY THIS MENU:      \t" << " -h\n";
        std::cout << "\n\nSUPPORTED FILE FORMATS:\n\t.png .bmp\n";
        return 0;
    }

    else if(argc == ARGC_INFO && (help_or_execution_mode == "--info" || help_or_execution_mode == "-i")) {
        const auto input_filename = std::string(argv[2]);

        if(is_file_format_supported(input_filename))
            std::cout << "File format is supported\n";
        else std::cout << "File format is not supported\n";
        return 0;
    }

    else if((argc == ARGC_CHECK_WRITE || argc == ARGC_CHECK_READ) && (help_or_execution_mode == "--check" || help_or_execution_mode == "-c")) {
        const auto input_filename = std::string(argv[2]);

        if(!is_file_format_supported(input_filename)) {
            std::cout << "File format is not supported or file does not exist\n";
            return 1;
        }

        // Question: does message fit into file
        if(argc == ARGC_CHECK_WRITE) {
            const auto message = std::string(argv[3]);
            if(message_fits_in_image(input_filename, message))
                std::cout << "Message will fit in the image\n";
            else std::cout << "Message is too long\n";
        }

        // Question: may this file hold a message?
        else {
            if(may_message_exist(input_filename) > 0)
                std::cout << "Message may exist inside of the image\n";
            else std::cout << "Message does not exist inside of the image\n";
        }

        return 0;
    }

    else if(argc == ARGC_ENCRYPT && (help_or_execution_mode == "--encode" || help_or_execution_mode == "-e")) {
        const auto input_filename = std::string(argv[2]);
        const auto output_filename = std::string(argv[3]);
        const auto message = std::string(argv[4]);

        if(!is_file_format_supported(input_filename) || !is_file_format_supported(output_filename)) {
            std::cout << "File format is not supported or file does not exist\n";
            return 1;
        }

        std::cout << "Attempting to encode message into " << input_filename << " (output = " + output_filename + ")\n";
        const auto result = encode(input_filename, output_filename, message);
        if(result) std::cout << "Message encoded successfully\n";
        else std::cout << "Failed to encode message\n";
        return 0;
    }

    else if(argc == ARGC_DECRYPT && (help_or_execution_mode == "--decrypt" || help_or_execution_mode == "-d")) {
        const auto input_filename = std::string(argv[2]);

        if(!is_file_format_supported(input_filename)) {
            std::cout << "File format is not supported or file does not exist\n";
            return 1;
        }

        std::cout << "Decoding " << input_filename << "\n";
        const auto message = decode(input_filename);
        if(!message.empty()) std::cout << "Decoded message (" << message.length() << " bytes): '" << message << "'\n";
        else std::cout << "File may have been encrypted, but no message was found in image\n";
        return 0;
    }

    else {
        std::cout << "Unsupported argument or their combination, see --help for help\n";
        return 0;
    }
}