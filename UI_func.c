#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 100

// Structure to hold an array of strings and the count
typedef struct {
    char **terms;  // Array of strings
    int n;         // Number of terms entered
} SeriesData;

// Function that processes user input and stores each term as a string
SeriesData process_series(void) {
    SeriesData data;
    data.n = 0;
    int capacity = 10;
    
    // Allocate memory for the array of string pointers
    data.terms = malloc(capacity * sizeof(char *));
    if (!data.terms) {
        printf("Memory allocation failed.\n");
        return data;
    }

    char input[INPUT_BUFFER_SIZE];

    printf("Enter terms of the series ('e' to finish):\n");
    while (1) {
        printf("Term %d: ", data.n + 1);
        if (scanf("%99s", input) != 1) {
            printf("Input error!\n");
            break;
        }

        // Check for termination command "e"
        if (strcmp(input, "e") == 0) {
            if (data.n < 2) {
                printf("At least 2 terms required! Please continue.\n");
                continue;
            }
            break;
        }
// --------- D E B U G G I N G ---------
        // Validate the input as a number by attempting to convert it
        char *endptr;
        double value = strtod(input, &endptr);
        if (*endptr != '\0') {
            printf("Invalid input '%s'! Please enter a number or 'e'.\n", input);
            continue;
        }

        // For the first term, ensure it's not zero
        if (data.n == 0 && value == 0) {
            printf("Error: First term cannot be zero. Please enter a non-zero value.\n");
            continue;
        }
// --------- D E B U G G I N G ---------

        // Resize the array if necessary
        if (data.n >= capacity) {
            capacity *= 2;
            char **temp = realloc(data.terms, capacity * sizeof(char *));
            if (!temp) {
                printf("Memory reallocation failed.\n");
                for (int i = 0; i < data.n; i++) {
                    free(data.terms[i]);
                }
                free(data.terms);
                data.terms = NULL;
                data.n = 0;
                return data;
            }
            data.terms = temp;
        }

        // Duplicate the input string and save it into the array
        data.terms[data.n] = strdup(input);
        if (!data.terms[data.n]) {
            printf("Memory allocation for term failed.\n");
            for (int i = 0; i < data.n; i++) {
                free(data.terms[i]);
            }
            free(data.terms);
            data.terms = NULL;
            data.n = 0;
            return data;
        }
        data.n++;
    }

    return data;
}

//converting it to string
int SeriesData_to_str(int *argc, char ***argv) {
    // Retrieve the series data (array of strings and count)
    SeriesData series = process_series();

    // Check if series data is valid
    if (series.terms == NULL || series.n <= 0) {
        return 0;
    }

    // Allocate memory for the array of string pointers
    char **result = (char **)malloc(series.n * sizeof(char *));
    if (!result) {
        printf("Memory allocation for result array failed.\n");
        return 0;
    }

    // Copy each term into the result array
    for (int i = 0; i < series.n; i++) {
        // Allocate memory for each term (+1 for null terminator)
        result[i] = (char *)malloc(strlen(series.terms[i]) + 1);
        if (!result[i]) {
            // Free all previously allocated memory on failure
            printf("Memory allocation for term %d failed.\n", i);
            for (int j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            return 0;
        }
        strcpy(result[i], series.terms[i]);
    }

    // Clean up the original series data
    for (int i = 0; i < series.n; i++) {
        free(series.terms[i]);
    }
    free(series.terms);
    *argv = result; //setting the values
    *argc = series.n; //setting the values
    return 1; //if 1 success
}

// int main() 
// {
//     SeriesData_to_str();
//     return 0;

// }

