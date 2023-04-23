# Final Project - 2D Fluid Simulation

**Usage:** This program simulates the movement of 2D fluids.

## Compilation, Dependencies, & Runtime Arguments

**Compilation Arguments:** 
- There are many variations of SIMD usage, thus all of the programs have an example compilation at the top.

**Run Command/Arguments:**
- Argument 1: Matrix Size
- Argument 2: Target FPS
- Example: ./vis2d.o 512 10

**Libraries Used:**
- iostream
- cmath
- cstring
- immintrin.h
- omp.h
- fstream
- p8g.hpp (Visualizer)

## Navier-Stokes Fluid Equations & Implementation

The main Navier-Stokes equations are the theoretical basis for all fluid movement. This includes terms for diffusion and convection. When the diffusion term is greater than the convection term, the fluid tends to flow more smoothly, and when the convection term is greater than the diffusion term, the flow becomes erratic and turbulent. Physicists are currently figuring out how to solve the equation, as while it models fluid movements, there is no way to know exactly what a fluid system will look like after a certain amount of time.

Jos Stam divided the Navier-Stokes equations for two-dimensional incompressible fluid simulation. He purposed that one equation models the movement of the velocity field and one the density field. As mass and momentum are both conserved in fluid mechanics. This brings us to our two main equations:

![image](https://user-images.githubusercontent.com/112660711/233766054-5660b673-229f-44c8-9de3-61d6a2df8849.png)

The first equation is: the partial derivative of velocity is equal to the convection, diffusion, and its initial value

The second equation is: the partial derivative of density is equal to the convection, diffusion, and its initial value

Each cell in our simulation is given a velocity and density value of zero (in two discrete arrays) when the simulation begins. Calculations are completed in matrices of all velocity and density values around the current cell. They first satisfy the second term in the equations above (diffusion); this is an iterative process where the fluid gets closer to the real value upon each iteration. Then, the program applies the velocity field to the density field to calculate the movement of the density field. The visualization is the density field. This is a basic definition and can be expanded upon more beginning on page 8 in Michael Ash's thesis paper: [Simulation and Visualization of a 3D Fluid](https://github.com/B3ngu1N/AdvCompSys/blob/main/FinalProject/Final_Project_Papers/thesis-en.pdf)

## Visualization

This 2D fluid simulation is using C++ library p8g to build 2D visualization of fluid (C++ Version of Processing). This library is being used in a very simple way to visualize the 2D density array. Besides installing mesa-utils below, on Windows 11 this program should just compile, run, and open its own window. If using Windows 10, you will have to install a separate program for visualizing (see below). If running the SIMD-AVX_Obstacle version, make note that you have to use a 256x256 matrix for it to work properly because the placement of the box is not relative to window size (it's relative to matrix size). Sadly, we have not had time to update this to make it more streamlined (would've also liked to make other shapes).

If having troubles with GLX (OpenGL):

```shell
sudo apt install mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev
```

**For Windows 10 (pain):**
Use VcXsrv/XLaunch to setup OpenGL + Following Commands (see link for more info).

```shell
export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0
export LIBGL_ALWAYS_INDIRECT=0
```

[Helpful Link for Windows 10](https://gist.github.com/Mluckydwyer/8df7782b1a6a040e5d01305222149f3c)

[p8g GitHub Repository](https://bernhardfritz.github.io/p8g/docs/get-started)

## Improvements

**SIMD:**

SIMD was implemented using Intel's AVX instructions and takes advantage of the way data is stored in our 2D array. In the naive implementation of the Navier-Stokes equations, it can be seen that there are many nested for loops such as the one below:

```c
  float cRecip = 1.0 / c;
  for (int t = 0; t < ITR; t++) {
      for (int j = 1; j < N-1; j++) {
          for (int i = 1; i < N-1; i++) {
              in_x[IX(i, j)] = (in_x0[IX(i, j)] +
                              a * (in_x[IX(i+1, j)] +
                                   in_x[IX(i-1, j)] +
                                   in_x[IX(i, j+1)] +
                                   in_x[IX(i, j-1)])) * 
                              cRecip;
          }
      }
      SetBoundaries(b, in_x);
  }
```

Given that the computation of each cell relies on the surrounding 4 cells (as seen by the image below), this can easily be broken up into rows of 8 cell computations at a time with Intel AVX instructions (given that each cell holds a FP32 value). Since all of the data is in a row-based matrix loading in the data and storing the data is a simple single operation command (load/store 256 bits from a starting address). This means that we can both reduce operations needed to perform a full matrix computation and also minimize the number of cache misses.

![image](https://github.com/B3ngu1N/AdvCompSys/blob/main/FinalProject/Images/2DFluid_ACS.png?raw=true)

If we weren't using the p8g visualizer library (and taking advantage of a GPU to visualize) we could combine these AVX instructions with pthreads or OpenMP to break up these matrix computations even further and gain more performance/fps.

Below is a sample output with an obstacle:

![image](https://github.com/B3ngu1N/AdvCompSys/blob/main/FinalProject/Images/sample_output_obstacle.png?raw=true)

**Continuity:**

To make the program continuous throughout devices a few addons were implemented. The user may choose the matrix size and their desired framerate (frames per second). The matrix size represents one dimension of the velocity/density matrix. With a larger value, there will be significantly more calculations present, which slows down the simulation calculations. In order to combat this, a desired frames per second is allowed. One process that can be limited is the iterations of the diffusion term calculation. Thus, when the program has a target framerate, it adjusts the depth/iterations of the diffusion calculation to get as close to the target frames per second as possible. This allows the program to run at the same speed across devices, no matter the hardware.

**Data Output File:**

Merging the ideas of SIMD and Continuity, we decided to benchmark and compare the difference between naive, openmp, and avx. To do this, the program now outputs a csv file with columns of time taken on that cycle, the calculated FPS of that cycle, and the depth/iterations used on that cycle.

## Experiment

### Notes
**Hardware Environment:**

- WSL Environment
- 6-core Intel i7-9750H CPU @ 2.60 GHz
- 32.0 GB RAM at 2667 MHz
- L1d: 192 kB
- L1i: 192 kB
- L2: 1.5 MB
- L3: 12 MB

**Program Settings:**

- The naive program was created by commenting out the pragma openmp lines
- All programs were run with matrix size 512

### Resulting Graphs
![image](https://user-images.githubusercontent.com/112660711/233849668-e2de3fa6-5af1-4154-901c-f6723c1d66ec.png)

![image](https://user-images.githubusercontent.com/112660711/233849675-897840d8-67e1-4ad7-9977-a89353d49e3f.png)

![image](https://user-images.githubusercontent.com/112660711/233849680-7cb90402-f766-46c4-8b4e-56b83fba67c7.png)

## Process and Conclusions

This course has focused on large data sets and the manipulation of those sets. We have learned how to efficiently manipulate these data sets using multithreading and SIMD. The project was chosen due to interest in fluid simulation and its inately large data sets, which make it a great fit for improving with this course's material. Our process was simplistic as we worked collaboratively to learn about the topic, finding Michael Ash's thesis paper and making it our main source. This gave us a great foundation, as the mathematics behind the fluid simulation was already figured out and written in code. Thus, we had our large data set. Since our final project would end with a presentation, we thought it was important to add a visual element to let us demo our program. The p8g library allowed us to do just that, and automatically incorporated multithreading into our project.

After implementing multithreading and visualization, we wanted to improve the efficiency of the mathematics by applying SIMD. We began by implementing openmp. Openmp applies parallel processing to for loops, thus we believed it would create a more efficient program. At this point we were testing with our visualizations individually with different devices and wanted the results to be somewhat uniform. This led to the idea of coming up with user options for a targetting FPS and matrix size. These were both implemented without much trouble.

We still had a week to improve the program before our project was due, so decided to try using AVX commands to improve performance, which would also give us a great comparison between the two SIMD methods we had used in previous projects. Figuring out new commands with AVX and getting it to work always poses problems, but we were able to complete it in time. To benchmark, we decided to output the numbers we were already calculating for continuity into a csv file and graph it, which is seen in the resulting graphs section above.

At first, we were surprised that the naive version performed significantly better than with openmp. However, we realized openmp tries to multithread the program, but the threads are already being used by the p8g library for visualization. This means the parallelism of openmp was actually slowing down our process. However, our AVX implementation made significant improvements to the original code. Therefore, we achieved the goals set out for our project: visualization, multithreading, and using SIMD. This project allowed for real-world implementation of the skills we have learned in this course.
