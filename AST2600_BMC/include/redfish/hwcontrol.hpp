#pragma once
#include <redfish/stdafx.hpp>

#include<stdio.h>
#include<set>
#include<sys/vfs.h>


//gpio buildroot
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#define MOUNT_STR_LEN 255
#define DEVICE_STR_LEN 255
#define MAX_CPU_NUM 32
#define DF_LINE_LEN MOUNT_STR_LEN+DEVICE_STR_LEN

using namespace std;
/**
 * @brief 디스크 정보를 저장하기 위한 구조체
 * @author 기철
 * 
 */ 
struct df
{
    int blocks; //할당된 블럭
    int used ; //사용하고 있는 블럭
    int avail; //사용할 수있는 블럭
    char filesystem[DEVICE_STR_LEN];
    char mount[MOUNT_STR_LEN]; //마운트 디렉토리
    char device[DEVICE_STR_LEN]; //장치명

};


/**
 * @brief CPU 사용율 // 추후 사용용도로 작성
 * @author 기철
 * 
 */ 
struct cpu_usage
{
    int cpuid; //cpu id임
    int countcpu ; //등록된 씨피유수
    int cpuusage[MAX_CPU_NUM]; //씨피유의 사용률 저장
};

/**
 * @brief 메모리정보
 * @author 기철
 * 
 */ 
struct mem_data
{
    ulong real_ph; //PM 크기
    ulong free_ph; //PM 남은 크기
    ulong swap_use; //swap 크기
    ulong swap_free; //swap free사이즈
};

struct ltstr
{
    bool operator()(const df m1, const df m2){
        return strcmp(m1.mount,m2.mount) <0;
    }
};

class JSms
{
    private:
    int mcurrent_usage[MAX_CPU_NUM][3]; // cpu 사요율
    
    //바로전의 CPU 사용율
    int mold_usage[MAX_CPU_NUM][3];

    public :
    vector<df>  GetDf(); // 디스크정보
    cpu_usage GetCpuIdle(); //CPU 사용율
    mem_data GetMem();
    

    // 현재 cpu 사용률

};


