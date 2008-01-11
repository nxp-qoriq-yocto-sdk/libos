
#include <libos/libos.h>
#include <libos/hcalls.h>

long hcall_ret0(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token);

long hcall_ret1(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token,uint32_t *retbuf);

long hcall_ret3(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token,uint32_t *retbuf);


int32_t fh_cpu_whoami(uint32_t *cpu_index)
{
	uint32_t retbuf[1];
	long status;

	status = hcall_ret1(0,0,0,0,0,0,0,0,FH_CPU_WHOAMI,&retbuf[0]);

	*cpu_index = retbuf[0];

	return status;
}

int32_t fh_lpar_get_status(int lpar_id, uint32_t *lpar_status,
	uint32_t *num_cpus, uint32_t *mem_size)
{
	uint32_t retbuf[3];
	long status;

	status = hcall_ret3(lpar_id,0,0,0,0,0,0,0,FH_LPAR_GET_STATUS,&retbuf[0]);

	*lpar_status = retbuf[0];
	*num_cpus = retbuf[1];
	*mem_size = retbuf[2];

	return status;
}

int32_t fh_byte_channel_send(uint32_t handle, int32_t count,
	uint32_t str0, uint32_t str1, uint32_t str2, uint32_t str3)
{
	long status;

	status = hcall_ret0(handle,count,str0,str1,str2,str3,0,0,FH_BYTE_CHANNEL_SEND);

	return status;
}
