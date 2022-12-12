
#include <boost/log/trivial.hpp>
#include <redfish/hwcontrol.hpp>


vector<df> JSms::GetDf()
{
    struct statfs lstatfs;
    unsigned int i = 0;

    set<df, ltstr> smtab, smount;
    vector<df> vmtab;

    char line[DF_LINE_LEN];

    df ldf_data;
	FILE *fp = popen("df", "r");
    char *temp = (char *)malloc(sizeof(char)*256);
    //char *temp = new char[256];
	int num;
	//cout <<"GetDf 1"<<endl;
    while(fgets(line, DF_LINE_LEN, fp) != NULL)
    {
		//cout <<"GetDf 2"<<endl;
		if (i ==0 )
		{
			i ++;
			continue;	
		}
		//cout <<"GetDf 3"<<endl;
        sscanf(line, "%s	%d	%d	%d	%s	%s",ldf_data.filesystem, &ldf_data.blocks,&ldf_data.used,temp,temp, ldf_data.mount);
        //cout <<"GetDf 4"<<endl;
		vmtab.push_back(ldf_data);
		// printf("%s	%d	%d	%s \n",ldf_data.filesystem, ldf_data.blocks,ldf_data.used,ldf_data.mount);
		// cout <<"GetDf 5"<<endl;
	}
	fclose(fp);
    return vmtab;
}

// CPU사용율을 체크한다. 
// CPU사용율을 위해서는 현재의 jiffies값을 구하고 
// 과거의 jiffies의 값과 비교해서 얼마만큼 증가 했는지를 
// 판단해서 백분율로 나타내게 된다.
// 그러므로 이 메서드는 두번 호출해야 한다. 
// 처음 호출로 알아낸 jiffies값은 별도의 변수에 저장되고 
// sleep를 호출한 후 잠시후 다시 이 메서드를 호출해야 현재의 
// jiffies값과 과거의 jiffies를 이용한 계산이 가능해 진다. 
cpu_usage JSms::GetCpuIdle()
{
    FILE *fp = NULL;
    char line[80];
    cpu_usage lcpu_usage;
    char cpuid[32];
    int nulldata;
    int totaldiff;
    int diff[3];
    int cpu_num = 0;

    memset((void *)&lcpu_usage, 0x00, sizeof(cpu_usage));

    // /proc/stat파일을 읽어서 jiffies값을 계산한다. 
    fp = fopen("/proc/stat", "r");
    if(fp == NULL)
    {
        cout << "Cannot open stat" << endl;
        return lcpu_usage;
    }
    while(1)
    {
        fgets(line,80, fp);
        if (strstr(line, "cpu") == NULL)
        {
            fclose(fp);
            // cpu 갯수를 세팅하고 리턴한다. 
            lcpu_usage.countcpu = cpu_num -1;
            return lcpu_usage;
        }
        else
        {
            // /proc/stat에서 최초에 읽어 들이는 데이터는 
            // 전체 CPU에 대한 통계 데이터다. 
            // 그러므로 개별 CPU에 대한 데이터를 얻기 위해서는 처음 라인은 
            // 무시하고 넘어가야 한다. 
            if (cpu_num > 0)
            {
                // sscanf를 이용해서 jiffies값을 얻어온다. 
                // 그리고 과거의 jiffies값과의 연산을 통해서 
                // 각각의 jiffies가 얼마만큼 변했는지를 확인하고 
                // 이 값을 백분율로 나타낸다. 
                sscanf(line, "%s %d %d %d %d",
                               cpuid,
                               &mcurrent_usage[cpu_num-1][0],
                               &nulldata,
                               &mcurrent_usage[cpu_num-1][1],
                               &mcurrent_usage[cpu_num-1][2]);
                diff[0] = mcurrent_usage[cpu_num-1][0] - mold_usage[cpu_num-1][0];
                diff[1] = mcurrent_usage[cpu_num-1][1] - mold_usage[cpu_num-1][1];
                diff[2] = mcurrent_usage[cpu_num-1][2] - mold_usage[cpu_num-1][2];

                totaldiff = diff[0] + diff[1] + diff[2];
                lcpu_usage.cpuusage[cpu_num-1] = (diff[2]*100)/totaldiff;

                memcpy((void *)&mold_usage,
                                (void *)&mcurrent_usage,
                                sizeof(mcurrent_usage));
            }
            cpu_num ++;
        }
    }
    if(fp != NULL)
        fclose(fp);
}

mem_data JSms::GetMem()
{

	FILE * fp = NULL;
	char line[128];
	int index =0;
	mem_data lmem_data;
	int nulldata;
	char null[12];

	fp=fopen("/proc/meminfo","r");
	if(fp ==NULL)
	{
		cout<<"Can not open stat"<<endl;
		return lmem_data;
	}

	while(1)
	{
		fgets(line,128,fp);
		if(index == 0)
		{
			index ++;
			continue;
		}
		else if(index ==1)
		{
			sscanf(line, "%s %lu %d %lu",null, &lmem_data.real_ph,&nulldata,&lmem_data.free_ph);
		}
		else if (index ==2)
		{
			sscanf(line, "%s %lu %lu %lu",null,&nulldata,&lmem_data.swap_use,&lmem_data.swap_free);
		}
		else
		{
			fclose(fp);
			return lmem_data;
		}
		index ++;

	}
}
    
