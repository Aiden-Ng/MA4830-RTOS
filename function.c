#ifndef __FUNCTION_C__
#define __FUNCTION_C__

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

typedef struct
{
	size_t count;
	const char *data;
} String_View;

typedef struct{
    size_t size; //what is the difference between size and capacity
    size_t capacity; //
    long double *data;
} Buffer;

// ====================  Buffer related functions  ========================================

/*  
    resize buffer to new_capacity, my current code, size is same as capacity. just by
    reading  [ buffer->size / sizeof(int) ], i know how many integer inside the buffer
*/
void buffer_resize(Buffer *buffer, size_t new_capacity){
    buffer->capacity = new_capacity;
    buffer->data = realloc(buffer->data, buffer->capacity);
}

void buffer_write(Buffer *buffer, const long double *data, size_t size){
    /*  How does buffer_write work
        stores long double as bytes in an array
        
        before storing, need to realloc the old buffer so there is enough space for new data

        before exiting, need to free;
    */
    if(buffer->size + size > buffer->capacity){
        buffer_resize(buffer, buffer->capacity + size);
    }
    memcpy(buffer->data + (buffer->size / sizeof(long double)), data, size); //is this incrementing it? the + (buffer->size / sizeof(int)) part
    buffer->size += size;
}

void buffer_free(Buffer *buffer){
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}

int buffer_read(Buffer *buffer, size_t index, long double *out_value){
    /*	How does buffer_read work ?
    
        buffer_read access the buffer like an array
    */
    if (index >= buffer->size){
        return 0;
    }
    *out_value = buffer->data[index];
    return 1;
}

// ====================  sv related function  ========================================

String_View sv_from_cstr(const char *cstr){
    /*
        converts char array aka string
        lets say string = "This is a string"
        string_view =
            .data = "This is a string"
            .count = 16
    */
    return (String_View){
        .count = strlen(cstr),
        .data = cstr,
    };
}

String_View sv_chop_left(String_View *sv, size_t n){
    /*
        how does sv_chop_left work ?
        similar to sv_chop_by_delim but instead of a char, we chop based on number

        input: 123456789

        overflow = sv_chop_left(input, 5)

        overwrite the input 				: 6789
        return value, stored in overflow	: 12345

    */

    if(n > sv->count){
        n = sv->count;
    }
    String_View result = {
        .data = sv->data,
        .count = n,
    };
    sv->data += n;
    sv->count -= n;
    return result;
}

String_View sv_chop_by_delim(String_View *sv, char delim){
    /*	How does sv_chop_by_delim work? 
        Example 1:
            input: "-12.345"

            sv_chop_by_delim(input, '-')
            overwrite the input					: 12.345
            return value, not stored			: -

        Example 2:
            input: "-12.345"

            before_dot = sv_chop_by_delim( input, '.')
            overwrite the input					:   345
            return value, stored in before_dot	:   -12
    */
    size_t i = 0;
    while(i<sv->count && sv->data[i] != delim){
        i += 1;     // cycles through and find the delim
    }  
    String_View result = {
        .count = i,
        .data = sv->data,
    };
    // if i < count, +1 to include delimiter
    if( i < sv->count){
        sv->count -= i + 1;
        sv->data += i + 1;
    } else {    // if i > count, delimiter does not exist.
        sv->count -= i;
        sv->data += i;
    }
    return result;
}

int sv_has_prefix(String_View sv, String_View prefix){
    /*
        search linearly until finding prefix. Prefix must be in front of sv.
        For example,  input = "-123"
            returns 1 if found prefix
            returns 0 if not found prefix
    */
    if(prefix.count <= sv.count){
        for(size_t i = 0; i < prefix.count; ++i){
            if(prefix.data[i] != sv.data[i]){
                return 0;
            }
        }
        return 1;
    } else{
        return 0;
    }
}

int sv_to_u64(String_View sv, long double *output)
{
    /*	How does sv_to_u64 work? 

        Theory: by casting (int) to a string, you get the ASCII value, minus of '0', you get integer
        
        input:  1234
        initially, result = 0
        i=0,  reads 1,  result = 0*10 + 1 = 1
        i=1,  reads 2,  result = 1*10 + 2 = 12
        i=2,  reads 3,  result = 12*10 + 3 = 123
        i=3,  reads 4,  result = 123*10 + 4 = 1234

        UPDATES: cast to long double
        if non-digit is found, replace input_num with 0

        function returns 1 if non-digit is found, otherwise return 0

        function written like this so that, function can do 2 things:
            1) return 1 and 0, to check non-digit
            2) modify the input_num

    */
    long double result = 0;

    for (size_t i = 0; i < sv.count; ++i) {
        result = result * 10 + (long double)(int)sv.data[i] - '0';
        if(isdigit(sv.data[i])){

        } else {
            *output = 0;
            return 1;
        }
    }
    *output = result;
    return 0;
}

// ====================  Unneccessarily complex way of printf  ========================================

void VLOG(FILE *stream, const char* tag, const char* fmt, va_list args)
{
    fprintf(stream, "[%s] ", tag);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

// INFO
void INFO(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "INFO", fmt, args);
    va_end(args);
}

// WARNING and does not kill application
void WARN(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "wARN", fmt, args);
    va_end(args);
}

// ERROR and kills application
void PANIC(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    VLOG(stderr, "ERRO", fmt, args);
    va_end(args);
    exit(1);
}

#endif