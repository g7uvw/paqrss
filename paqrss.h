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

#define FFTSIZE 4096
#define grab_length 1


/* The Pulse Audio sample type to use */
	static const pa_sample_spec ss =
	{
		.format = PA_SAMPLE_S16LE,
		.rate = 4096,
		.channels = 1
	};
