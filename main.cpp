// standard library headers
// cl
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <BaseTsd.h>

#include <iostream>
#include <string>

#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS

#define INITIAL_BUFFER_SIZE 128

// own functions
//#include "func.h"

typedef SSIZE_T ssize_t;

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;


//
InputBuffer* new_input_buffer()
{
    InputBuffer* input_buffer = static_cast<InputBuffer*>(malloc(sizeof(InputBuffer)));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}


// Prints db prompt for cli
void print_prompt()
{
    printf("db > ");

}


void read_input(InputBuffer* input_buffer) {
    // <string> called line
    std::string line;

    //  use getline to store into string
    std::getline(std::cin, line);


    if (line.empty()) {
        std::cerr << "Error reading input\n";
        std::exit(EXIT_FAILURE);
    }


    // now store getline into our struct:
    // Check if the buffer needs to be resized
    if (line.length() + 1 > input_buffer->buffer_length) {
        input_buffer->buffer_length = line.length() + 1;
        input_buffer->buffer = static_cast<char*>(realloc(input_buffer->buffer, input_buffer->buffer_length));
        if (!input_buffer->buffer) {
            std::cerr << "Memory allocation failed\n";
            std::exit(EXIT_FAILURE);
        }
    }

    // Copy input string into buffer
    std::strcpy(input_buffer->buffer, line.c_str());
    input_buffer->input_length = line.length();

}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}


// Main
int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();

    // infinite main loop
    while (true) {
        print_prompt(); // prints db cli
        read_input(input_buffer); //reads our input


        // Exit command: .exit
        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }
        else {
            printf("Command unrecognized: '%s'.\n", input_buffer->buffer);
        }
    }
}
