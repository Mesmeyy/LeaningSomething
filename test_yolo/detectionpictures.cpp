/*
 * detection.cpp
 *
 *  Created on: Dec 23, 2019
 *      Author: root
 */
#include "detection.h"
#include <cstring>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std;


int main()
{
	std::string infile = "/data/006zzy/files/project5-master/1018CPUGPUBSV6/detection-master-v6/dog.jpg";
    char inputfile[256];
    memcpy(inputfile,infile.c_str(),256);

    std::string outfile = "/data/006zzy/files/project5-master/1018CPUGPUBSV6/detection-master-v6/pre";
    char outputfile[256];
    memcpy(outputfile,outfile.c_str(),256);

    float thresh = 0.5;
    int fullscreen = 1;
    std::string cocofile = "/data/006zzy/files/project5-master/Project5v8/src/cfg/coco.data";
    char cocodata[256];
    memcpy(cocodata,cocofile.c_str(),256);

    char** names = load_names(cocodata);
    if(names == NULL) {
        printf("load names fail...\n");
    }else{
        printf("load names success...\n");
    }

    std::string cfgf = "/data/006zzy/files/project5-master/Project5v8/src/cfg/yolov3.cfg";
    char cfgfile[256];
    memcpy(cfgfile,cfgf.c_str(),256);

    std::string yolof = "/data/006zzy/files/project5-master/Project5v8/src/yolov3.weights";
    char yoloweightfile[256];
    memcpy(yoloweightfile,yolof.c_str(),256);

    network *net = load_network_test(cfgfile,yoloweightfile,0,0,0);
    float hier_thresh =0.5;
    int w=768;
    int h=576;
    int c=3;
    std::string labelp = "/data/006zzy/files/project5-master/Project5v8/src/data/labels";
    char labelpath[256];
    memcpy(labelpath,labelp.c_str(),256);


    unsigned char* jpgbytes = jpg2BytesInC(inputfile,3);
    boxesAndAcc* bTest=detectByInputBytes(jpgbytes,thresh, hier_thresh, outputfile,fullscreen, names,net, w, h, c, labelpath);
    if(bTest!=NULL){
       printf("detection success...\n");
       free(bTest);
    }else{
        printf("detection fail...\n");
    }
    return 0;

}



