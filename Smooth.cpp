#include <iostream>
#include <time.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include "bmp.h"

using namespace std;

#define NSmooth 10
#define ThreadNUM 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int id_signal[ThreadNUM] = {0};

BMPHEADER bmpHeader;            
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                              
RGBTRIPLE **BMPData = NULL;                                                   

int readBMP(char *fileName); //read file
int saveBMP(char *fileName); //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X); //allocate memory
void *smooth(void *arg);
void process_data0(RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp, int id);
void process_data(int i, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp);

int main(int argc,char *argv[])
{
	clock_t time_start = clock();

    char *infileName = (char*)"input.bmp";
    char *outfileName = (char*)"output.bmp";
	
    if ( readBMP( infileName) )
    	cout << "Read file successfully!!" << endl;
    else 
        cout << "Read file fails!!" << endl;

    BMPData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

	// threads
	pthread_t tid[ThreadNUM];
	unsigned int ints[ThreadNUM];

	// smoothing operation using threads
	for(int i = 0; i < ThreadNUM; i++) {
		ints[i] = i;
		pthread_create(&tid[i], NULL, smooth, &ints[i]);
	}
	for(int i = 0; i < ThreadNUM; i++) {
		pthread_join(tid[i], NULL);
	 }

	printf("Time taken %.2f seconds\n", (double)(clock()-time_start)/CLOCKS_PER_SEC);

    if ( saveBMP( outfileName ) )
    	cout << "Save file successfully!!" << endl;
    else
        cout << "Save file fails!!" << endl;

	pthread_mutex_destroy(&mutex);
	free(BMPData);
	free(BMPSaveData);
    return 0;
}

int readBMP(char *fileName)
{
    ifstream bmpFile( fileName, ios::in | ios::binary );

    if (!bmpFile ){
    	cout << "It can't open file!!" << endl;
       	return 0;
   	}
 
	bmpFile.read((char*)&bmpHeader, sizeof(BMPHEADER));
 
    if(bmpHeader.bfType != 0x4d42){
    	cout << "This file is not .BMP!!" << endl ;
        return 0;
    }
 
    bmpFile.read((char*)&bmpInfo, sizeof(BMPINFO));
        
    if (bmpInfo.biBitCount != 24){
   		cout << "The file is not 24 bits!!" << endl;
        return 0;
   	}

    while(bmpInfo.biWidth % 4 != 0)
    	bmpInfo.biWidth++;

    BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
	bmpFile.read((char*)BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);
	
    bmpFile.close();
 
   	return 1;
}

int saveBMP( char *fileName)
{
	if(bmpHeader.bfType != 0x4d42) {
    	cout << "This file is not .BMP!!" << endl ;
        return 0;
    }
        
    ofstream newFile( fileName,  ios:: out | ios::binary );
 
    if (!newFile){
   		cout << "The File can't create!!" << endl;
        return 0;  
	}
 	
    newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

    newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

    newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

    newFile.close();
 
    return 1;
}

RGBTRIPLE **alloc_memory(int Y, int X )
{        
    RGBTRIPLE **temp = new RGBTRIPLE *[Y];
	RGBTRIPLE *temp2 = new RGBTRIPLE [Y * X];
   	memset( temp, 0, sizeof( RGBTRIPLE) * Y);
   	memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

	for( int i = 0; i < Y; i++){
        temp[ i ] = &temp2[i*X];
    }
 
    return temp; 
}

void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}

void *smooth(void *arg)
{
	int id = *((int*)arg);
	int total_area = bmpInfo.biHeight * bmpInfo.biWidth;
	RGBTRIPLE **BMPTemp = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
	RGBTRIPLE **BMPSaveData_local = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
	RGBTRIPLE **BMPData_local = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
	std::copy(&BMPSaveData[0][0], &BMPSaveData[0][0] + total_area, &BMPSaveData_local[0][0]);
	std::copy(&BMPData[0][0], &BMPData[0][0] + total_area, &BMPData_local[0][0]);
	swap(BMPSaveData_local, BMPTemp);

	for(int count = 0; count < NSmooth; count ++) {
		printf("count: %d\n", count);
		//update Data!!!! then copy data to data local.
		

		swap(BMPTemp, BMPData_local);
		process_data0(BMPData_local, bmpInfo, BMPTemp, id);
	}

	// resets signal for the next section of code
	if(id == 0) {
		for(int i = 0; i < ThreadNUM; i++) {
			id_signal[i] = 0;
		}
	}

	if(id != (ThreadNUM-1)) {
		while(1) {
			if(id_signal[id] == 1) {
				break;
			}		
		}
	}

	// race condition	
	pthread_mutex_lock(&mutex);
	std::copy(&BMPTemp[0][0], &BMPTemp[0][0]+total_area/(ThreadNUM-id), &BMPSaveData[0][0]);
	if(id!=0) id_signal[id-1] = 1;
	pthread_mutex_unlock(&mutex);

	free(BMPTemp);
	free(BMPSaveData_local);
	free(BMPData_local);
	return 0;
}

void process_data0(RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp, int id)
{
	int start_i = ((bmpInfo.biHeight/ThreadNUM)*id);
	int end_i = ((bmpInfo.biHeight/ThreadNUM)*(id+1));
	if(end_i != bmpInfo.biHeight) end_i+=2;
	if(start_i != 0) start_i-=2;
	for(int i = start_i; i < end_i; i++) {
		process_data(i, BMPData, bmpInfo, BMPTemp);
	}
}	

void process_data(int i, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp)
{
	for(int j = 0; j < bmpInfo.biWidth; j++) {
		int Top = i > 0 ? i-1 : bmpInfo.biHeight-1;
		int Down = i<bmpInfo.biHeight-1 ? i+1 : 0;
		int Left = j>0 ? j-1 : bmpInfo.biWidth-1;
		int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
		BMPTemp[i][j].rgbBlue = (double)(BMPData[i][j].rgbBlue+BMPData[Top][j].rgbBlue+BMPData[Top][Left].rgbBlue+BMPData[Top][Right].rgbBlue+BMPData[Down][j].rgbBlue+BMPData[Down][Left].rgbBlue+BMPData[Down][Right].rgbBlue+BMPData[i][Left].rgbBlue+BMPData[i][Right].rgbBlue)/9+0.5;
		BMPTemp[i][j].rgbGreen = (double)(BMPData[i][j].rgbGreen+BMPData[Top][j].rgbGreen+BMPData[Top][Left].rgbGreen+BMPData[Top][Right].rgbGreen+BMPData[Down][j].rgbGreen+BMPData[Down][Left].rgbGreen+BMPData[Down][Right].rgbGreen+BMPData[i][Left].rgbGreen+BMPData[i][Right].rgbGreen)/9+0.5;
		BMPTemp[i][j].rgbRed = (double)(BMPData[i][j].rgbRed+BMPData[Top][j].rgbRed+BMPData[Top][Left].rgbRed+BMPData[Top][Right].rgbRed+BMPData[Down][j].rgbRed+BMPData[Down][Left].rgbRed+BMPData[Down][Right].rgbRed+BMPData[i][Left].rgbRed+BMPData[i][Right].rgbRed)/9+0.5;
	}
}
