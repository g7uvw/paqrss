#include "paqrss.h"

FIBITMAP * CreateBitmap(uint16_t Xsize,uint16_t Ysize,uint8_t BPP);

FIBITMAP * CreateBitmap(uint16_t Xsize,uint16_t Ysize,uint8_t BPP)
{
	
	printf("\nInit FreeImage        ");
	FreeImage_Initialise(FALSE);
	FIBITMAP *bitmap = FreeImage_Allocate(Xsize, Ysize, BPP,0,0,0); 
	printf("[OK]");
	return bitmap;
	
}


int main(int argc, char*argv[])
{
	pa_simple *s = NULL;
	int ret = 1;
	int error;
	int16_t buf[FFTSIZE];
	int n_out = ((FFTSIZE/2)+1);
	double *in;
    fftw_complex *out;
    
    static const uint16_t specXSize = 1000;
	static const uint16_t specYSize = 2048;
	
	FIBITMAP *bitmap = CreateBitmap(specXSize,specYSize,24);
	
	//printf("\nInit FreeImage        ");
	//FreeImage_Initialise(FALSE);
	//FIBITMAP *bitmap = FreeImage_Allocate(specXSize, specYSize, 24,0,0,0); 
	//printf("[OK]");
	
	uint16_t Image_Col = 0;
    
    //alloc the FFT workspace
    printf("\nAllocate FFT in array        ");
    in = (double*)fftw_malloc(sizeof(double) * FFTSIZE);
    printf("[OK]");
    
    printf("\nPad buffer        ");
    //part fill the buffer to start.
	for (int i=0; i<FFTSIZE/2; i++)
		in[i] = 0.0;
    printf("[OK]");
    
    // complex numbers out
     printf("\nAllocate FFT out array        ");
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_out);
     printf("[OK]");
     

	printf("\nConnect to PulseAudio        ");
	/* Create the recording stream */
	if (!(s = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error)))
	{
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		goto finish;
	}
	printf("[OK]");
	
	start:
	printf("\nGet Time        ");
	time_t current_time, start_time;
	time(&start_time);
	time(&current_time);
	printf("[OK]");

	time_t next_time = start_time + (grab_length*60);

	
	printf("\nStarting...        ");
	while (current_time < next_time)
	
	//for (;;)
	{
	
		/* Record some data ... */
		if (pa_simple_read(s, buf, sizeof(buf), &error) < 0)
		{
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			goto finish;
		}
		
		for (unsigned int a = 0; a < FFTSIZE; a++)
		{
			in[a] = buf[a] * 0.54 - 0.46 * cosf( (6.283185 * a) / (FFTSIZE - 1) );
		}
		
		
		fftw_plan fftplan;
		fftplan = fftw_plan_dft_r2c_1d ( FFTSIZE, in, out, FFTW_ESTIMATE );
        fftw_execute ( fftplan );

		//rewrite output buffer out[i][0] to be ABS^2 of the complex value
		
		for (uint i = 0; i < ((FFTSIZE/2)+1); ++i)
		{
		out[i][0] = out[i][0]*out[i][0] + out[i][1]*out[i][1];
		}

		//do the image stuff here.
		if (Image_Col < specXSize)
		{
			uint samplesPerPixel = n_out / specYSize;
			float val;
			RGBQUAD pixel;
			for (uint y = 0, sample = 0; y < specYSize; ++y)
				{
				// Average values over samplesPerPixel numbers.
				val = 0.f;
				for (uint localSample = 0; localSample < samplesPerPixel; ++localSample, ++sample)
				val += out[sample][0];
				val /= (float)samplesPerPixel;
      
				val = log10f(val + 1.f); // Logarithm.
				
				val *= 0.4f / 3.f; // Manually selected scaling.
				if (val < 0.0) printf("\n negaative val");
				//if (val > 1.0) val = 1.0;
				
				//val = common::minmax(0.f, val, 1.f); // Clamp.
				//val = powf(val, 1.f/2.2f); // Gamma (a computer graphics thing :)
				
				pixel.rgbBlue = 254;
				pixel.rgbRed = pixel.rgbGreen = (uint8_t)(val * 255.f + 0.5f);
				FreeImage_SetPixelColor(bitmap, Image_Col, y, &pixel);
				}

		++Image_Col;
		printf("\n %ld Seconds left in this spectrogram",next_time-current_time);
		}

		
		time(&current_time);
	}
	char filename[30];
    sprintf( filename, "%ld.png", current_time );
    printf("\nSaving spectrogram %s\n",filename);
	//FreeImage_Save(FIF_PNG, bitmap, "spectrogram2.png",PNG_DEFAULT);
	FreeImage_Save(FIF_PNG, bitmap, filename,PNG_DEFAULT);
	goto start;
	
	FreeImage_Unload(bitmap);
	FreeImage_DeInitialise();


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



