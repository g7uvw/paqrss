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
	double *in;
    fftw_complex *out;
    
    static const uint16_t specXSize = 1000;
	static const uint16_t SepcYSize = 512;
	static FIBITMAP *bitmap;
    
    //alloc the FFT workspace
    in = (double*)fftw_malloc(sizeof(double) * BUFSIZE);
    int n_out = ((BUFSIZE/2)+1);
    // complex numbers out
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_out);


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
		
		for (unsigned int a = 0; a < BUFSIZE; a++)
		{
			in[a] = buf[a] * 1.0;
		}
		
		fftw_plan fftplan;
		fftplan = fftw_plan_dft_r2c_1d ( BUFSIZE, in, out, FFTW_ESTIMATE );
        fftw_execute ( fftplan );

		//rewrite output buffer out[i][0] to be ABS^2 of the complex value
		
		for (uint i = 0; i < ((BUFSIZE/2)+1); ++i)
		{
		out[i][0] = out[i][0]*out[i][0] + out[i][1]*out[i][1];
		}


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
	fftw_free ( in );
    fftw_free ( out );
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



