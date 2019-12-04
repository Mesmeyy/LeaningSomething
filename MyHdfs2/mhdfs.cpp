/*
 * mhdfs.cpp
 *
 *  Created on: Dec 2, 2019
 *      Author: root
 */



/*
 * mhdfs.cpp
 *
 *  Created on: Dec 2, 2019
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
using namespace std;

struct hdfsBuilder *pbld = NULL;
std::queue<struct hdfsStreamBuilder*> queue_hdfsstreambuilder;
hdfsFS my_hdfsfs;
hdfsFile my_hdfsfile;
std::string masterip = "172.19.0.207";
int masterport = 9000;
std::string FilePath = "/data/pictures";
int filenum;

std::vector<string> filepath;
//connect with hdfs cluster
bool ConnectMyHdfs(void);
bool ReadMyHdfs();
bool ConnectMyHdfs(void){
	pbld = hdfsNewBuilder();
	hdfsBuilderSetNameNode(pbld,masterip.c_str());
	hdfsBuilderSetNameNodePort(pbld,masterport);
	hdfsBuilderConfSetStr(pbld,"fs.hdfs.impl","org.apache.hadoop.hdfs.DistributedFileSystem");
	my_hdfsfs = hdfsBuilderConnect(pbld);
    int CheckHDFS = hdfsExists(my_hdfsfs,"/");
    if(!my_hdfsfs || CheckHDFS!= 0){
    	std::cout << "file / not exist..."<<endl;
    	return false;
    }else{
    	return true;
    }
}

bool ReadMyHdfs()
{
	bool isconnect = ConnectMyHdfs();
	if(!isconnect){
		cout<<":Connect to hdfs failed,cur time:"<<time(NULL)<<endl;
		return false;
	}
	cout<<":Connect to hdfs success,cur time:"<<time(NULL)<<endl;
	hdfsFileInfo *fileinfo;
	std::string dicpath = "/data/pictures";
	if (NULL != (fileinfo = hdfsListDirectory(my_hdfsfs, FilePath.c_str(), &filenum))){
		for (int i = 0; i < filenum; i++,fileinfo++){
            std::cout << "*********************************"<<endl;
			string filename = fileinfo->mName;
			cout << "filename = " << filename<<endl;
			int portindex = filename.rfind(":");
			portindex += 5;
			string filefullpath = filename.substr(portindex);
			filepath.push_back(filefullpath);
			std::cout << "file_full_path = " << filefullpath<<endl;

            /*
			struct hdfsStreamBuilder *temp;
            temp = hdfsStreamBuilderAlloc(my_hdfsfs,filefullpath.c_str(),O_RDONLY);
            if(temp != NULL){
            	queue_hdfsstreambuilder.push(temp);
            }

            hdfsFile my_hdfsfile = hdfsStreamBuilderBuild(temp);
            */

            //get default block size
            //tOffset defaultsize = hdfsGetDefaultBlockSizeAtPath(my_hdfsfs,filefullpath.c_str());
           //std::cout << "dedaultsize = " << defaultsize<<endl;

            std::cout <<"file size = "<< fileinfo->mSize<<endl;
            std::cout << "file block size = "<<fileinfo->mBlockSize<<endl;

            char*** filehost = hdfsGetHosts(my_hdfsfs, filefullpath.c_str(),0, fileinfo->mBlockSize);
             if(filehost != 0){
            	 if(filehost[0] != NULL) cout << filehost[0][1]<<endl;
             }
            /*
			hdfsFile hdfstmpfile = hdfsOpenFile(my_hdfsfs, filename.c_str(), O_RDONLY, 0, 2, 0);
		    if(!hdfstmpfile){
		    	cout << "file("<<filename<<") open fail..."<<endl;
		    }*/
		    /*read file in while*/
		    /*char buffer[1024];
		    int length = buffer[1024];
		    tSize result = hdfsRead(my_hdfsfs, my_hdfsfile, (void*)buffer, length);
		    //hdfsseek
		    if(result <-1) cout << "Read fial..."<< endl;
		    cout << "file("<<filename<<") open success..."<<endl;
		    */
            cout << "*************************************"<<endl;
		}
	}
	if ( 0 != hdfsDisconnect(my_hdfsfs) ){
	        cout<<__LINE__<<"hdfsDisconnect failed"<<endl;
	   return false;
	}
    return true;
}
int main()
{
    if(ReadMyHdfs()!= false){
    	cout << "OK use"<<endl;
    	return 0;
    }
    return -1;
}




