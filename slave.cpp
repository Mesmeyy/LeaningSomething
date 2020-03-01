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

const double INF= 1e20;//精细度
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
    struct aCluster Slave_Acluster;//该slave独有的一个Cluster_Num
    struct aCluster All_Cluster[MAXN];
    int Point_Num = 0;//样本数
    int Point_Dimension = 0;//样本属性维度
    int Cluster_Num = 0;
    double Tempcenter[MAXD];


    public:
    int Set_Cluster_Index(int a);//设置该类的下标
    int Get_Cluster_Index();//获取该类的下标
    bool ReadData();//读取全部的样本点
    int Mapper();//进行分片计算
    double Distance(int index,int j);//根据样本下表计算该样本距离j类中心的距离
    int Combiner();//汇总该中心的各个维度总距离
    int Reducer();//求该类各个维度的中心值
};
int S_Kmeans::Set_Cluster_Index(int a)
{
    Cluster_Index = a;
    return 0;
}
int S_Kmeans::Get_Cluster_Index()
{
    return Cluster_Index;
}
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
    std::cout << "read data.txt form this slave is ok..."<<std::endl;
    //每一个slave都需要知道其他slave的中心值才能得知它到样本点是否最小
    for(int i = 0;i < Cluster_Num;i++){
        std::string number = std::to_string(i);
        std::string filename = "tempdata_";
        filename += number;
        filename += ".txt";
        ifstream infile;
        infile.open(filename);
        for(int j = 0;j < Point_Dimension;j++){
            infile >> All_Cluster[i].Center[j];
        }
    }
    //slave不需要把数据写入其中，但是他需要从属于它的tempdata里面读取center
    return true;
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
    for(int i = 0;i < Point_Num;i++){//把符合条件的样本点加入该类
        int index = 0;
        double dis = INF;//这里有问题，不应该每次都是INF,应该记录每个point的最小值
        for(int j = 0;j < Cluster_Num;j++){
            if(Distance(i,j) < dis){
                dis = Distance(i,j);
                index = j;
            }
        }
        std::cout << "Point " << i << " belong to "<< Cluster_Index << " slave..."<<std::endl;
        Slave_Acluster.Member[Slave_Acluster.Number++] = i;
    }
    return 0;
}

double S_Kmeans::Distance(int p,int c)
{
    double dis = 0.0;
    for(int j = 0;j < Point_Dimension;j++){
        dis += (Point[p][j]-All_Cluster[c].Center[j])*(Point[p][j]-All_Cluster[c].Center[j]);
    }
    return sqrt(dis);
}

int S_Kmeans::Combiner()
{
    memset(Tempcenter,0,sizeof(Tempcenter));
    for(int k = 0;k < Point_Dimension;k++){
        for(int j = 0 ;j < Slave_Acluster.Number;j++){
            Tempcenter[k] += Point[j][k];
        }
    }
    return 0;
}

int S_Kmeans::Reducer()
{
    for(int i = 0;i < Point_Dimension;i++){
        Tempcenter[i] /= Slave_Acluster.Number;
    }
    std::string filename = "tempdata_";
    std::string number = std::to_string(Cluster_Index);
    filename += number;
    filename += ".txt";
    ofstream outfile;
    outfile.open(filename);
    for(int i = 0;i < Point_Dimension;i++){
        outfile << Tempcenter[i];
        outfile << " ";
    }
    return 0;
}

int main(int argc,char* argv[])
{
    for(int i = 0;i < argc;i++){
        std::cout << "param "<< i << " = "<<argv[i] << std::endl;
    }
    std::cout << "This is the " << argv[1] << " th slave..."<<std::endl;
    S_Kmeans *skmeans = new S_Kmeans();
    skmeans -> Set_Cluster_Index(atoi(argv[1]));
    std::cout << "Cluster_Index = " << skmeans -> Get_Cluster_Index() << std::endl;
    skmeans -> ReadData();
    skmeans -> Mapper();
    skmeans -> Combiner();
    skmeans -> Reducer();
    return 0;
}
