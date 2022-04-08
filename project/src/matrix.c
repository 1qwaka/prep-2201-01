#include "matrix.h"
#include <stdlib.h>

#define TYPE_T double

// выделение памяти для двумерного массива здорового человека
// static void* alloc_table(size_t rows, size_t cols) {
//     TYPE_T **table = calloc(rows, sizeof(TYPE_T *));

//     if (table == NULL)
//         return NULL;

//     for (size_t i = 0; i < rows; ++i) {
//         table[i] = calloc(sizeof(TYPE_T), cols);

//         if (table[i] == NULL) {
//             for (int j = i - 1; j >= 0; --j) {
//                 free(table[j]);
//             }
//             free(table);
//             return NULL;
//         }
//     }

//     return table;
// }

// выделение памяти для двумерного массива без брейков и ретурнов в середине функции
// static void* alloc_table(size_t rows, size_t cols) {
//     TYPE_T **table = calloc(rows, sizeof(TYPE_T *));
//     int flag = 0;

//     if (table != NULL) {
//         for (size_t i = 0; i < rows && flag == 0; ++i) {
//             table[i] = calloc(sizeof(TYPE_T), cols);

//             if (table[i] == NULL) {
//                 flag = i;
//             }
//         }

//         if (flag > 0) {
//             for (int j = flag - 1; j >= 0; --j) {
//                 free(table[j]);
//             }
//             free(table);
//             table = NULL;
//         }
//     }

//     return table;
// }


static void* alloc_table(size_t rows, size_t cols) {
    TYPE_T **table = calloc(rows, sizeof(TYPE_T *));
    // выделяю в строчку потому что так удобнее и уменьшает шанс фрагментации кучи (наверное)
    void *table_mem = calloc(rows * cols, sizeof(TYPE_T));
    
    if (table != NULL && table_mem != NULL) {
        for (size_t i = 0; i < rows; ++i) {
            table[i] = table_mem + i * cols * sizeof(TYPE_T);
        }
    } else {
        if (table != NULL) {
            free(table);
            table = NULL;
        }
        if (table_mem != NULL) {
            free(table_mem);
            table_mem = NULL;
        }
    }
    
    return table;
}

static void free_table(TYPE_T **table, size_t rows) {
    for (size_t i = 0; i < rows; ++i)
    {
        free(table[i]);
    }
    free(table);
}

Matrix* create_matrix(size_t rows, size_t cols) {
    
}

void free_matrix(Matrix* matrix) {
    if (matrix == NULL || matrix->table == NULL)
        return;
    free_table(matrix->table, matrix->rows);
    free(matrix);
}