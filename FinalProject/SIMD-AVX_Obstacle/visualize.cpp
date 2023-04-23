/*
	2D Fluid Simulation with Navier-Stokes Equations - 2D Visualization
	using p8g c++ library for 2D rendering. Obstacle put in path of flow
	This implementation uses SIMD with AVX and pthreads are used by the p8g library
	to visualise the 2D simulation with OpenGL.

	Attempt at adding an obstacle into the path of the fluid.
	Likely going to have issues with boundary problems and AVX.

	Only works with 256x256 matrix (because of how box was implemented).

    @author Thomas Petr
    @author Ben Haft
    @date 4/22/2023

	Compilation:
	g++ -std=c++11 visualize.cpp 2d_fluid.cpp -g -mavx2 -L. -lp8g++ -Wl,-rpath=. -o vis2d.o
	
	Running: ./vis2d.o <matrix-size> <target-fps>
	./vis2d.o 256 24

	If having troubles with GLX (OpenGL):
		sudo apt install mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev


	For Windows 10 (pain):
	Use VcXsrv/XLaunch to setup OpenGL + Following Commands (see link for more info)

	export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0
	export LIBGL_ALWAYS_INDIRECT=0

	https://gist.github.com/Mluckydwyer/8df7782b1a6a040e5d01305222149f3c
*/

#include "2d_fluid.h"

using namespace p8g;

Fluid2D* fluid;
float t;
int fps;
extern int N;
extern int PAD_N;
extern int ITR;

double first10fps[10];
double first10itr[10];
int count = 0;

void p8g::draw() {
    colorMode(p8g::HSB);
	stroke(52);
	strokeWeight(2);
	
	// Middle Jet Begin
	int cx = (int)((0.25 * width) / SCALE);
	int cy = (int)((0.5 * height) / SCALE);
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddDensity(cx + i, cy + j, ((rand() / RAND_MAX) * 100.0 + 50));
		}
	}

	float angle = 0;
    float xVec = cos(angle) * 0.2;
    float yVec = sin(angle) * 0.2;
    for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddVelocity(cx + i, cy + j, xVec, yVec);
		}
	}
	// Middle Jet End

	// Bottom Jet Begin
	cx = (int)((0.25 * width) / SCALE);
	cy = (int)((0.35 * height) / SCALE);
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddDensity(cx + i, cy + j, ((rand() / RAND_MAX) * 100.0 + 50));
		}
	}

	angle = M_PI/6.0;
    xVec = cos(angle) * 0.2;
    yVec = sin(angle) * 0.2;
    for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddVelocity(cx + i, cy + j, xVec, yVec);
		}
	}
	// Bottom Jet End

	// Top Jet Begin
	cx = (int)((0.25 * width) / SCALE);
	cy = (int)((0.65 * height) / SCALE);
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddDensity(cx + i, cy + j, ((rand() / RAND_MAX) * 100.0 + 50));
		}
	}

	angle = -M_PI/6.0;
    xVec = cos(angle) * 0.2;
    yVec = sin(angle) * 0.2;
    for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddVelocity(cx + i, cy + j, xVec, yVec);
		}
	}
	// Top Jet End

	clock_t start, end;
    
	// Timer Start
    start = clock();

	fluid->SimStep();
    fluid->RenderDensity();

	// Timer End
    end = clock();

	// Calculating time taken by the program for one frame.
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

	// Convert to Calculations per 1 second (Calculated Frames Per Second)
	double calc_fps = 1.0 / time_taken;

	// Save to Array to Print when Window Closed
	if (!first10fps[9]){
		first10fps[count] = calc_fps;
		first10itr[count] = ITR;
		count++;
	}

	// Compare to Ideal User-Defined FPS, Set ITR (Depth) Accordingly
	if (calc_fps > fps){
		ITR++;
	}
	else if (ITR > 1){
		ITR--;
	}
}

void p8g::keyPressed() {}
void p8g::keyReleased() {}
void p8g::mouseMoved() {}
void p8g::mousePressed() {}
void p8g::mouseReleased() {}
void p8g::mouseWheel(float delta) {}

void printarray(double array[]) {
    int i;
    for(i = 0; i < 10; i++) {
        printf("%.2f\n",array[i]);
    }
}

int main(int argc, char** argv) 
{
	const char* const exeName = argv[0]; // Name of file to execute

    if (argc != 3) { // Need 3 runtime arguments
        printf("Wrong Arguments\n");
        printf("%s SIMULATION_SIZE FRAMES_PER_SECOND\n", exeName);
        return 1;
    }

	int size = atoi(argv[1]);
	N = size;

	int segment_breakup = 256/(sizeof(float)*8);
    PAD_N = ceil((float)N/segment_breakup) * segment_breakup + 8; //adding 8 to PAD for AVX loads/computations

	fps = atoi(argv[2]);

    fluid = new Fluid2D(N, 0.0, 0.0000001, 0.2);
    t = 0;

	run(600, 600, "2D Fluid Simulation");

	printarray(first10fps);
	printarray(first10itr);

	return 0;
}


