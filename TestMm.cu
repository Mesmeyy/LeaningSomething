#include <stdio.h>
#include <errno.h>
#include<time.h>
#include<cuda.h>
#include<cuda_runtime.h>
#include <device_launch_parameters.h>

#define BLOCK_NUM 32   //块数量
#define THREAD_NUM 256 // 每个块中的线程数

#define R_SIZE BLOCK_NUM * THREAD_NUM
//#define R_SIZE 4096
//#define R_SIZE 10

#define M_SIZE R_SIZE * R_SIZE

__global__ void mat_mul(int *mat1, int *mat2, int *result) {
    const int bid = blockIdx.x;
    const int tid = threadIdx.x;
    // 每个线程计算一行
    const int row = bid * THREAD_NUM + tid;
    for (int c = 0; c < R_SIZE; c++) {
        for (int n = 0; n < R_SIZE; n++) {
            result[row*R_SIZE+c] += mat1[row*R_SIZE+n] * mat2[n*R_SIZE+c];
        }
    }
}

int main(int argc, char *argv[]) {
    int num;
    cudaDeviceProp prop;
    cudaError_t cudaStatus;
    cudaStatus = cudaGetDeviceCount(&num);
    printf("deviceCount := %d\n",num);
    for(int i=0;i<num;i++){
            printf("i = %d\n",i);
            cudaGetDeviceProperties(&prop,i);
            printf("name:%s\n",prop.name);
            printf("totalGlobalMem:%d\n",prop.totalGlobalMem);
            printf("totalGlobalMem:%d\n",prop.totalGlobalMem/1024);
            printf("totalGlobalMem:%d\n",prop.totalGlobalMem/1024/1024);
            printf("totalGlobalMem:%d\n",prop.totalGlobalMem/1024/1024/1024);
            printf("multiProcessorCount:%d\n",prop.multiProcessorCount);
            printf("maxThreadsPerBlock:%d\n",prop.maxThreadsPerBlock);
            printf("major:%d,minor:%d\n",prop.major,prop.minor);
            printf("  Device PCI Domain ID / Bus ID / location ID:   %d / %d / %d\n",
		          prop.pciDomainID, prop.pciBusID, prop.pciDeviceID);

    }
    cudaStatus=cudaSetDevice(0);
    printf("return : %d\n",cudaStatus);
    cudaStatus = cudaSetDevice(0);
    printf("return : %d\n",cudaStatus);
    //cudaStatus=cudaSetDevice(1);
    //printf("return : %d\n",cudaStatus);
    int *mat1, *mat2, *result;
    int *g_mat1, *g_mat2, *g_mat_result;
    FILE *fp;
    char path[256]="\0";
    char buf[R_SIZE*16];
    int i=0;
    clock_t start,finish; // 定义变量
    double time;
    
    start=clock();     //start file copy
    
    if(argc!=2){
    	strcpy(path,"/data/006zzy/files/big_MM/bid_MM.txt");
    }else
        strcpy(path,argv[1]);
        
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("file is error=%d.",errno);
        return -1;
    }
    
    // 用一位数组表示二维矩阵
    mat1 = (int*) malloc(M_SIZE * sizeof(int));
    mat2 = (int*) malloc(M_SIZE * sizeof(int));
    result = (int*) malloc(M_SIZE * sizeof(int));

    // initialize
    /*
    for (i = 0; i < M_SIZE; i++) {
        mat1[i] = rand()/1000000;
        mat2[i] = rand()/1000000;
        result[i] = 0;
        
    }*/
    i=0;
    //int j=0;
    while(fgets(buf,sizeof(buf),fp)){
       //printf(" --%d ---%s\n",j++,buf);
       char *s =NULL;
       s=strtok(buf," \t\n");
       while(s != NULL)
       {
           //if(i>M_SIZE) break;
           mat1[i++] =atoi(s);
           s = strtok(NULL," \t\n");
       }
       buf[0]='\0';
    }
    //fclose(fp);
    
    //printf(" %d \n",mat1[M_SIZE-1]);
    i=0;
    //int j=0;
    while(fgets(buf,sizeof(buf),fp)){
       //printf(" --%d ---%s\n",j++,buf);
       char *s =NULL;
       s=strtok(buf," \t\n");
       while(s != NULL)
       {
           //if(i>M_SIZE) break;
           mat2[i++] =atoi(s);
           s = strtok(NULL," \t\n");
       }
       buf[0]='\0';
    }
    fclose(fp);
    finish=clock();    //file read over
    time=(double)(finish-start)/CLOCKS_PER_SEC;//time for file read
    
    start=clock();     //start count    

    cudaMalloc((void **)&g_mat1, sizeof(int) * M_SIZE);
    cudaMalloc((void **)&g_mat2, sizeof(int) * M_SIZE);
    cudaMalloc((void **)&g_mat_result, sizeof(int) * M_SIZE);

    cudaMemcpy(g_mat1, mat1, sizeof(int) * M_SIZE, cudaMemcpyHostToDevice);
    cudaMemcpy(g_mat2, mat2, sizeof(int) * M_SIZE, cudaMemcpyHostToDevice);

    mat_mul<<<BLOCK_NUM, THREAD_NUM>>>(g_mat1, g_mat2, g_mat_result);

    cudaMemcpy(result, g_mat_result, sizeof(int) * M_SIZE, cudaMemcpyDeviceToHost);
    cudaFree(g_mat1);
    cudaFree(g_mat2);
    cudaFree(g_mat_result);
    free(mat1);
    free(mat2);
    free(result);
    finish=clock();    //结束
    time=(double)(finish-start)/CLOCKS_PER_SEC;//计算运行时间
    printf("time=%lf\n",time);//输出运行时间
    return 0;
}
