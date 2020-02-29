/*************************************************************************
	> File Name: slave.cpp
	> Author: 
	> Mail: 
	> Created Time: Fri 28 Feb 2020 07:45:11 PM CST
 ************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <fstream>
#include <unistd.h>

using namespace std;

const double INF= 1e20;
const int MAXD = 1000;
const int MAXN = 1000;
const int MAXC = 50;

struct aCluster
{
    double Center[MAXD];//类的中心
    int Number = 0;//类中包含的样本point数目
    int Member[MAXN];//类中包含的样本point的index
    //double Err;//我想同意使用一个err
};
class S_Kmeans{
    private:
    int Cluster_Index = -1;//该类的编号
    double Point[MAXN][MAXD];//slave必须要保存所有的数值
    struct aCluster Slave_Acluster;//该slave独有的一个cluster
    int Point_Num = 0;//样本数
    int Point_Dimension = 0;//样本属性维度
    int Cluster_Num = 0;
    double Tempcenter[MAXD];


    public:
    bool ReadData();//读取全部的样本点
    int Mapper();//进行分片计算
    double Distance(int index);//根据样本下表计算该样本距离本类中心的距离
    int Combiner();
};
bool S_Kmeans::ReadData()
{
    ifstream infile;
    infile.open("data.txt");
    infile >> Point_Num;
    infile >> Point_Dimension;
    infile >> Cluster_Num;
    for(int i = 0;i < Point_Num;i++){
        for(int j = 0;j < Point_Dimension;j++){
            infile >> Point[i][j];
        }
    }
    infile.close();
    std::cout << "read tempdata for this slave is ok..."<<std::endl;
    //slave不需要把数据写入其中，但是他需要从属于它的tempdata里面读取center
}

int S_Kmeans::Mapper()
{
    std::string filename = "tempdata_";
    std::string number = std::to_string(Cluster_Index);
    filename += number;
    filename += ".txt";
    ifstream infile;
    infile.open(filename);
    double temp;
    Slave_Acluster.Number = 0;
    for(int j = 0;j < Point_Dimension;j++){//读取中心
        infile >> temp;
        Slave_Acluster.Center[j] = temp;
    }
    for(int j = 0;j < Point_Num;j++){//把符合条件的样本点加入该类
        int index = 0;
        double dis = INF;
        if(Distance(j) < dis){
            dis = Distance(j);
            index = Cluster_Index;
        }
        Slave_Acluster.Member[Slave_Acluster.Number++] = j;
    }

}

double S_Kmeans::Distance(int i)
{
    double dis = 0;
    for(int j = 0;j < Point_Dimension;j++){
        dis += (Point[i][j] - Slave_Acluster.Center[j])*(Point[i][j] - Slave_Acluster.Center[j]); 
    }
    return sqrt(dis);
}

int S_Kmeans::Combiner()
{
    int id;
    memset(Tempcenter,0,sizeof(Tempcenter));
    for(int k = 0;k < Point_Dimension;k++){
        for(int j = 0 ;j < Slave_Acluster.Number;j++){
            Tempcenter[k] += Point[j][k];
        }
    }
}

