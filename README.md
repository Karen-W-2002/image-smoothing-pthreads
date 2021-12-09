# Parallel Image Smoothing using POSIX Threads
Program written in C++ with the use of pthread.h

### Description 
Image smoothing of a bmp file using thread parallelisation. This image is smoothed 1000 times and parallelisation will allow this image to be processed faster. The image smoothing is done by averaging the pixels around the current pixel, and parallelisation is done through pthreads.
#### Before
![Image of an unsmoothed BMP file](https://github.com/Karen-W-2002/image-smoothing-pthreads/blob/main/input.bmp)
#### After
![Image of a smoothed BMP file](https://github.com/Karen-W-2002/image-smoothing-pthreads/blob/main/output.bmp)
### How Does the Program Work?
The program uses **concurrent** multithreading making the program run faster
Number of threads: your choice, edit #define ThreadNUM

The threads splits the image equally and each process this image concurrently, achieving a faster run time

First, the main program creates n threads, and then, in order to achieve concurrency and thread safety during image processing, each thread first copies the global variable into their own local variable with the use of **mutexes** and **std::fill**

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

- **smooth(void \*arg)**
  - Smoothing operation, called by pthread_create

- **process_column(RGBTRIPLE \*\*BMPData, BMPInfo bmpInfo, RGBTRIPLE \*\*BMPTemp, int id)**
  - For loop that calls process_row, runs bmpInfo.biHeight/ThreadNUM times, each thread has a different range that is calculated inside this function

- **process_row(int i, RGBTRIPLE \*\*BMPData, BMPInfo bmpInfo, RGBTRIPLE \*\*BMPTemp)**
  - For loop that processes/edits the image (which is BMPTemp, BMPTemp is later swapped to complete the image), the range is the whole row (bmpInfo.biWidth)
  - 
### Compilation
`g++ -g -Wall -o Smooth Smooth.cpp -lpthread`
### Execution
`./Smooth`
## Results
![Image of a graph](https://github.com/Karen-W-2002/image-smoothing-pthreads/blob/main/Graph.png)

### Analysis on the results
Concurrent multithreading makes the result faster, however because of overhead, the cost of resources when too many threads are created, that can cause the program to run slower and slower, which explains the graph of the negative slope then positive slope.

### My thoughts
