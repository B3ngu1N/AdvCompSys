/*
  This program uses AVX SIMD functions to parallelize a matrix-matrix multiplication.

  Compilation: g++ matrix_mult.cpp -mavx2 -o mult_avx.o

  @author Thomas Petr
  @author Ben Haft
  @date 3/30/2023
*/

#include "matrix_mult.h"

template <>
void Matrix<float>::printMatrix()
{
    int i, j;
    for(i=0; i < rows_; ++i){
        for(j=0; j < cols_; ++j){
            if (j != 0) { std::cout << " "; }
            std::cout << std::setw(4) << std::setfill(' ') << std::setprecision(3)
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


/*
  Partial Mult Helper Function - Does 8x8 dimension float multiplication
*/
void multHelper_8x8(const Matrix<float>& a, int i_A, int j_A,
                             const Matrix<float>& b, int i_B, int j_B,
                             Matrix<float>& c)
{
    int seg_breakup = 256/(sizeof(float)*8);
    __m256 sum_current_row = _mm256_setzero_ps();
    for (int i_a = i_A; i_a < i_A+seg_breakup; i_a++) { 
        __m256 a_ = _mm256_loadu_ps(&a(i_a, j_A)); // Get row of 8x elements from A
        float a_row[seg_breakup]; // 8x 32bit floats in 256 bits
        _mm256_storeu_ps(a_row, a_); // Breakup row of data into individual elements

        for (int i_b = i_B, a_row_itr=0; i_b < i_B+seg_breakup; i_b++, a_row_itr++) { // Go through rows of B
            __m256 dup_rVal = _mm256_set1_ps(a_row[a_row_itr]); // duplicate value from row to all values in __m256

            __m256 b_row = _mm256_loadu_ps(&b(i_b, j_B)); // load in row of B

            // Multiply every value of B row with the duplicated A row value, 
            // then add each value to the sums for the row. Each sum value is
            // the value to be put in the position of the 8x8 row value.
            sum_current_row = _mm256_add_ps(sum_current_row, _mm256_mul_ps(dup_rVal, b_row));
        }

        // Load in the information already at C(i_a, j_B) and add new information to it
        __m256 temp = _mm256_loadu_ps(&c(i_a, j_B)); // get current result at c(i_a,j_B)
        sum_current_row = _mm256_add_ps(sum_current_row, temp); // add current result to new results
        _mm256_storeu_ps(&c(i_a, j_B), sum_current_row); // all results for the current row of 8x output elements
        sum_current_row = _mm256_setzero_ps(); // reset the sums to 0.0
    }
}

/*
Intel AVX Instruction-Based Matrix Multiplication for float data-type
*/
Matrix<float> operator*(const Matrix<float>& a, const Matrix<float>& b)
{
    Matrix<float> c(a.Rows());  // Assuming square matrix

    int seg_breakup = 256/(sizeof(float)*8);
    int num_256xBlock = ceil((float)c.Rows()/seg_breakup); // find the number of 256 bit segments
    
    // Must use Tiling with 8x8 Matrices
    // i & j correspond to tile/block of 8x8 matrix locations
    for(int i = 0; i < num_256xBlock; i++) {
        for(int j = 0; j < num_256xBlock; j++) {
            for (int k = 0; k < num_256xBlock; k++) {
                multHelper_8x8(a,i*seg_breakup,k*seg_breakup, b,k*seg_breakup,j*seg_breakup, c);
            }
        }
    }

    return c;
}

