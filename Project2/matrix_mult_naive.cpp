/*
  This program does a naive matrix-matrix multiplication.

  Compilation: g++ matrix_mult_naive.cpp -o mult_naive.o

  @author Thomas Petr
  @author Ben Haft
  @date 2/4/2023
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <bits/stdc++.h>

template <typename T>
class Matrix {
 public:
 // Need to add padding to allocation to allow for 0s at end of rows and columns
 // to be a multiple of 256 bits
  Matrix(int matrix_dim) : rows_(matrix_dim), cols_(matrix_dim) {
    // choose SSE or AVX depending on input type
    int avx_or_sse = 256;
    if(sizeof(T) < 4) avx_or_sse = 128;
    int segment_breakup = avx_or_sse/(sizeof(T)*8);
    int padding_dim = ceil((float)matrix_dim/segment_breakup) * segment_breakup;
    data_ = new T[padding_dim * padding_dim];
    memset(data_, (T)0.0, sizeof(T) * padding_dim * padding_dim);
    rows_padding = padding_dim;
    cols_padding = padding_dim;
  }
  
  ~Matrix() {
    delete[] data_;
  }

  int Rows() const { return rows_; }
  int Cols() const { return cols_; }
  int RowsPad() const { return rows_padding; }
  int ColsPad() const { return cols_padding; }
  T* Data() { return data_; }
  const T* Data() const { return data_; }

  T& operator()(int i, int j) { return data_[i * cols_padding + j]; }
  const T& operator()(int i, int j) const { return data_[i * cols_padding + j]; }

  void setVal(int i, int j, T val) { data_[i * cols_padding + j] = val; }
  void printMatrix();
  void printMatrixWithPad();

 private:
  int rows_;
  int cols_;
  int rows_padding;
  int cols_padding;
  T* data_;
};


template <>
void Matrix<float>::printMatrix()
{
  int i, j;
  for(i=0; i < rows_; ++i){
    for(j=0; j < cols_; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(7) << std::setfill(' ') << std::setprecision(4)
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

template <>
void Matrix<short>::printMatrix()
{
  int i, j;
  for(i=0; i < rows_; ++i){
    for(j=0; j < cols_; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(5) << std::setfill(' ')
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

template <>
void Matrix<float>::printMatrixWithPad()
{
  int i, j;
  for(i=0; i < rows_padding; ++i){
    for(j=0; j < cols_padding; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(5) << std::setfill(' ') << std::setprecision(2)
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

template <>
void Matrix<short>::printMatrixWithPad()
{
  int i, j;
  for(i=0; i < rows_padding; ++i){
    for(j=0; j < cols_padding; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(5) << std::setfill(' ') 
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

/*
   Naive Matrix Multiplication Implementation
*/
Matrix<float> operator*(const Matrix<float>& a, const Matrix<float>& b) {
  Matrix<float> c(a.Rows());  // Assuming square matrix
  for (int i = 0; i < a.Rows(); i++) { // iterate through rows of a
    for (int j = 0; j < b.Cols(); j++) { // iterate through columns of b
      for (int k = 0; k < c.Cols(); k++) {
        c.setVal(i, j, c(i, j)+a(i, k)*b(k, j) );
      }
    }
  }
  return c;
}

Matrix<short> operator*(const Matrix<short>& a, const Matrix<short>& b) {
  Matrix<short> c(a.Rows());  // Assuming square matrix
  for (int i = 0; i < a.Rows(); i++) { // iterate through rows of a
    for (int j = 0; j < b.Cols(); j++) { // iterate through columns of b
      for (int k = 0; k < c.Cols(); k++) {
        c.setVal(i, j, c(i, j)+a(i, k)*b(k, j) );
      }
    }
  }
  return c;
}

int main(int argc, const char** argv)
{
  clock_t start, end;
  const char* const exeName = argv[0]; // Name of file to compress

  if (argc != 3) { 
      printf("Wrong Arguments\n");
      printf("%s MATRIX_DIM DATA_TYPE\n", exeName);
      printf("DATA_TYPE = i (short) or f (float)\n");
      return 1;
  }

  // Square Matrix Dimensions
  int matrix_dim = atoi(argv[1]);
  std::string argv2 = argv[2];

  if(argv2 == "f")
  {
    // Setup of Matrix A and Matrix B
    // Values calculated with random float function
    int i, j;
    Matrix<float> A = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        A.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
      }
    }
    Matrix<float> B = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        B.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
      }
    }

    // Print out initial matrices (if dim < 20)
    if(matrix_dim <= 20)
    {
      A.printMatrix();
      std::cout << std::endl;
      B.printMatrix();
      std::cout << std::endl;
    }

    // Timer Start
    start = clock();

    // Matrix multiplication
    Matrix<float> C = A * B;

    // Timer End
    end = clock();

    // Print out final multiplied matrix
    if(matrix_dim <= 20) {
      C.printMatrix();
      std::cout << std::endl;
    }

    // Calculating total time taken by the program.
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by SIMD matrix multiplication is : " << std::fixed
          << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;
  }

  else if(argv2 == "i")
  {
    // Setup of Matrix A and Matrix B
    // Values calculated with random function
    int i, j;
    Matrix<short> A = Matrix<short>(matrix_dim); //value cap of a short is 65535
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        A.setVal(i, j, (short)(rand() % 10)); // float between 0 and 9
      }
    }
    Matrix<short> B = Matrix<short>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        B.setVal(i, j, (short)(rand() % 10)); // float between 0 and 9
      }
    }

    // Print out initial matrices (if dim < 20)
    if(matrix_dim <= 20)
    {
      A.printMatrix();
      std::cout << std::endl;
      B.printMatrix();
      std::cout << std::endl;
    }

    // Timer Start
    start = clock();

    // Matrix multiplication
    Matrix<short> C = A * B;

    // Timer End
    end = clock();

    // Print out final multiplied matrix
    if(matrix_dim <= 20) {
      C.printMatrix();
      std::cout << std::endl;
    }

    // Calculating total time taken by the program.
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by SIMD matrix multiplication is : " << std::fixed
          << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;
  }
  else{
    std::cout << "DATA_TYPE Input is Invalid - Try 'i' or 'f'" << std::endl;
  }

  if(matrix_dim > 20) {
    std::cout << "Did not print out matrices because matrix dimension >20." 
              << std::endl;
  }

  return 0;
}