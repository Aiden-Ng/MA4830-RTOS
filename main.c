#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>

#include "function.c"

char* foo = "true";
int arr[4] = {1,4,16,64};

static char *shift(int *argc, char ***argv){
	assert(*argc > 0);
	char* result = **argv;
	*argv += 1;
	*argc -= 1;
	return result;
}
bool isGeometric(Buffer *buffer){
	int count = buffer->size/sizeof(int);
	//INFO("%d", count);   // for debug
	if (count < 2){
		INFO("Please input at least 2 values");
		return false;
	}
	int a = buffer->data[0];
	int b = buffer->data[1];
	int r = b / a;
	int z = a;
	for(int i = 1; i < count; ++i){
		z = z * r;
		//INFO("%d", z);   // for debug 
		if(z != buffer->data[i]){
			INFO("Not Geometric Series, wrong number is %d, the correct number is %d", buffer->data[i], z);
			return false;
		};
	}
	INFO("Is a Geometric Series");
	return true;
}

static Buffer actual_buffer = {0};

int main(int argc, char** argv)
{

	printf("MA4830 CA1\n\n");
	const char *program = shift(&argc, &argv);
	//printf("%s\n", program);

	if(argc == 0){
		INFO("Usage: %s -i <int arrays>", program);
		PANIC("ERROR: No flag provided");
	}

	const char *flag = shift(&argc, &argv);
	if(strcmp(flag, "-i") == 0){
		if(argc == 0){
			INFO("Usage: %s -i <int arrays>", program);
			PANIC("ERROR: No argument is provided for flag '%s'", flag);
		}
	} else if(strcmp(flag, "-u") == 0){
		INFO("%s\n", "UI mode");
		exit(0);
	} else if(strcmp(flag, "-h") == 0){
		INFO("%s\n", "This is the help section");
		exit(0);
	} else {
		INFO("Usage: %s -i <int arrays>", program);
		PANIC("ERROR: Unknown flag '%s'", flag);
	}

	while(argc > 0){
		const char *input_str = shift(&argc, &argv);
		String_View input_sv = sv_from_cstr(input_str);
        int input_num = sv_to_u64(input_sv);
		buffer_write(&actual_buffer, &input_num, sizeof(int));
	}

	
	int data_toscr = 0;
	for(size_t i = 0; i < actual_buffer.size / sizeof(int); ++i){
		buffer_read(&actual_buffer, i, &data_toscr);
		INFO("%lld", data_toscr);  // for debug
	}
	
	isGeometric(&actual_buffer);

	//fprintf(stdout, "%ld\n", actual_buffer.size);
	buffer_free(&actual_buffer);
	return 0;
}