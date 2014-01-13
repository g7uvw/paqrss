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
#include <freeimage.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
struct holder {
	fftw_complex *output;
	fftw_plan plan;
	int height, width;
	unsigned int samples_count;
	double *samples;
	double max;
	int channels;
	int samplerate;
};
static ssize_t loop_write(int fd, const void*data, size_t size);
static void prepare_fftw(struct holder *holder);
static void destroy_fftw(struct holder *holder);
static void compute_avg(struct holder *holder, float *buf, unsigned int count);
static void compute_fftw(struct holder *holder);
static void decode(struct holder *holder);


