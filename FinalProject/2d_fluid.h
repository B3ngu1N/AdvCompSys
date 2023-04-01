/*
    2D Fluid Simulation with Navier-Stokes Equations Header File

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/
#include "matrix_mult.h"
#include <stdlib.h>

#define ITR 16
#define N 256

class Fluid2D {
  public:
    Fluid2D(int sim_dimension, float diffusion, float viscosity, float dt_) : 
            size(sim_dimension), dt(dt_), diff(diffusion), visc(viscosity) {
        this->Vx = Matrix<float>(sim_dimension);
        this->Vy = Matrix<float>(sim_dimension);

        this->Vx0 = Matrix<float>(sim_dimension);
        this->Vy0 = Matrix<float>(sim_dimension);

        this->s = Matrix<float>(sim_dimension);
        this->density = Matrix<float>(sim_dimension);
    }
    
    ~Fluid2D() {}

    void AddDensityScalar(int x, int y, float amount);

    void AddVelocityScalar(int x, int y, float amountX, float amountY);

    void SimStep();

  private:
    int size;
    float dt;
    float diff;
    float visc;

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

