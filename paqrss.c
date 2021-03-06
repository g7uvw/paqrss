#include "paqrss.h"


FIBITMAP * CreateBitmap(uint16_t Xsize,uint16_t Ysize,uint8_t BPP)
{
	
	printf("\nInit FreeImage        ");
	FreeImage_Initialise(FALSE);
	FIBITMAP *bitmap = FreeImage_Allocate(Xsize, Ysize, BPP,0,0,0); 
	printf("[OK]");
	return bitmap;
	
}

double * CreateFFTworkspace( unsigned int FFTSize)
{
	//alloc the FFT workspace
    printf("\nAllocate FFT in array        ");
    double * in = fftw_malloc(sizeof(double) * FFTSIZE);
    printf("[OK]");
    return in;
    
}

void PrePadFFT(double* in,int overlap)
{
	printf("\nPad buffer        ");
    //part fill the buffer to start.
	for (int i=0; i<FFTSIZE/overlap; i++)
		in[i] = 0.0;
    printf("[OK]");
	
}

fftw_complex* CreateFFToutbuf(unsigned int size)
{
	// complex numbers out
    printf("\nAllocate FFT out array        ");
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * size);
		printf("[OK]");
	return out;
}

struct pa_simple* OpenPAStream(pa_sample_spec ss)
{
	printf("\nConnect to PulseAudio        ");
	pa_simple *s = NULL;
	int error;
	/* Create the recording stream */
	if (!(s = pa_simple_new(NULL, "PAQRSS", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error)))
	{
		fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		exit(1);
	}
	printf("[OK]");	
	return s;
}

void HanningWindow(int16_t* unwindowed,double* windowed, unsigned int FFTSize)
{
	
	for (unsigned int a = 0; a < FFTSIZE; a++)
		{
			windowed[a] = unwindowed[a] * 0.54 - 0.46 * cosf( (6.283185 * (double) a) / (FFTSize - 1) );
		}
}

//inline void PlotFFTData(FIBITMAP* bitmap, fftw_complex *out, unsigned int n_out, uint16_t specXSize, uint16_t specYSize,uint16_t Image_Col)
//{
			//unsigned int  samplesPerPixel = n_out / specYSize;
			//float val;
			//RGBQUAD pixel;
			
			//for (unsigned int y = 0; y < specYSize; y++)
			//{
				//val = 0.0;
				////do basic samples per pixel averaging
				//for (unsigned int counter = 0; counter < samplesPerPixel; counter++)
				//{
					//val += out[y+counter+500][0];
				//}
				
				//val /= (float) samplesPerPixel;
				////compute LOG10 for scaling.
				//val = log10f(val + 1.0);
				//val *= 0.4f / 3.f; // Manually selected scaling.
				//pixel.rgbBlue = 254;
				//pixel.rgbRed = pixel.rgbGreen = (uint8_t)(val * 255.f + 0.5f);
				//FreeImage_SetPixelColor(bitmap, Image_Col, y, &pixel);
				
			//} 
			
			
			

	
	
//}

inline void PlotFFTData(FIBITMAP* bitmap, fftw_complex *out, unsigned int n_out, uint16_t specXSize, uint16_t specYSize,uint16_t Image_Col)
{
//                      unsigned int samplesPerPixel = n_out / specYSize, yStart;
                        unsigned int PixelsPerSample = 4, yStart;
                        float val;
                        RGBQUAD pixel;
                        
                        for (unsigned int y = 500; y < 1500; y++)
                        {
                                val = out[y][0];
                                val = log10f(val + 1.0);
                                val *= 0.4f / 3.f; // Manually selected scaling.
                                pixel.rgbBlue = 254;
                                pixel.rgbRed = pixel.rgbGreen = (uint8_t)(val * 255.f + 0.5f);
                                //do basic samples per pixel averaging
								yStart = (y-500)*4;
                                for (unsigned int counter = 0; counter < PixelsPerSample; counter++)
                                {
                                	FreeImage_SetPixelColor(bitmap, Image_Col, yStart+counter, &pixel);
                                }
                                
                                //compute LOG10 for scaling.
                                
                        }
                        
                        
                        

        
        
}


int main(int argc, char*argv[])
{
	pa_simple *s = NULL;
	int ret = 1;
	int error;
	int16_t buf[FFTSIZE];
	unsigned int n_out = ((FFTSIZE/2)+1);
	double *in;
    fftw_complex *out;
    
    static const uint16_t specXSize = 1000;
	static const uint16_t specYSize = 2048;
	uint16_t Image_Col = 0;
	
	//connect to pulseadio first - if tthis fails, no point doing anything else
	s = OpenPAStream(ss);
	if (!s)
		{
			printf("\nCan't connect to Pulse Audio");
			exit(1);
		}
	//create a plan
	fftw_plan fftplan;
	
	//somewhere to draw an image
	FIBITMAP *bitmap = CreateBitmap(specXSize,specYSize,24);
    
    //create FFT buffers and pre-pad input for overlap (if used)
    in = CreateFFTworkspace (FFTSIZE);
    PrePadFFT(in,2);    
    out = CreateFFToutbuf(n_out);

	start:
	printf("\nGet Time        ");
	time_t current_time, start_time;
	time(&start_time);
	time(&current_time);
	printf("[OK]");

	time_t next_time = start_time + (grab_length*60);

	
	printf("\nStarting...        ");
	while (current_time < next_time)
	
	{
	
		/* Record some data ... */
		if (pa_simple_read(s, buf, sizeof(buf), &error) < 0)
		{
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			goto finish;
		}
		
		HanningWindow(buf,in, FFTSIZE);

		
		fftplan = fftw_plan_dft_r2c_1d ( FFTSIZE, in, out, FFTW_ESTIMATE );
        fftw_execute ( fftplan );

		//rewrite output buffer out[i][0] to be ABS^2 of the complex value
		
		for (uint i = 0; i < ((FFTSIZE/2)+1); ++i)
		{
		out[i][0] = out[i][0]*out[i][0] + out[i][1]*out[i][1];
		}

		if (Image_Col < specXSize)
		{
			PlotFFTData(bitmap, out, n_out, specXSize, specYSize,Image_Col);
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
	
	if (Image_Col < specXSize)
		goto start;
	else
	
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





