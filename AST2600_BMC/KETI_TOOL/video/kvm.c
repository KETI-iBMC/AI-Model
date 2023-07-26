#include "kvm.h"

void *KmRun(void *data)
{
	_run_km_server();
}

void *VRun(void *data)
{
	_run_v_server();
}

int main(int argc, char **argv)
{
	pthread_t kmrecv_thread;
	pthread_t vsend_thread;

	pthread_create(&kmrecv_thread, NULL, KmRun, NULL);
	pthread_create(&vsend_thread, NULL, VRun, NULL); 

	pthread_join(kmrecv_thread, NULL);
	pthread_join(vsend_thread, NULL);

	return (0); 
}