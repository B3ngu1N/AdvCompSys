#include "matrix_mult.h"

int main(){
    // Square Matrix Dimensions
    int matrix_dim = 2;

    // Setup of Matrix A and Matrix B
    // Values calculated with random float function
    int i, j;
    Matrix<float> A = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
        for( j = 0;  j < matrix_dim; ++j) {
        A.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
        }
    }

    A.printMatrix();
    std::cout << std::endl;

    A(0,0) += 1.0;

    A.printMatrix();
    std::cout << std::endl;

    return 0;
}