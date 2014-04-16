#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include "main.h"

typedef struct {
    char **cubes;
    int cols;
    int rows;
    int alloc_cols;
    int alloc_rows;
} matrix_t;

void free_matrix(matrix_t *matrix) {
    for (int i=0; i < matrix->alloc_rows; i++)
        free(matrix->cubes[i]);
    free(matrix->cubes);
    free(matrix);
}

static void replace_under_with_dash(char *cube, int num_cols)
{
    for (int i=0; i<num_cols; i++)
        if (cube[i] == '_') cube[i] = '-';
}

inline char all_dash(matrix_t *matrix)
{
    char has_all_dash = 0;

    for (int i=0; i < matrix->rows; i++) {

        has_all_dash = 1;
        for (int j=0; j < matrix->cols; j++) {
            if (matrix->cubes[i][j] == '1') has_all_dash = 0;
            if (matrix->cubes[i][j] == '0') has_all_dash = 0;
        }
        if (has_all_dash) break;
    }

    return has_all_dash;
}

int find_most_binate(matrix_t *matrix)
{
    int comp_form = 0;
    int true_form = 0;
    int min_column = 0;
    int min_difference = INT_MAX;

    for (int j=0; j<matrix->cols; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i=0; i<matrix->rows; i++) {
            if (matrix->cubes[i][j] == '1') true_form++;
            if (matrix->cubes[i][j] == '0') comp_form++;
        }

        if (true_form == 0 || comp_form == 0)
            continue;

        int difference = abs(true_form - comp_form);
        if (difference < min_difference) {
            min_difference = difference;
            min_column = j;
        }
    }

    return min_column;
}

matrix_t *co_factor(matrix_t *matrix, int column, char pc)
{
    matrix_t *temp_matrix = (matrix_t *)malloc(sizeof(matrix_t));

    temp_matrix->rows = matrix->rows;
    temp_matrix->cols = matrix->cols;
    temp_matrix->alloc_rows = matrix->rows;
    temp_matrix->cubes = (char **)malloc(matrix->rows*sizeof(char*));

    for (int i=0; i<matrix->rows; i++) {
        temp_matrix->cubes[i] = (char *)malloc(matrix->cols);
    }

    int row = 0;

    for (int i=0; i<matrix->rows; i++) {

        if (matrix->cubes[i][column] != pc && matrix->cubes[i][column] != '-') continue;

        memcpy(temp_matrix->cubes[row], matrix->cubes[i], matrix->cols);

        temp_matrix->cubes[row][column] = '-';

        row++;
    }

    temp_matrix->rows = row;

    return temp_matrix;
}

matrix_t *unate_reduction(matrix_t *matrix)
{
    matrix_t *temp_matrix = (matrix_t *)malloc(sizeof(matrix_t));

    temp_matrix->rows = matrix->rows;
    temp_matrix->cols = matrix->cols;
    temp_matrix->alloc_rows = matrix->rows;
    temp_matrix->cubes = (char **)malloc(matrix->rows*sizeof(char*));

    for (int i=0; i<matrix->rows; i++) {
        temp_matrix->cubes[i] = (char *)malloc(matrix->cols);
    }

    //
    // Find the unate columns
    //

    char *unate_columns = malloc(matrix->cols);

    int comp_form = 0;
    int true_form = 0;

    for (int j=0; j<matrix->cols; j++) {

        true_form = 0;
        comp_form = 0;

        // total up the number of 1s and 0s in this column
        for (int i=0; i<matrix->rows; i++) {
            if (matrix->cubes[i][j] == '1') true_form++;
            if (matrix->cubes[i][j] == '0') comp_form++;
        }

        if ((true_form == 0 && comp_form >= 0) ||
            (comp_form == 0 && true_form >= 0)) {
            unate_columns[j] = 1;
        } else {
            unate_columns[j] = 0;
        }
    }

    //
    // Figure out what rows to keep
    //

    char *keep_rows = malloc(matrix->rows);

    for (int i=0; i<matrix->rows; i++) {

        // assume all unate columns have dash unless proven otherwise
        char all_dash = 1;

        // check that all unate columns in this row are '-'
        for (int j=0; j<matrix->cols; j++) {
            if (!unate_columns[j]) continue;
            if (matrix->cubes[i][j] != '-')
                all_dash = 0;
        }

        if (all_dash)
            keep_rows[i] = 1;
        else
            keep_rows[i] = 0;
    }

    //
    //
    //

    int row = 0;
    int col = 0;
    for (int i=0; i<matrix->rows; i++) {
        if (!keep_rows[i]) continue;

        col = 0;
        for (int j=0; j<matrix->cols; j++) {
            if (unate_columns[j]) continue;
            temp_matrix->cubes[row][col] = matrix->cubes[i][j];
            col++;
        }

        row++;
    }

    temp_matrix->rows = row;
    temp_matrix->cols = col;

    free(keep_rows);
    free(unate_columns);

    return temp_matrix;
}

//http://cc.ee.ntu.edu.tw/~jhjiang/instruction/courses/fall10-lsv/lec03-2_2p.pdf
int check_tautology(matrix_t *matrix)
{
    /*
     positive cofactor (x = 1):
     [..1..] => remove from minterm from cube
     [..0..] => replace minterm with don't care (-)
     [..-..] => leave cube alone

     negative cofactor (x = 0):
     [..0..] => remove from minterm from cube
     [..1..] => replace minterm with don't care (-)
     [..-..] => leave cube alone
    */

    // check to see if we have run out of cubes
    if (matrix->rows == 0) return 0;

    // check to see if cube consists of all dashes, if so then we have a
    // tautology
    if (all_dash(matrix)) return 1;

    if (matrix->rows == 1 && !all_dash(matrix))
        return 0;

    matrix_t *reduced_matrix = unate_reduction(matrix);

    // pick most binate variable to check for tautology
    int binate_var = find_most_binate(reduced_matrix);

    matrix_t *C0 = co_factor(reduced_matrix, binate_var, '0');
    if (!check_tautology(C0)){
        free_matrix(reduced_matrix);
        free_matrix(C0);
        return 0;
    }

    matrix_t *C1 = co_factor(reduced_matrix, binate_var, '1');
    if (!check_tautology(C1)){
        free_matrix(reduced_matrix);
        free_matrix(C1);
        return 0;
    }

    free_matrix(C0);
    free_matrix(C1);
    free_matrix(reduced_matrix);

    return 1;
}

void *heur(void *filename)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    FILE *fp = fopen((const char *)filename, "r");
    if (fp == NULL) {
        printf("Could not open file!\n");
        pthread_cond_signal(&done_signal);
        return NULL;
    }

    matrix_t *matrix = (matrix_t *)malloc(sizeof(matrix_t));

    fscanf(fp, "%d", &matrix->cols);
    fscanf(fp, "%d", &matrix->rows);

    printf("Found %d variables and %d cubes\n", matrix->cols, matrix->rows);

    matrix->cubes = (char **)malloc(matrix->rows*sizeof(char*));

    matrix->alloc_rows = matrix->rows;

    for (int i=0; i<matrix->rows; i++) {
        matrix->cubes[i] = (char *)malloc(matrix->cols);
    }

    printf("Done allocating cubes\n");

    for (int i=0; i<matrix->rows; i++) {
        fscanf(fp, "%s", matrix->cubes[i]);
        replace_under_with_dash(matrix->cubes[i], matrix->cols);
    }

    printf("Done reading in file\n");

    fclose(fp);

    if (check_tautology(matrix)) {
        printf("Function is a tautololgy\n");
    } else {
        printf("Function is NOT a tautololgy\n");
    }

    free_matrix(matrix);

    printf("Heur found it\n");
    pthread_cond_signal(&done_signal);

    return NULL;
}