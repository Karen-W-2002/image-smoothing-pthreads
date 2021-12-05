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

pthread_mutex_t lock;


BMPHEADER bmpHeader;            
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                              
RGBTRIPLE **BMPData = NULL;                                                   

int readBMP(char *fileName); //read file
int saveBMP(char *fileName); //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X); //allocate memory
//void smooth(RGBTRIPLE **BMPSaveData, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp);
void *smooth(void *args);
void process_data0(int height, RGBTRIPLE **BMPSaveData, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp);
void process_data(int i, RGBTRIPLE **BMPSaveData, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp);
//void store_data(int i, RGBTRIPLE ***BMPSaveData, RGBTRIPLE ***BMPTemp);

struct Arguments {
	RGBTRIPLE **BMPSaveData;
	RGBTRIPLE **BMPData;
	BMPINFO bmpInfo;
	RGBTRIPLE **BMPTemp;
} args;

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
	RGBTRIPLE **BMPTemp = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
	// threads
	pthread_mutex_init(&lock, NULL);
	pthread_t tid[ThreadNUM];
		
	swap(BMPSaveData, BMPTemp);
	// smooth 1000 times
	//struct Arguments args;
	//for(int i = 0; i < ThreadNUM; i++) {
		args.BMPSaveData = BMPSaveData;
		args.BMPData = BMPData;
		args.bmpInfo = bmpInfo;
		args.BMPTemp = BMPTemp;	
		pthread_create(&tid[0], NULL, smooth, (void*)args);
	//}

	/*for(int i = 0; i < ThreadNUM; i++) {
		pthread_join(tid[i], NULL);
	 }*/
	//smooth(BMPSaveData, BMPData, bmpInfo, BMPTemp);
	//smooth(args);

	printf("Time taken %.2f seconds\n", (double)(clock()-time_start)/CLOCKS_PER_SEC);

    if ( saveBMP( outfileName ) )
    	cout << "Save file successfully!!" << endl;
    else
        cout << "Save file fails!!" << endl;

	pthread_mutex_destroy(&lock);
	free(BMPData);
	free(BMPSaveData);
	free(BMPTemp);
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

//void smooth(RGBTRIPLE **BMPSaveData, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp)
void *smooth(void* args)
{
	Arguments *local_args = (Arguments*)args;

	for(int count = 0; count < NSmooth; count ++) {
		// Barrier
		printf("count: %d\n", count);
		swap(local_args->BMPTemp, local_args->BMPData);
		// Barrier
		process_data0(local_args->bmpInfo.biHeight, local_args->BMPSaveData, local_args->BMPData, local_args->bmpInfo, local_args->BMPTemp);
	}
	pthread_mutex_lock(&lock);
	std::copy(&local_args->BMPTemp[0][0], &local_args->BMPTemp[0][0]+(local_args->bmpInfo.biWidth*local_args->bmpInfo.biHeight)/2, &local_args->BMPSaveData[0][0]);
	pthread_mutex_unlock(&lock);
}

void process_data0(int height, RGBTRIPLE **BMPSaveData, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp)
{
	for(int i = 0; i < height; i++) {
		process_data(i, BMPSaveData, BMPData, bmpInfo, BMPTemp);
	}
}	

void process_data(int i, RGBTRIPLE **BMPSaveData, RGBTRIPLE **BMPData, BMPINFO bmpInfo, RGBTRIPLE **BMPTemp)
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
