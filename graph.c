#ifndef  __GRAPH_C__
#define __GRAPH_C__

//This is to plot the graph, additional file on top of fudens
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> //this is for realloc
#include <math.h>   //this is for fabs

long double y_val;

// ---------------- S i g n a t u r e ----------------
int PLOT_HEIGHT = 20;
void plot_series(long double terms[], int n) {
    if (n == 0) return;

    long double max_abs = 0.0;
    for (int i = 0; i < n; i++) {
        long double abs_val = fabsl(terms[i]); //getting the absolute values
        if (abs_val > max_abs) max_abs = abs_val; //resetting the max values
    }

    if (max_abs == 0.0) {
        printf("\nAll terms are zero - no plot available\n");
        return;
    }

    // Initialize plot grid
    char plot_grid[PLOT_HEIGHT][n];
    for (int row = 0; row < PLOT_HEIGHT; row++) {
        for (int col = 0; col < n; col++) {
            plot_grid[row][col] = ' ';
        }
    }

    // Calculate plot positions
    for (int i = 0; i < n; i++) {
        long double scaled_y = ((terms[i] + max_abs) / (2 * max_abs)) * (PLOT_HEIGHT - 1);
        int row = (int)(scaled_y + 0.5);
        if (row < 0) row = 0;
        if (row >= PLOT_HEIGHT) row = PLOT_HEIGHT - 1;
        plot_grid[row][i] = '*';
    }

    // how to i get the max size of the y_val
    for (int row = PLOT_HEIGHT - 1; row >= 0; row--) {
        y_val = ((row / (long double)(PLOT_HEIGHT-1)) * 2 * max_abs - max_abs);
    }
    // Print plot
    printf("\nSeries Plot (max=%.2Lf):\n", max_abs);
    for (int row = PLOT_HEIGHT - 1; row >= 0; row--) {
        y_val = ((row / (long double)(PLOT_HEIGHT-1)) * 2 * max_abs - max_abs);
        int spacing = (int)log10(max_abs) + 5; //5 because u need to include the negative, dot and decimals
        printf("%*.2Lf | ",spacing, y_val); 
        for (int col = 0; col < n; col++) {
            printf("%c ", plot_grid[row][col]);
        }
        printf("\n");
    }

    // Print x-axis
    printf("       +");
    for (int col = 0; col < n; col++) printf("--");
    printf("\n        ");
    for (int col = 0; col < n; col++) printf("%-2d", col+1);
    printf("\n");
}

#endif