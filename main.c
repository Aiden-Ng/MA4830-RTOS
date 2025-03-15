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
#include "graph.c" 
#include "UI_func.c" 

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
		if(a == 0) WARN("Detected first value is 0");
		if(b == 0) WARN("Detected second value is 0");
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
	const char *program = shift(&argc, &argv);

	if(argc == 0){
		INFO("Usage: %s -i <int arrays>", program);
		PANIC("ERROR: No flag provided");
	}

	// this checks user flags <> -i, -u, -h
	const char *flag = shift(&argc, &argv);
	if(strcmp(flag, "-i") == 0){ //immediate input mode
		if(argc == 0){
			INFO("Usage: %s -i <integer Series>", program);
			INFO("Example 1:  ./geom -i 1 2 4 16 64");
			INFO("Example 2:  ./geom -i 1 -2.1 4.41 -9.261 ");
			PANIC("ERROR: No argument is provided for flag '%s'", flag);
		}
	} else if(strcmp(flag, "-u") == 0){ //user input mode
		INFO("%s\n", "UI mode");
		SeriesData_to_str(&argc, &argv);

	} else if(strcmp(flag, "-h") == 0){ //help section
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
	
	char ** args_initial_ptr; //this is to initialize the pointer for freeing the malloc for -u mode at the end of the whole code
	args_initial_ptr = argv; //the reason for doing this because malloc is used to create storage for -u mode
	while(argc > 0){

		const char *input_str = shift(&argc, &argv);

		String_View input_sv = sv_from_cstr(input_str);
		String_View original = input_sv;  // makes a copy of input_sv
		String_View neg = sv_from_cstr("-");	
		long double _decimal, input_num, rounding_overflow, zero = 0;
	
		if(sv_has_prefix(input_sv , neg)){
			sv_chop_by_delim(&input_sv, '-');
		}
		String_View before_dot = sv_chop_by_delim(&input_sv, '.');

		//  check if integer overflow  1_e19 is max
		//  If overflow, replace that number is 0
		if(before_dot.count > 19){
			INFO("Detected Long Double Integer Overflow at %s", input_str);
			INFO("Replacing with 0 . . . . . . ");
			before_dot.data = "0";
			before_dot.count = 1;
			input_sv.data = "";
			input_sv.count = 0;
		}

        if(sv_to_u64(before_dot, &input_num)){
			buffer_write(&actual_buffer, &zero, sizeof(long double));
			INFO("Detected non-digit in input %s", input_str);
			INFO("Replacing with 0 . . . . . ");
			continue;
		}
	
		if(input_sv.count > 9){ // decimal overflow, round up, we take at most 9 decimals, if more than 9 decimals, the 10th decimal onwards will be rounded up, then add to the 9 decimals
			INFO("Detected Long Double Decimal Overflow at %s", input_str);
			String_View overflow = sv_chop_left(&input_sv,9);
			if(sv_to_u64(input_sv, &rounding_overflow)){
				buffer_write(&actual_buffer, &zero, sizeof(long double));
				INFO("Detected non-digit in input %s", input_str);
				INFO("Replacing with 0 . . . . . ");
				continue;
			}
			rounding_overflow = round(rounding_overflow / powl(10,input_sv.count) );
			if(sv_to_u64(overflow, &_decimal)){
				buffer_write(&actual_buffer, &zero, sizeof(long double));
				INFO("Detected non-digit in input %s", input_str);
				INFO("Replacing with 0 . . . . . ");
				continue;
			}
			_decimal += rounding_overflow;
			_decimal = _decimal / powl(10,overflow.count);
			input_num += _decimal;
			INFO("Rounding into %Lf", input_num);
		} else {
			// if no overflow, run these
			if(sv_to_u64(input_sv, &_decimal)){
				buffer_write(&actual_buffer, &zero, sizeof(long double));
				INFO("Detected non-digit in input %s", input_str);
				INFO("Replacing with 0 . . . . . ");
				continue;
			}
			_decimal = _decimal / powl(10,input_sv.count);
			input_num += _decimal;
		}

		// multiple -1 for negative value
		// using original because input_sv has been modified
		if(sv_has_prefix(original, neg)){
			input_num = input_num * (-1);
		}

		buffer_write(&actual_buffer, &input_num, sizeof(long double));
		
		//free the malloc allocated for the -u mode
		if(strcmp(flag, "-u") == 0){
			//free the memory here
			free((void*)input_str);
		}
	}

	// this part is specifically for output to screen, all the data inputs by user
	long double data_toscr = 0;
	// fprintf(stdout, "\n");
	for(size_t i = 0; i < actual_buffer.size / sizeof(long double); ++i){
		
		buffer_read(&actual_buffer, i, &data_toscr);
		//INFO("	  %18.9Lf", data_toscr);  // for debug
	}
	fprintf(stdout, "\n"); // please do not comment this

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
		free(args_initial_ptr); //free the malloced pointers for UI_func.c
	}

	return 0;
}