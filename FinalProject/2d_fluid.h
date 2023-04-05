/*
    2D Fluid Simulation with Navier-Stokes Equations Header File

    Credit - Jos Stam & Mike Ash

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/

#define ITR 16
#define N 256
#define SCALE 4

#include "matrix_mult.h"
#include "p8g.hpp"

class Fluid2D {
  public:
    Fluid2D(int sim_dimension, float diffusion, float viscosity, float dt_);
    
    ~Fluid2D() {}

    void AddDensity(int x, int y, float amount);

    void AddVelocity(int x, int y, float amountX, float amountY);

    void SimStep();

    void RenderDensity();

    Matrix<float>& rxVx() {
        return this->Vx;
    }

    Matrix<float>& rxVy() {
        return this->Vy;
    }

    Matrix<float>& rxD() {
        return this->density;
    }

    float* rxtmp() {
        return this->tmp;
    }

  private:
    int size;
    float dt;
    float diff;
    float visc;

    float* tmp;

    Matrix<float> Vx;
    Matrix<float> Vy;

    Matrix<float> Vx0;
    Matrix<float> Vy0;

    Matrix<float> s;
    Matrix<float> density;

};


void SetBoundaries(int b, Matrix<float>& in_x);

void LinSolve(int b, Matrix<float>& in_x, Matrix<float>& in_x0, float a, float c);

void Diffuse(int b, Matrix<float>& in_x, Matrix<float>& in_x0, float in_diff, float in_dt);

void Project(Matrix<float>& in_Vx, Matrix<float>& in_Vy, 
              Matrix<float>& p, Matrix<float>& div);

void Advect(int b, Matrix<float>& d, Matrix<float>& d0, Matrix<float>& in_Vx, 
            Matrix<float>& in_Vy, float in_dt);


