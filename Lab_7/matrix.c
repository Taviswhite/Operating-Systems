#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 20          // largest matrix supported (20 x 20)
#define NUM_THREADS 10   // total number of worker threads

int matA[SIZE][SIZE]; 
int matB[SIZE][SIZE]; 

int matSumResult[SIZE][SIZE];
int matDiffResult[SIZE][SIZE]; 
int matProductResult[SIZE][SIZE]; 

int MAX = SIZE;          // actual matrix size used (set from the command line)

void fillMatrix(int matrix[SIZE][SIZE]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            matrix[i][j] = rand()%10+1;
        }
    }
}

void printMatrix(int matrix[SIZE][SIZE]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Thread i handles every row r where r % NUM_THREADS == i.
// With a 20 x 20 matrix and 10 threads, each thread gets 2 rows.

// Sets each cell in this thread's rows of matSumResult
// to the sum of the matching cells of matA and matB.
void* computeSum(void* args) { // pass in the number of the ith thread
    int id = *(int *)args;
    for (int i = id; i < MAX; i += NUM_THREADS)
        for (int j = 0; j < MAX; j++)
            matSumResult[i][j] = matA[i][j] + matB[i][j];
    return NULL;
}

// Sets each cell in this thread's rows of matDiffResult
// to the difference of the matching cells of matA and matB.
void* computeDiff(void* args) { // pass in the number of the ith thread
    int id = *(int *)args;
    for (int i = id; i < MAX; i += NUM_THREADS)
        for (int j = 0; j < MAX; j++)
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
    return NULL;
}

// Sets each cell in this thread's rows of matProductResult
// to the inner product of row i of matA and column j of matB.
void* computeProduct(void* args) { // pass in the number of the ith thread
    int id = *(int *)args;
    for (int i = id; i < MAX; i += NUM_THREADS)
        for (int j = 0; j < MAX; j++) {
            int total = 0;
            for (int k = 0; k < MAX; k++)
                total += matA[i][k] * matB[k][j];
            matProductResult[i][j] = total;
        }
    return NULL;
}

// Each of the 10 threads computes its share of all three results,
// then frees the memory holding its thread number.
void* computeAll(void* args) {
    computeSum(args);
    computeDiff(args);
    computeProduct(args);
    free(args);
    return NULL;
}

// 10 threads total; each thread fills 2 rows of each result matrix.
int main(int argc, char *argv[]) {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX
    if (argc > 1) {
        MAX = atoi(argv[1]);
        if (MAX < 1 || MAX > SIZE) {
            printf("Matrix size must be between 1 and %d\n", SIZE);
            return 1;
        }
    }

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads.
    pthread_t threads[NUM_THREADS];

    // 4. Create the threads, passing each one its thread number.
    for (int i = 0; i < NUM_THREADS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, computeAll, id);
    }

    // 5. Wait for all threads to finish.
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
    return 0;
}
