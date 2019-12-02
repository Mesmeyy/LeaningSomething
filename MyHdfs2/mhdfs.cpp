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
using namespace std;

struct hdfsBuilder *pbld = NULL;
hdfsFS my_hdfsfs;
std::string masterip = "172.19.0.207";
int masterport = 9000;

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
	if (NULL != (fileinfo = hdfsListDirectory(my_hdfsfs, "/data/pictures", &filenum))){
		for (int i = 0; i < filenum; i++,fileinfo++){
			string filename = fileinfo->mName;
			cout << "filename = " << filename<<endl;
			hdfsFile hdfstmpfile = hdfsOpenFile(my_hdfsfs, filename.c_str(), O_RDONLY, 0, 2, 0);
		    if(!hdfstmpfile){
		    	cout << "file("<<filename<<") open fail..."<<endl;
		    }
		    //Size result = hdfsRead(my_, readFile, (void*)buffer, bufferSize);
		    cout << "file("<<filename<<") open success..."<<endl;

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




