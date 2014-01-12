/***
This file is part of PulseAudio.
PulseAudio is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation; either version 2.1 of the License,
or (at your option) any later version.
PulseAudio is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.
You should have received a copy of the GNU Lesser General Public License
along with PulseAudio; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA.
***/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <fftw3.h>
#include <complex.h>

#define BUFSIZE 1024

struct stuff
	{
	fftw_complex *output;
	fftw_plan plan;
	int height, width;
	unsigned int samples_count;
	double *samples;
	double max;
	int16_t buf[BUFSIZE];
	};


/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void*data, size_t size)
{
	ssize_t ret = 0;
	while (size > 0)
	{
		ssize_t r;
		if ((r = write(fd, data, size)) < 0)
			return r;
		if (r == 0)
			break;
		ret += r;
		data = (const uint8_t*) data + r;
		size -= (size_t) r;
	}
	return ret;
}

static void prepare_fftw(struct stuff *stuff)
{
	unsigned int a;

	stuff->samples = fftw_alloc_real(stuff->samples_count);
	if (!stuff->samples)
		errx(3, "cannot allocate input");
	stuff->output = fftw_alloc_complex(stuff->samples_count);
	if (!stuff->output)
		errx(3, "cannot allocate output");

	for (a = 0; a < stuff->samples_count; a++) {
		stuff->samples[a] = 0;
		stuff->output[a][a] = 0;
	}

	stuff->plan = fftw_plan_dft_r2c_1d(stuff->samples_count,
			stuff->samples, stuff->output, 0);
	if (!stuff->plan)
		errx(3, "plan not created");
}

static void destroy_fftw(struct stuff *stuff)
{
	fftw_destroy_plan(stuff->plan);
	fftw_free(stuff->output);
	fftw_free(stuff->samples);
}

static void compute_fftw(struct stuff *stuff)
{
	unsigned int a;

	fftw_execute(stuff->plan);

	for (a = 0; a < stuff->samples_count / 2; a++) {
		stuff->samples[a] = cabs(stuff->output[a][a]);
		if (stuff->samples[a] > stuff->max)
			stuff->max = stuff->samples[a];
	}
}

/* compute avg of all channels */
//static void compute_avg(struct stuff *stuff, float *buf, unsigned int count)
//{
//	unsigned int channels = 1; //stuff->ininfo.channels;
//	unsigned int a, ch;
//
//	for (a = 0; a < count; a++) {
//		stuff->samples[a] = 0;
//		for (ch = 0; ch < channels; ch++)
//			stuff->samples[a] += buf[a * channels + ch];
//		stuff->samples[a] /= channels;
//	}
//}





int main(int argc, char*argv[])
{
	/* The sample type to use */
	static const pa_sample_spec ss =
	{
		.format = PA_SAMPLE_S16LE,
		.rate = 4096,
		.channels = 1
	};

	struct stuff stuff = {};
	stuff.samples_count = ss.rate / 100;
	pa_simple *s = NULL;
	int ret = 1;
	int error;

	prepare_fftw(&stuff);

	/* Create the recording stream */
	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error)))
	{
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		goto finish;
	}
	for (;;)
	{
	
		/* Record some data ... */
		if (pa_simple_read(s, stuff.buf, sizeof(stuff.buf), &error) < 0)
		{
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			goto finish;
		}
		
		for (int dd=0; dd<sizeof(stuff.buf); dd++)
		{
		stuff.samples[dd]=stuff.buf[dd];
		}
		//compute_fftw(&stuff);

		/* And write it to STDOUT */
		if (loop_write(STDOUT_FILENO, stuff.buf, sizeof(stuff.buf)) != sizeof(stuff.buf))
		{
			fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
			goto finish;
		}
	}
	ret = 0;
	finish:
	if (s)
		pa_simple_free(s);
	return ret;
}
