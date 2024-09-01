// standard library headers
// cl
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <BaseTsd.h>

#include <iostream>
#include <string>
#include <stdint.h>

#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS

#define INITIAL_BUFFER_SIZE 128

typedef SSIZE_T ssize_t;

typedef struct
{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;

typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum
{
    STATEMENT_INSERT, STATEMENT_SELECT
} StatementType;


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct
{
    uint32_t id;
    // +1 for \n null character
    char username[COLUMN_USERNAME_SIZE +1];
    char email[COLUMN_EMAIL_SIZE +1];
} Row;

typedef struct
{
    StatementType type;
    // used by insert cmd
    Row row_to_insert;
} Statement;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
} Table;

void print_row(Row* row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serialize_row(Row* source, void* destination) {
    memcpy((char*)destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy((char*)destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy((char*)destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->id), (char*)source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), (char*)source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), (char*)source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* row_slot(Table* table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table->pages[page_num];
    if (page == NULL) {
        // Allocate memory only when we try to access page
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return (char*)page + byte_offset;
}

Table* new_table() {
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table* table) {
    for (int i = 0; table->pages[i]; i++) 
    {
        free(table->pages[i]);
    }
    free(table);
}





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




MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        free_table(table);
        exit(EXIT_SUCCESS);
    }
    else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) 
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) 
    {
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email);
        if (args_assigned < 3) 
        {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "select") == 0) 
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement* statement, Table* table)
{
    if (table->num_rows >= TABLE_MAX_ROWS) 
    {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows++;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table)
{
    switch (statement->type) {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);
    case (STATEMENT_SELECT):
        return execute_select(statement, table);
    }
}



// Main
int main(int argc, char* argv[])
{
    // Initiate new tables and input buffers
    InputBuffer* input_buffer = new_input_buffer();
    Table* table = new_table(); 

    // REPL infinite Loop that doesn't break until command .exit
    while (true)
    {
        
        print_prompt(); // prints db CLI 
        read_input(input_buffer); //reads our input


        // Waiting for commands
        // Note: .exit to break loop
        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table))
            {
            case (META_COMMAND_SUCCESS): // matches command 
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND): // does not match command 
                printf("Command not recognized '%s'\n", input_buffer->buffer);
                continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_SYNTAX_ERROR):
            printf("Syntax error. Could not parse statement.\n");
            continue;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s'.\n",
                input_buffer->buffer);
            continue;
        }

        switch (execute_statement(&statement, table))
        {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        case (EXECUTE_TABLE_FULL):
            printf("Error: Table full.\n");
            break;

        }

    }
}
