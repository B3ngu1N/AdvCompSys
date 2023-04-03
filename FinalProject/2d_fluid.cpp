/*
    2D Fluid Simulation with Navier-Stokes Equations - Currently not Multithreaded/SIMD

    Credit - Jos Stam & Mike Ash

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/

#include "2d_fluid.h"

void Fluid2D::AddDensity(int x, int y, float amount)
{
    this->density(x, y) += amount;
}

void Fluid2D::AddVelocity(int x, int y, float amountX, float amountY)
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

void Fluid2D::RenderDensity()
{
    
}

void SetBoundaries(int b, Matrix<float>& in_x)
{
    for (int i = 1; i < N-1; i++) {
        in_x(i, 0) = b == 2 ? -in_x(i, 1) : in_x(i, 1);
        in_x(i, N-1) = b == 2 ? -in_x(i, N-2) : in_x(i, N-2);
    }

    for (int j = 1; j < N-1; j++) {
        in_x(0, j) = b == 1 ? -in_x(1, j) : in_x(1, j);
        in_x(N-1, j) = b == 1 ? -in_x(N-2, j) : in_x(N-2, j);
    }

    in_x(0, 0) = 0.5 * (in_x(1, 0) + in_x(0, 1));
    in_x(0, N-1) = 0.5 * (in_x(1, N-1) + in_x(0, N-2));
    in_x(N-1, 0) = 0.5 * (in_x(N-2, 0) + in_x(N-1, 1));
    in_x(N-1, N-1) = 0.5 * (in_x(N-2, N-1) + in_x(N-1, N-2));
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
    int jtmp = 1, itmp = 1;
    int Ntmp = N - 2;
    int dtX = in_dt * Ntmp, dtY = in_dt * Ntmp;

    float tmp1, tmp2, tmp3;
    int i_flr, i_ceil, j_flr, j_ceil;
    float x, y, s0, s1, t0, t1;

    for (int j = 1; j < N-1; j++, jtmp++) {
        for (int i = 1; i < N-1; i++, itmp++) {
            tmp1 = dtX * in_Vx(i, j);
            tmp2 = dtY * in_Vy(i, j);
            x = itmp - tmp1;
            y = jtmp - tmp2;

            if(x < 0.5) x = 0.5;
            if(x > Ntmp + 0.5) x = Ntmp + 0.5;

            i_flr = (int)floor(x);
            i_ceil = i_flr + 1;
            
            if(y < 0.5) y = 0.5;
            if(y > Ntmp + 0.5) y = Ntmp + 0.5;

            j_flr = (int)floor(y);
            j_ceil = j_flr + 1;

            s1 = x - i_flr;
            s0 = 1.0 - s1;
            t1 = y - j_flr;
            t0 = 1.0 - t1;

            d(i, j) = s0 * (t0 * d0(i_flr, j_flr) + t1 * d0(i_flr, j_ceil)) +
                        s1 * (t0 * d0(i_ceil, j_flr) + t1 * d0(i_ceil, j_ceil));
        }
    }

    SetBoundaries(b, d);
}




