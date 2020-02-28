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
    //double Err; 分布式再启用此变量
};

class D_K_Means
{
    private:
    //注意：数组较大时，尽量使用new，否则会出现Segmentation fault (core dumped)错误。
    double Point[MAXN][MAXD];//第i个样本点的第j个属性
    aCluster Cluster[MAXC];//所有类
    int Cluster_Num;//类的个数
    int Point_Num;//样本数
    int Point_Dimension;//样本属性维度
    aCluster TempCluster[MAXC];//临时存放类的中心
    double Distance(int,int);

    public:
    bool ReadData();//读取初始数据
    void Init();//初始化K类的中心
    void Mapper();
    void Combiner();
    void Reducer();
    bool TempWrit();//将一轮迭代结束后的结果写入临时文件
    void Write_Result();//输出结果

    int Get_Cluster_Num();
};

double D_K_Means::Distance(int p,int c)//编号为p的点与第c类的中心的距离
{
    double dis=0;
    for(int j=0;j<Point_Dimension;j++)
    {
        dis+=(Point[p][j]-Cluster[c].Center[j])*(Point[p][j]-Cluster[c].Center[j]);//算的是各个维度以后的综合距离，而不是单个维度
    }
    return sqrt(dis);
}

bool D_K_Means::ReadData()//读取数据
{
    ifstream infile;
    infile.open("data.txt");
    infile >>Point_Num;
    infile >>Point_Dimension;
    infile >>Cluster_Num;

    for(int i=0;i<Point_Num;i++)
    {
        for(int j=0;j<Point_Dimension;j++)
        {
            infile >>Point[i][j];//读取第i个样本点的第j个属性
        }
    }
    infile.close();
    std::cout << "read data.txt is ok..."<<std::endl;
    Init();//初始化K个类的中心
    TempWrit();//将所有类的中心作为第一轮迭代前的数据写入临时文件
}

void D_K_Means::Init()//初始化K个类的中心
{
    srand(time(NULL));//抛随机种子
    for(int i=0;i<Cluster_Num;i++)
    {
        int r=rand()%Point_Num;//随机选择所有样本点中的一个作为第i类的中心
        Cluster[i].Member[0]=r;
        for(int j=0;j<Point_Dimension;j++)
        {
            TempCluster[i].Center[j]=Point[r][j];
        }
    }
    std::cout <<"tempcenter choice ok..."<<std::endl;
}

void D_K_Means::Mapper()//求解每个类下的样本点
{
    /*FILE *fp;
    if((fp=fopen("TempData.txt","r"))==NULL)
    {
        cout<<"Open TempData.txt Error!"<<endl;
        exit(0);
    }*/
    ifstream infile;
    infile.open("TempData.txt");
    double temp;
    for(int i=0;i<Cluster_Num;i++)//读取中心
    {
        Cluster[i].Number=0;
        for(int j=0;j<Point_Dimension;j++)
        {
            infile>>temp;
            Cluster[i].Center[j]=temp;
        }
    }
    for(int i=0;i<Point_Num;i++)//重新计算所有类中的样本点
    {
        int index;
        double dis=INF;
        for(int j=0;j<Cluster_Num;j++)
        {
            if(Distance(i,j)<dis)
            {
                dis=Distance(i,j);
                index=j;
            }
        }
        Cluster[index].Member[Cluster[index].Number++]=i;
    }
    //fclose(fp);
    infile.close();
}

void D_K_Means::Combiner()
{
    int id;
    for(int i=0;i<Cluster_Num;i++)
    {
        memset(TempCluster[i].Center,0,sizeof(TempCluster[i].Center));
    }
#if 0 //以下代码错误
    for(int i;i<Cluster_Num;i++)
    {
        for(int j=0;j<Cluster[i].Number;j++)
        {
            for(int k=0;k<Point_Dimension;k++)
            {
                //TempCluster[i].Center[j] += Point[Cluster[i].Member[j]][k];
            }
        }
    }
#endif
    for(int i;i<Cluster_Num;i++) //对于每个聚类顶点，
    {
    	//计算聚类顶点的位置的和，即point(x1,x2,x3,...,xn)中的x1,x2等的值，等待reduce时计算平均值
    	//为什么现在不计算平均值，目的是为了分布式的开发，否则直接在函数结束前计算均值就可以
    	for(int k=0;k<Point_Dimension;k++)//计算每个维度的数字和
        {
            for(int j=0;j<Cluster[i].Number;j++) //每个聚类顶点中包含的原始数据点 
            {
                TempCluster[i].Center[k] += Point[Cluster[i].Member[j]][k];
            }
        }
    }
}


void D_K_Means::Reducer()
{
    for(int i=0;i<Cluster_Num;i++)
    {
        for(int j=0;j<Point_Dimension;j++)
        {
            TempCluster[i].Center[j]/=Cluster[i].Number;
        }
    }
}

//该函数只能在master上进行，用于计算误差，以便得到新的聚类中心，同时确定是否需要继续迭代,这块在master里面将来需要改动
bool D_K_Means::TempWrit()//将所有类的中心写入临时文件
{
    double ERR=0.0;
    ofstream outfile;
    outfile.open("TempData.txt");
    for(int i=0;i<Cluster_Num;i++)//将TempCluster的中心坐标复制到Cluster中，同时计算与上一次迭代的变化（取2范数的平方）
    {
        for(int j=0;j<Point_Dimension;j++)
        {
            ERR+=(TempCluster[i].Center[j]-Cluster[i].Center[j])*(TempCluster[i].Center[j]-Cluster[i].Center[j]);
            Cluster[i].Center[j]=TempCluster[i].Center[j];
        }
    }
    for(int i=0;i<Cluster_Num;i++)//将Cluster的中心坐标写入临时文件
    {
        for(int j=0;j<Point_Dimension;j++)
        {
            outfile<<Cluster[i].Center[j];
            if(j!=Point_Dimension-1) outfile<<" ";
            else outfile<<endl;
        }
    }
    outfile.close();
    std::cout<<"tempcenter write is ok..."<<std::endl;
    if(ERR<0.1) return true;
    else return false;
}

void D_K_Means::Write_Result()//输出结果
{
    /*(FILE *fp;
    if((fp=fopen("Result.txt","w"))==NULL)
    {
        cout<<"Open Result.txt Error!"<<endl;
        exit(0);
    }*/
    ofstream outfile;
    outfile.open("Result.txt");
    for(int i=0;i<Cluster_Num;i++)
    {
        cout<<"Cluster "<<i<<" : ";
        sort(Cluster[i].Member,(Cluster[i].Member)+Cluster[i].Number);//类内序号排序，方便输出
        for(int j=0;j<Cluster[i].Number;j++)
        {
            outfile<<Cluster[i].Member[j];
            if(j!=Cluster[i].Number-1) outfile<<" ";
            else outfile<<endl;
        }
    }
    //fclose(fp);
    outfile.close();
}

int D_K_Means::Get_Cluster_Num()
{
    return Cluster_Num;
}
void FrameWork(D_K_Means *kmeans)
{
    bool converged=false;
    int times=1;
    kmeans->ReadData();
    int slave_number = kmeans->Get_Cluster_Num();
    for(int i = 0;i < slave_number;i++){
        char *envp[] = {"sourcefile=data.txt", "BB=22", NULL};//只是这样使用execle，但是具体内容还未定
        execle("./slave","slave",NULL,envp);
    }
    while(converged==false)
    {
        kmeans->Mapper();
        kmeans->Combiner();
        kmeans->Reducer();
        converged=kmeans->TempWrit();//返回true时结束循环
    }
    kmeans->Write_Result();//把结果写入文件
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
