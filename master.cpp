#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
using namespace std;

const double INF=1e20;
const int MAXD = 1000;//最高维度数
const int MAXN = 1000;//最大样本点数
const int MAXC = 50;//类的最大个数

struct aCluster//类
{
    double Center[MAXD];//类的中心
    int Number;//类中包含的样本point数目
    int Member[MAXN];//类中包含的样本point的index
    //double Err; 分布式再启用此变量,想着所有类公用一个Err
};

class D_K_Means
{
    private:
    //注意：数组较大时，尽量使用new，否则会出现Segmentation fault (core dumped)错误。
    double Point[MAXN][MAXD];//第i个样本点的第j个属性
    aCluster Cluster[MAXC];//所有类
    //int Cluster_Num;//类的个数
    //int Point_Num;//样本数
    //int Point_Dimension;//样本属性维度
    aCluster TempCluster[MAXC];//临时存放类的中心
    //double Distance(int,int);

    public:
    int Cluster_Num;//类的个数
    int Point_Num;//点的个数
    int Point_Dimension;//样本属性维度
    int Slave_Num;//确定分片数目
    bool ReadData();//读取初始数据
    bool SplitData();//点数据分片存储
    int Init();//初始化K类的中心
    bool TempWrit();//将一轮迭代结束后的结果写入临时文件
    int Write_Result();//输出结果

    int Get_Cluster_Num();
};
/*
double D_K_Means::Distance(int p,int c)//编号为p的点与第c类的中心的距离
{
    double dis=0;
    for(int j=0;j<Point_Dimension;j++)
    {
        dis+=(Point[p][j]-Cluster[c].Center[j])*(Point[p][j]-Cluster[c].Center[j]);//算的是各个维度以后的综合距离，而不是单个维度
    }
    return sqrt(dis);
}
*/
bool D_K_Means::SplitData(){
    //确定分片数目
    double n = (double)Point_Num;
    int every_points = ceil(n/Slave_Num);//除最后一片，每片拿走every_points个数
    std::cout << "There are "<< every_points <<"points in one split..."<<std::endl;
    int count = 0;
    for(int i = 0;i < Slave_Num;i++){
        std::string number = std::to_string(i);
        std::string filename = "data_";
        filename += number;
        filename += ".txt";
        ofstream outfile;
        outfile.open(filename);
        std::cout << "In split there is creating filename = " << filename << std::endl;
        while(1){
            for(int j = 0;j < Point_Dimension;j++) {
                outfile << Point[count][j];
                outfile << " ";
            }
            count ++;
            if(!(count % every_points)) break;//满足一个完整的split的数据个数
            if(count == Point_Num) break;//所有点都划分完毕
        }
    }
}
bool D_K_Means::ReadData()//读取数据
{
    ifstream infile;
    infile.open("data.txt");
    infile >>Point_Num;
    infile >>Point_Dimension;
    infile >>Cluster_Num;

    for(int i = 0;i < Point_Num;i++)
    {
        for(int j = 0;j < Point_Dimension;j++)
        {
            infile >> Point[i][j];//读取第i个样本点的第j个属性
        }
    }
    infile.close();
    std::cout << "read data.txt is ok..."<<std::endl;
    Init();//初始化K个类的中心
    TempWrit();//将所有类的中心作为第一轮迭代前的数据写入临时文件
}

int D_K_Means::Init()//初始化K个类的中心
{
    srand(time(NULL));//抛随机种子
    for(int i = 0;i < Cluster_Num;i++)
    {
        int r = rand() % Point_Num;//随机选择所有样本点中的一个作为第i类的中心
        Cluster[i].Member[0]=r;
        for(int j = 0;j < Point_Dimension;j++)
        {
            Cluster[i].Center[j] = Point[r][j];
        }
    }
    std::cout <<"tempcenter choice is ok..."<<std::endl;
    return 0;
}


//该函数只能在master上进行，用于计算误差，以便得到新的聚类中心，同时确定是否需要继续迭代,这块在master里面将来需要改动
bool D_K_Means::TempWrit()//将所有类的中心写入临时文件
{
    double ERR=0.0;
    for(int i = 0;i < Cluster_Num;i++){
        memset(TempCluster[i].Center,0,sizeof(TempCluster[i].Center));
    }
    //tempdata,存放各个slave计算出来的中心值文件要么不存在，要么已经由各个slave计算并保存于文件，因此，这里使用读文件方式读取tempdata里面已经计算出来的最新的中心值
    for(int i = 0 ; i < Slave_Num;i++){
        std::string filename = "tempdata_";
        std::string number = std::to_string(i);
        filename += number;
        filename += ".txt";
        ifstream infile;
        infile.open(filename);
        if(!infile){
            //第一次执行TempWrit函数
        }else{
            double tempdata;
            for(int i = 0;i < Cluster_Num;i++){
                for(int j = 0;j < Point_Dimension;j++){
                    infile >> tempdata;
                    TempCluster[i].Center[j] += tempdata;
                }
            }
        }
    }
    //汇聚各个Slave后求平均中心值就是TempCluster最新值
    for(int i = 0;i < Cluster_Num;i++){
        for(int j = 0;j < Point_Dimension;j++){
            TempCluster[i].Center[j] /= Slave_Num;
        }
    }
    for(int i=0;i<Cluster_Num;i++)//更新Cluster.Center同时计算与上一次迭代的变化（取2范数的平方）
    {
        for(int j=0;j<Point_Dimension;j++)
        {
            double temperr = TempCluster[i].Center[j]-Cluster[i].Center[j];
            ERR+=(TempCluster[i].Center[j]-Cluster[i].Center[j])*(TempCluster[i].Center[j]-Cluster[i].Center[j]);
            Cluster[i].Center[j]=TempCluster[i].Center[j];
        }
    }
Writetemp:
    for(int i = 0;i < Cluster_Num;i++){
        std::string filename = "tempcenter.txt";//把新得到的center放入
        ofstream outfile;
        outfile.open(filename);
        for(int j = 0;j < Point_Dimension;j++){
            outfile << Cluster[i].Center[j];
            if(j != Point_Dimension-1) outfile << " ";
            else outfile << endl;
        }
        outfile.close();
    }
    std::cout<<"tempcenter files write is ok..."<<std::endl;
    std::cout << "Err = "<<ERR<<std::endl;
    if(ERR < 0.1) return true;
    else return false;
}

int  D_K_Means::Write_Result()//输出结果
{
    ofstream outfile;
    outfile.open("Result.txt");
    for(int i = 0;i < Cluster_Num;i++){
        for(int j = 0;j < Point_Dimension;j++){
            outfile << Cluster[i].Center[j];
            outfile << " ";
        }
        std::cout << std::endl;
    }
    outfile.close();
    return  0;
}

int D_K_Means::Get_Cluster_Num()
{
    return Cluster_Num;
}

int FrameWork(D_K_Means *kmeans)
{
    bool converged = false;
    int times = 1;
    kmeans->ReadData();
    kmeans->SplitData();
    kmeans->Slave_Num = 4;
    std::cout << "master has cluster number = "<< kmeans->Slave_Num<<std::endl;
    while(converged == false){
        for(int index = 0; index < kmeans->Slave_Num;index++){
            std::string command = "./slave ";
            std::string number = std::to_string(kmeans->Cluster_Num);
            command += " ";command += number;//CLuster_Num
            number = std::to_string(kmeans->Point_Num);
            command += " ";command += number;//Point_Num
            number = std::to_string(kmeans->Point_Dimension);
            command += " ";command += number;//Point_Dimension
            number = std::to_string(kmeans->Slave_Num);
            command += " ";command += number;//Slave_Num
            number = std::to_string(index);
            command += " ";command += number;//index
            std::cout << index << " slave start success..."<<std::endl;
            system(command.c_str());
        }
        sleep(2);
        converged = kmeans -> TempWrit();
    }
    kmeans->Write_Result();//把结果写入文件
    return 0;
}


int main(int argc, char *argv[])
{
    D_K_Means *kmeans=new D_K_Means();
    FrameWork(kmeans);//算法主体过程
    return 0;
}

/*分布式实现思路
1 按照文件分片运行Mapper()和Combiner()
2 在指定的几个机器上运行Reducer()，一个reduce计算一个或者多个聚类中心，得到新的聚类中心和ERR
3 Reducer结果通过TempWrit返回到master节点，进行ERR汇总，然后下发，进行下一次迭代
*/
