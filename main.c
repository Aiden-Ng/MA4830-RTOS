#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#include "function.c"
#include "graph.c" // this is to print the graph, Aiden
#include "UI_func.c" //this is to have the ui, daniel's part

#define PRECISION 1e-9

static Buffer actual_buffer = {0};
static Buffer wrong_data = {0};
bool convergence_flag = false;

char *shift(int *argc, char ***argv){
	assert(*argc > 0);
	char* result = **argv;
	*argv += 1;
	*argc -= 1;
	return result;
}

// compare 2 long double values, and ignore overflow
int isDoubleEqual(double a, double b){
	return fabs(a - b) < PRECISION;
}

// check whether is geometric series
int checkGeometric(Buffer *buffer, Buffer *wrong){
	long double error = 404;

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

	if (fabsl(r) < 1)
	{
		convergence_flag = true; //convergence
	}


	for(int i = 1; i < count; ++i){
		z = z * r;
		if(isDoubleEqual(z, buffer->data[i])){
		} else {
			buffer_write(wrong, &buffer->data[i], sizeof(long double));
			if(z > 1e20){
				buffer_write(wrong, &error, sizeof(long double));
			} else {
				buffer_write(wrong, &z, sizeof(long double));
			}
		};
	}
	return 1;
}

int main(int argc, char** argv)
{
	// argc = 8;
	// char * list[] = {"./program.exe","-i", "1", "2", "4", "8", "32", "64"};
	// argc = 2;
	// char * list[] = {"./program.exe","-u"};
	// argv = list;
	
	// for (int i = 0; i < argc; i++) {
    //     printf("argv[%d]: %s\n", i, argv[i]);
    // }

	const char *program = shift(&argc, &argv);

	// Why is this here? Duplicated with the loop?
	if(argc == 0){
		INFO("Usage: %s -i <int arrays>", program);
		PANIC("ERROR: No flag provided");
	}

	// this checks user flags
	const char *flag = shift(&argc, &argv);
	if(strcmp(flag, "-i") == 0){
		if(argc == 0){
			INFO("Usage: %s -i <integer Series>", program);
			INFO("Example 1:  ./geom -i 1 2 4 16 64");
			INFO("Example 2:  ./geom -i 1 -2.1 4.41 -9.261 ");
			PANIC("ERROR: No argument is provided for flag '%s'", flag);
		}
	} else if(strcmp(flag, "-u") == 0){
		INFO("%s\n", "UI mode");
		SeriesData_to_str(&argc, &argv);
	} else if(strcmp(flag, "-h") == 0){
		INFO("%s\n", "This is the help section");
		INFO("There are 2 modes available: \n");
		INFO("-i <integer series>");
		INFO("Example 1:  ./geom -i 1 2 4 16 64");
		INFO("Example 2:  ./geom -i 1 -2.1 4.41 -9.261\n");

		INFO("The UI Mode");
		INFO("./geom -u");

		exit(0);
	} else {
		INFO("Usage: %s -i <int arrays>", program);
		PANIC("ERROR: Unknown flag '%s'", flag);
	}
	char ** args_initial_ptr;
	args_initial_ptr = argv;

	//int temp_buffer_to_clear = argc; //holds a temporary placeholder for number of args beacuse we need to do a for loop at the end to clear release the buffer
	while(argc > 0){

		const char *input_str = shift(&argc, &argv);
		
		/*
			converts char array aka string
			lets say string = "This is a string"
			string_view =
				.data = "This is a string"
				.count = 16
		*/

		String_View input_sv = sv_from_cstr(input_str);
		String_View original = input_sv;  						// makes a copy of input_sv

		/*
			string_view neg =
				.data = "-"
				.count = 1
		*/

		String_View neg = sv_from_cstr("-");	
		long double _decimal, input_num, rounding_overflow = 0;

		/*
			search linearly until finding prefix. Prefix must be in front of sv.
			For example,  input = "-123"
				returns 1 if found prefix
				returns 0 if not found prefix
		*/
		if(sv_has_prefix(input_sv , neg)){
			sv_chop_by_delim(&input_sv, '-');
		}
		String_View before_dot = sv_chop_by_delim(&input_sv, '.');

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
		

		//  check if integer overflow  1_e19 is max
		//  If overflow, replace that number is 0
		if(before_dot.count > 19){
			INFO("Long Double Integer Overflow detected at %s", input_str);
			INFO("Replacing with 0 . . . . . . ");
			before_dot.data = "0";
			before_dot.count = 1;
			input_sv.data = "";
			input_sv.count = 0;
		}

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

        if(sv_to_u64(before_dot, &input_num)){
			buffer_write(&actual_buffer, &input_num, sizeof(long double));
			INFO("Detected non-digit in input %s", input_str);
			INFO("Replacing with 0 . . . . . ");
			continue;
		}
	
		/*
			decimal overflow, round up
			in our case, we take at most 9 decimals, if more than 9 decimals, 
				the 10th decimal onwards will be rounded up, then add to the 9 decimals
		
		*/

		/*
			how does sv_chop_left work ?
			similar to sv_chop_by_delim but instead of a char, we chop based on number

			input: 123456789

			overflow = sv_chop_left(input, 5)

			overwrite the input 				: 6789
			return value, stored in overflow	: 12345

		*/
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
			// if no overflow, run these
			sv_to_u64(input_sv, &_decimal);
			_decimal = _decimal / powl(10,input_sv.count);
			input_num += _decimal;
		}

		// multiple -1 for negative value
		// using original because input_sv has been modified
		if(sv_has_prefix(original, neg)){
			input_num = input_num * (-1);
		}


		/*  How does buffer_write work
			stores long double as bytes in an array
			
			before storing, need to realloc the old buffer so there is enough space for new data

			before exiting, need to free;
		*/

		buffer_write(&actual_buffer, &input_num, sizeof(long double));
		
		//free the memory here
		if(strcmp(flag, "-u") == 0){
			free((void*)input_str);
		}
			
	}

	// this part is specifically for output to screen, all the data inputs by user
	long double data_toscr = 0;
	// fprintf(stdout, "\n");
	for(size_t i = 0; i < actual_buffer.size / sizeof(long double); ++i){
		
		/*	How does buffer_read work ?
		
			buffer_read access the buffer like an array
		*/
		buffer_read(&actual_buffer, i, &data_toscr);
		// INFO("	  %18.9Lf", data_toscr);  // for debug
	}
	// fprintf(stdout, "\n");

	// check Geometric Series
	// wrong_data is a buffer for all the wrong datas
	if(checkGeometric(&actual_buffer, &wrong_data)){
		if (wrong_data.size > 0){
			INFO("Not a Geometric Series, wrong numbers are:");
			for(size_t i = 0; i < ((wrong_data.size)/2) / sizeof(long double); ++i){
				INFO("	  %29.9Lf  -->  %29.9Lf", wrong_data.data[i*2], wrong_data.data[i*2+1]);
			}
		} else {
			
			
			plot_series(actual_buffer.data, actual_buffer.size / sizeof(long double));
			INFO("Is a Geometric Series");
			if (convergence_flag){
				INFO("Type: 	Convergent Series");
			} else {
				INFO("Type: 	Divergent Series");
			}
			INFO("	  a = %18.9Lf", actual_buffer.data[0]);
			INFO("	  r = %18.9Lf", actual_buffer.data[1] / actual_buffer.data[0]);			
		}
	} else {
		WARN("Checking Geometric Series Failed");
	}

	// free the malloc	
	buffer_free(&actual_buffer);
	buffer_free(&wrong_data);
	
	if(strcmp(flag, "-u") == 0){
		free(args_initial_ptr); // free the malloced pointers for UI_func.c
	}

	return 0;
}