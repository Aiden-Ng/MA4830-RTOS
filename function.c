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
    if (index >= buffer->size){
        return 0;
    }
    *out_value = buffer->data[index];
    return 1;
}

// ====================  sv related function  ========================================

String_View sv_from_cstr(const char *cstr){
    return (String_View){
        .count = strlen(cstr),
        .data = cstr,
    };
}

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

int sv_to_u64(String_View sv, long double *output)
{
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