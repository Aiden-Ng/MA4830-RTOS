#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#include "function.c"

#define PRECISION 1e-9

static char *shift(int *argc, char ***argv){
	assert(*argc > 0);
	char* result = **argv;
	*argv += 1;
	*argc -= 1;
	return result;
}

int isDoubleEqual(double a, double b){
	return fabs(a - b) < PRECISION;
}

int checkGeometric(Buffer *buffer, Buffer *wrong){
	int count = buffer->size/sizeof(long double);
	if (count < 2){
		INFO("isGeometric Fault: Please input at least 2 values");
		return 0;
	}
	long double a = buffer->data[0];
	long double b = buffer->data[1];
	if (a == 0 || b == 0){
		if(a == 0) WARN("DETECTED first value is 0");
		if(b == 0) WARN("DETECTED second value is 0");
		return 0;
	}

	long double r = b / a;
	long double z = a;
	for(int i = 1; i < count; ++i){
		z = z * r;
		if(isDoubleEqual(z, buffer->data[i])){
			
		} else {
			buffer_write(wrong, &buffer->data[i], sizeof(long double));
			buffer_write(wrong, &z, sizeof(long double));
		};
	}
	return 1;
}

static Buffer actual_buffer = {0};
static Buffer wrong_data = {0};

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
		String_View original = input_sv;
		String_View neg = sv_from_cstr("-");
		
		long double _decimal, input_num, rounding_overflow = 0;

		if(sv_has_prefix(input_sv , neg)){
			sv_chop_by_delim(&input_sv, '-');
		}
		String_View before_dot = sv_chop_by_delim(&input_sv, '.');

		// integer overflow, replacing with 0
		if(before_dot.count > 19){
			INFO("Long Double Integer Overflow detected at %s", input_str);
			INFO("Replacing with 0 . . . . . . ");
			before_dot.data = "0";
			before_dot.count = 1;
			input_sv.data = "";
			input_sv.count = 0;
		}
        sv_to_u64(before_dot, &input_num);
	
		// decimal overflow, round up
		if(input_sv.count > 9){
			INFO("Long Double Decimal Overflow detected for %s", input_str);
			String_View overflow = sv_chop_left(&input_sv,9);
			sv_to_u64(input_sv, &rounding_overflow);
			rounding_overflow = round(rounding_overflow / powl(10,input_sv.count) );
			sv_to_u64(overflow, &_decimal);
			_decimal += rounding_overflow;
			_decimal = _decimal / powl(10,overflow.count);
			input_num += _decimal;
			INFO("Rounding into %Lf", input_num);
		} else {
			// normal condition
			 sv_to_u64(input_sv, &_decimal);
			_decimal = _decimal / powl(10,input_sv.count);
			input_num += _decimal;
		}

		// multiple -1 for negative value
		// Sadly need to init a copy of input_sv as original so we can check here.
		if(sv_has_prefix(original, neg)){
			input_num = input_num * (-1);
		}
		buffer_write(&actual_buffer, &input_num, sizeof(long double));
	}

	// this part is specifically for output to screen, all the data inputs by user
	long double data_toscr = 0;
	fprintf(stdout, "\n");
	for(size_t i = 0; i < actual_buffer.size / sizeof(long double); ++i){
		buffer_read(&actual_buffer, i, &data_toscr);
		INFO("	  %18.9Lf", data_toscr);  // for debug
	}
	fprintf(stdout, "\n");

	// check Geometric Series
	if(checkGeometric(&actual_buffer, &wrong_data)){
		if (wrong_data.size > 0){
			INFO("Not a Geometric Series, wrong numbers are:");
			for(size_t i = 0; i < ((wrong_data.size)/2) / sizeof(long double); ++i){
				INFO("	  %18.9Lf  -->  %18.9Lf", wrong_data.data[i*2], wrong_data.data[i*2+1]);
			}
		} else {
			INFO("Is a Geometric Series, with");
			INFO("	  a = %18.9Lf", actual_buffer.data[0]);
			INFO("	  r = %18.9Lf", actual_buffer.data[1] / actual_buffer.data[0]);			
		}
	} else {
		WARN("Checking Geometric Series Failed");
	}


	// free the malloc	
	buffer_free(&actual_buffer);
	buffer_free(&wrong_data);
	return 0;
}