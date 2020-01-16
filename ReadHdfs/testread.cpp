/*
 * testread.cpp
 *
 *  Created on: Jan 13, 2020
 *      Author: root
 */

#include "hdfs.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <ctime>
#include <sys/types.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <queue>
#include "mmhdfs.hpp"
using namespace std;
int func()
{
	hdfsFS fs = hdfsConnect("default", 0);
	const char* readPath = "/006zzy/slave06bidMM.txt";
	hdfsFile hf = hdfsOpenFile(fs, readPath, O_RDONLY|O_CREAT, 0, 0, 0);
	if(!hf) {
	     fprintf(stderr, "Failed to open %s for writing!\n", readPath);
	     exit(-1);
	}

	//read file
	clock_t start,end;
	start = clock();
	char buffer[10240];
	int size = 10240;
	while(hdfsRead(fs,hf,(void*)buffer,size) > 0){
		//cout << "read success.."<<endl;
	}
	end = clock();
	std::cout <<"read "<<readPath <<" read a file use time = " << (double)(end-start)/CLOCKS_PER_SEC<<endl;
	hdfsCloseFile(fs, hf);
	start = clock();
	mm();
	end = clock();
	std::cout <<" do a MM use time = " << (double)(end-start)/CLOCKS_PER_SEC<<endl;
	//get hosts
	/*
	char*** hosts = hdfsGetHosts(fs, readPath, 0, 1);
	if(hosts) {
	    fprintf(stderr, "hdfsGetHosts - SUCCESS! ... \n");
	    int i=0;
	    while(hosts[i]) {
	    	int j = 0;
	    	while(hosts[i][j]) {
	    		fprintf(stderr,  "\thosts[%d][%d] - %s\n", i, j, hosts[i][j]);
	    		++j;
	    	}
	    	++i;
	    }
		} else {
			fprintf(stderr, "waah! hdfsGetHosts - FAILED!\n");
	}*/


	//get every block host
	/*
	hdfsFileInfo *fileinfo;
	fileinfo = hdfsGetPathInfo(fs, readPath);
	if(fileinfo == NULL){
		std::cout << "get the file info failed..."<<endl;
		return -1;
	}
	std::cout<<"file name = "<< fileinfo->mName<<endl;
	std::cout<<"file size = "<< fileinfo->mSize<<endl;
	std::cout<<"file block size = "<<fileinfo->mBlockSize<<endl;

	int blocksize = fileinfo->mBlockSize;
	int retain = fileinfo->mSize % fileinfo->mBlockSize;
	int number = fileinfo->mSize / fileinfo->mBlockSize;
	int blocknumber = retain > 0 ? number + 1 : number;

	for(int i = 0;i < blocknumber;i++){
		char*** hosts = hdfsGetHosts(fs, readPath, i*fileinfo->mBlockSize, 1);
		if(hosts) {
			std::cout << "the "<<i<<"(rd) block locate at ";
			int i=0;
			while(hosts[i]) {
			 	int j = 0;
			   	while(hosts[i][j]) {
			 		//fprintf(stderr,  "\thosts[%d][%d] - %s\n", i, j, hosts[i][j]);
			   		std::cout <<hosts[i][j] << " ";
			   		++j;
			   	}
			   	++i;
			}
			std::cout << endl;
		} else {
					fprintf(stderr, "waah! hdfsGetHosts - FAILED!\n");
		}
	}
	*/

	return 0;

}

int main()
{
	clock_t start,end;
	start = clock();
	int times = 1;
	for(int i = 0;i < times;i++){func();}
	end = clock();
	std::cout <<times <<" MMs use time = " << (double)(end-start)/CLOCKS_PER_SEC<<endl;

}






