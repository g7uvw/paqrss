//paqrss.h
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <fftw3.h>
#include <complex.h>
#include <err.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <FreeImage.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

// GS HACKED from 4096 to 
int FFTSIZE = 65536;

#define grab_length 1

/* The Pulse Audio sample type to use */
	static const pa_sample_spec ss =
	{
		.format = PA_SAMPLE_S16LE,
		.rate = 48000,
		.channels = 1
	};

FIBITMAP* CreateBitmap(uint16_t Xsize,uint16_t Ysize,uint8_t BPP);
double* CreateFFTworkspace(unsigned int FFTSize);
fftw_complex* CreateFFToutbuf(unsigned int size);
void PrePadFFT(double* in,int overlap);
struct pa_simple* OpenPAStream(pa_sample_spec ss);
void HanningWindow(int16_t* unwindowed,double* windowed, unsigned int FFTSize);
void PlotFFTData(FIBITMAP* bitmap, fftw_complex *out, unsigned int n_out, uint16_t specXSize, uint16_t specYSize,uint16_t Image_Col);


