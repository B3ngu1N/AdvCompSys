/*
    2D Fluid Simulation with Navier-Stokes Equations

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/

#include "2d_fluid.h"

void Fluid2D::AddDensityScalar(int x, int y, float amount)
{
    this->density(x, y) += amount;
}

void Fluid2D::AddVelocityScalar(int x, int y, float amountX, float amountY)
{
    this->Vx(x, y) += amountX;
    this->Vy(x, y) += amountY;
}

void Fluid2D::SimStep()
{
    Diffuse(1, this->Vx0, this->Vx, this->visc, this->dt);
    Diffuse(2, this->Vy0, this->Vy, this->visc, this->dt);

    Project(this->Vx0, this->Vy0, this->Vx, this->Vy);

    Advect(1, this->Vx, this->Vx0, this->Vx0, this->Vy0, this->dt);
    Advect(2, this->Vy, this->Vy0, this->Vx0, this->Vy0, this->dt);

    Project(this->Vx, this->Vy, this->Vx0, this->Vy0);
    Diffuse(0, this->s, this->density, this->diff, this->dt);
    Advect(0, this->density, this->s, this->Vx, this->Vy, this->dt);
}

void SetBoundaries(int b, Matrix<float>& in_x)
{

}

void LinSolve(int b, Matrix<float>& in_x, Matrix<float>& in_x0, float a, float c)
{
    float cRecip = 1.0 / c;
    for (int t = 0; t < ITR; t++) {
        for (int j = 1; j < N-1; j++) {
            for (int i = 1; i < N-1; i++) {
                in_x(i, j) = (in_x0(i, j) +
                                a * (in_x(i+1, j) +
                                    in_x(i-1, j) +
                                    in_x(i, j+1) +
                                    in_x(i, j-1))) * 
                                cRecip;
            }
        }
        SetBoundaries(b, in_x);
    }
}

void Diffuse(int b, Matrix<float>& in_x, Matrix<float>& in_x0, float in_diff, float in_dt)
{
    float a = in_dt * in_diff * (N - 2) * (N - 2);
    LinSolve(b, in_x, in_x0, a, 1 + 6 * a);
}

void Project(Matrix<float>& in_Vx, Matrix<float>& in_Vy, Matrix<float>& p, Matrix<float>& div)
{
    for (int j = 1; j < N-1; j++) {
        for (int i = 1; i < N-1; i++) {
            div(i, j) = (-0.5 *
                            (in_Vx(i+1, j) -
                            in_Vx(i-1, j) +
                            in_Vy(i, j+1) -
                            in_Vy(i, j-1))) / N;
            p(i, j) = 0.0;
        }
    }

    SetBoundaries(0, div);
    SetBoundaries(0, p);
    LinSolve(0, p, div, 1, 6);

    for (int j = 1; j < N-1; j++) {
        for (int i = 1; i < N-1; i++) {
            in_Vx(i, j) -= 0.5 * (p(i+1, j) - p(i-1, j)) * N;
            in_Vy(i, j) -= 0.5 * (p(i, j+1) - p(i, j-1)) * N;
        }
    }

    SetBoundaries(1, in_Vx);
    SetBoundaries(2, in_Vy);
}

void Advect(int b, Matrix<float>& d, Matrix<float>& d0, Matrix<float>& in_Vx, Matrix<float>& in_Vy, float in_dt)
{

}




