#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_matrix(double** matrix, int size) {
    for (int i = 0; i < size; i++) {
        printf("\n");
        for (int j = 0; j < size; j++)
            printf("%0.3f ", matrix[i][j]);
    }
    printf("\n");

}

// Функция для вычисления обратной матрицы
void inverseMatrix(double** initial_matrix, int size) {
    // Создаем расширенную матрицу, объединяя исходную матрицу и единичную
    double** augmented_matrix = (double**)calloc(size, sizeof(double*));

    if (augmented_matrix) {

        for (int i = 0; i < size; ++i) {
            augmented_matrix[i] = (double*)calloc(2 * size, sizeof(double));

            if (augmented_matrix[i])
                for (int j = 0; j < size; ++j) {
                    augmented_matrix[i][j] = initial_matrix[i][j];
                    // Инициализируем единичную матрицу справа
                    augmented_matrix[i][j + size] = (i == j) ? 1.0 : 0.0;
                }
        }

        // Применяем метод Гаусса-Жордана
        for (int i = 0; i < size; ++i) {

            if (augmented_matrix[i]) {
                // Делаем главный элемент равным 1
                double pivot = augmented_matrix[i][i];
                for (int j = 0; j < 2 * size; ++j)
                    augmented_matrix[i][j] /= pivot;
                // Обнуляем остальные элементы в столбце
                for (int k = 0; k < size; ++k)
                    if (k != i) {
                        double factor = augmented_matrix[k][i];
                        for (int j = 0; j < 2 * size; ++j) {
                            double tmp = augmented_matrix[k][j];
                            augmented_matrix[k][j] -= factor * augmented_matrix[i][j];

                        }
                        //print_matrix(augmented_matrix, size);
                    }
            }
        }

        // Извлекаем обратную матрицу из расширенной матрицы
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                if (augmented_matrix[i])
                    initial_matrix[i][j] = augmented_matrix[i][j + size];

        // Освобождаем память
        for (int i = 0; i < size; ++i) free(augmented_matrix[i]);
        free(augmented_matrix);
    }
}


int main() {
    srand(time(NULL));

    int size, choise;
    double** matrix;

    printf("Choose way of entering matrix\n1 - Manual\n2 - Random\n>");
    char z = scanf("%d", &choise);
    printf("Enter size of matrix.\n>");
    z = scanf("%d", &size);

    if (choise == 1) {
        matrix = (double**)calloc(size, sizeof(double*));

        if (matrix != NULL)
            for (int i = 0; i < size; i++) {
                matrix[i] = (double*)calloc(2 * size, sizeof(double));
                for (int j = 0; j < size; j++) {
                    printf("A[%d][%d]=", i + 1, j + 1);
                    if (matrix[i] != NULL)
                        z = scanf("%lf", &matrix[i][j]);
                }
            }
    }
    else if (choise == 2) {
        matrix = (double**)calloc(size, sizeof(double*));

        if (matrix != NULL)
            for (int i = 0; i < size; i++) {
                matrix[i] = (double*)calloc(2 * size, sizeof(double));
                for (int j = 0; j < size; j++) {
                    if (matrix[i] != NULL)
                        matrix[i][j] = (double)(rand() % 10);
                }
            }
    }
    else {
        printf("ERROR");
        return 0;
    }

    clock_t start_time = clock();

    inverseMatrix(matrix, size);

    clock_t end_time = clock(); // конечное время
    printf("\nTime %f seconds\n", (float)(end_time - start_time) / CLOCKS_PER_SEC);

    printf("\nPress '1' to see inverse matrix. If you don't want to see it, press any other key.\n>");
    z = scanf("%d", &choise);
    if (choise == 1) print_matrix(matrix, size);

    if (matrix) {
        for (int i = 0; i < size; i++) free(matrix[i]);
        free(matrix);
    }

    return 0;
}