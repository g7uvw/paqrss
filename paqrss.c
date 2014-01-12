#include "paqrss.h"

#define BUFSIZE 1024




int main(int argc, char*argv[])
{
	/* The sample type to use */
	static const pa_sample_spec ss =
	{
		.format = PA_SAMPLE_S16LE,
		.rate = 4096,
		.channels = 1
	};

	struct holder holder = {};
	holder.channels = ss.channels;
	holder.samples_count = holder.samplerate / 100;
	
	pa_simple *s = NULL;
	int ret = 1;
	int error;

	int16_t buf[BUFSIZE];


	/* Create the recording stream */
	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error)))
	{
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		goto finish;
	}
	for (;;)
	{
	
		/* Record some data ... */
		if (pa_simple_read(s, buf, sizeof(buf), &error) < 0)
		{
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			goto finish;
		}
		
		//compute_fftw(&stuff);

		/* And write it to STDOUT */
		if (loop_write(STDOUT_FILENO, buf, sizeof(buf)) != sizeof(buf))
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

/* A simple routine calling UNIX write() in a loop */
static ssize_t loop_write(int fd, const void*data, size_t size) {
ssize_t ret = 0;
while (size > 0) {
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

static void prepare_fftw(struct holder *holder)
{
	unsigned int a;

	holder->samples = fftw_alloc_real(holder->samples_count);
	if (!holder->samples)
		errx(3, "cannot allocate input");
	holder->output = fftw_alloc_complex(holder->samples_count);
	if (!holder->output)
		errx(3, "cannot allocate output");

	for (a = 0; a < holder->samples_count; a++) {
		holder->samples[a] = 0;
		holder->output[a] = 0;
	}

	holder->plan = fftw_plan_dft_r2c_1d(holder->samples_count,
			holder->samples, holder->output, 0);
	if (!holder->plan)
		errx(3, "plan not created");
}


static void destroy_fftw(struct holder *holder)
{
	fftw_destroy_plan(holder->plan);
	fftw_free(holder->output);
	fftw_free(holder->samples);
}

/* compute avg of all channels */
static void compute_avg(struct holder *holder, float *buf, unsigned int count)
{
	unsigned int channels = holder->channels;
	unsigned int a, ch;

	for (a = 0; a < count; a++) {
		holder->samples[a] = 0;
		for (ch = 0; ch < channels; ch++)
			holder->samples[a] += buf[a * channels + ch];
		holder->samples[a] /= channels;
	}
}

static void compute_fftw(struct holder *holder)
{
	unsigned int a;

	fftw_execute(holder->plan);

	for (a = 0; a < holder->samples_count / 2; a++) {
		holder->samples[a] = cabs(holder->output[a]);
		if (holder->samples[a] > holder->max)
			holder->max = holder->samples[a];
	}
}

static void decode(struct holder *holder)
{
	unsigned int channels = holder->channels;
	float buf[channels * holder->samples_count];
	int count, short_read;

	do {
		count = sf_readf_float(holder->infile, buf,
				holder->samples_count);
		if (count <= 0)
			break;

		/* the last chunk? */
		short_read = count != holder->samples_count;
		if (!short_read) {
			compute_avg(holder, buf, count);
			compute_fftw(holder);
			show_graph(holder);
		}

		write_snd(holder, buf, count);
	} while (!short_read);
}


