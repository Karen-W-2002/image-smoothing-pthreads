# Parallel Image Smoothing using POSIX Threads
Program written in C++ with the use of pthreads.h

### Description 
Image smoothing of a bmp file using thread parallelisation. This image is smoothed over 1000 times and parallelisation will allow this image to be processed faster. The image smoothing is done by averaging the pixels around the current pixel, and parallelisation is done mostly through pthreads.

### Process of Creating my Method of Thread Parallelisation


### Program Functions
- **main(int argc, char \*argv[])**
  - Main function of the program

- **int readBMP(char \*fileName)**
  - Reads the BMP file (input.bmp) and saves into \*\*BMPReadData

- **int saveBMP(char \*fileName)**
  - Saves the BMP file from \*\* BMPSaveData and stores into BMP file (output.bmp)

- **void swap(RGBTRIPLE \*a, RGBTRIPLE \*b)**
  - Exchange pixel data with temp storge indicators

- **RGBTRIPLE \*\*alloc_memory(int Y, int X)**
  - Dynmically allocates memory

### Sections of The Program


### Compilation
``
### Execution
`./Smooth`
## Results
![Image of a graph]()

### Analysis on the results
The change is not too significant compared to the other parallel programs, however it is very significant for the first processor to the 4th, in itself. This might be because of how many times each processor needs to send and recieve from eachother to update their data

But the negative slope does mean that the time has definitely increased because of parallelisation

### My thoughts
