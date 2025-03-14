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
    size_t size;
    size_t capacity;
    long double *data;
} Buffer;

// ============================================================


/*  
    resize buffer to new_capacity, my current code, size is same as capacity. just by
    reading  [ buffer->size / sizeof(int) ], i know how many integer inside the buffer
*/
static void buffer_resize(Buffer *buffer, size_t new_capacity){
    buffer->capacity = new_capacity;
    buffer->data = realloc(buffer->data, buffer->capacity);
}

/*
    in my code, anyways need to resize when adding new integer
    the buffer works like an array, but resizable
*/
static void buffer_write(Buffer *buffer, const long double *data, size_t size){
    if(buffer->size + size > buffer->capacity){
        buffer_resize(buffer, buffer->capacity + size);
    }
    memcpy(buffer->data + (buffer->size / sizeof(long double)), data, size);
    buffer->size += size;
}

static void buffer_free(Buffer *buffer){
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}

/*
    read from buffer, similar to read from array.
    Because data is set as integer type, so buffer->data always read 4 bytes.
*/
static int buffer_read(Buffer *buffer, size_t index, long double *out_value){
    if (index >= buffer->size){
        return 0;
    }
    *out_value = buffer->data[index];
    return 1;
}

/*
static String_View buffer_to_sv(Buffer *buffer){
    return (String_View){
        .count = buffer->size,
        .data = buffer->data
    };
}
*/

// ============================================================


/*
    converts char array aka string, to sv
    sv is like an array, but not using array --> google why string view is better than array
*/

String_View sv_from_cstr(const char *cstr){
    return (String_View){
        .count = strlen(cstr),
        .data = cstr,
    };
}

String_View sv_trim_left(String_View sv){
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[i])){
        i += 1;
    }
    return (String_View){
        .count = sv.count - i,
        .data = sv.data + i,
    };
}

String_View sv_trim_right(String_View sv){
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[sv.count -1 -i])){
        i += 1;
    }
    return (String_View){
        .count = sv.count - i,
        .data = sv.data,
    };
}


/*
    deletes the extra spaces and tab in front and behind. Eg. (   testing   ) --> (testing)
*/

String_View sv_trim(String_View sv){
    return sv_trim_right(sv_trim_left(sv));
}

/*
    input:  testing, today, stream

    sv_chop_left( input, 10)

    overwrite the sv:   (oday, stream)                  there is a space in front of today
    returns:            (testing, t)

*/

String_View sv_chop_left(String_View *sv, size_t n){
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

/*
    find c
    return true, and index = the index which c is located
    return false, if cannot find c
*/

int sv_index_of(String_View sv, char c, size_t *index){
    size_t i = 0;
    while(i < sv.count && sv.data[i] != c){
        i += 1;
    }
    if(i < sv.count){
        *index = i;
        return 1;
    } else {
        return 0;
    }
}

/*
    input:  testing, today, stream

    sv_chop_by_delim( input, ',' )

    overwrite the sv:   ( today, stream)                  there is a space in front of today
    returns:            (testing)

*/

String_View sv_chop_by_delim(String_View *sv, char delim){
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

// search linearly until finding prefix. Prefix must be in front of sv.
int sv_has_prefix(String_View sv, String_View prefix){
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

// check if sv is equal to each other.
int sv_eq(String_View a, String_View b){
    if(a.count != b.count){
        return 0;
    } else {
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

/* 
    by casting (int) to a string, you get the ASCII value.
    input string:  1234

    initially, result = 0
    i=0,  reads 1,  result = 0*10 + 1 = 1
    i=1,  reads 2,  result = 1*10 + 2 = 12
    i=2,  reads 3,  result = 12*10 + 3 = 123
    i=3,  reads 4,  result = 123*10 + 4 = 1234


    UPDATES: cast to long double
    if not digit return as 0
*/

int sv_to_u64(String_View sv, long double *output)
{
    long double result = 0;

    for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i) {
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

// ============================================================

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