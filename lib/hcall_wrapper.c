
#include <libos/libos.h>
#include <libos/hcalls.h>

long hcall_ret0(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token);

long hcall_ret1(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token,uint32_t *retbuf);

long hcall_ret2(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token,uint32_t *retbuf);

long hcall_ret3(uint32_t arg0,uint32_t arg1,uint32_t arg2,
   uint32_t arg3,uint32_t arg4,uint32_t arg5,
   uint32_t arg6,uint32_t arg7,int32_t token,uint32_t *retbuf);

long hcall_ret5(uint32_t arg0,uint32_t arg1,uint32_t arg2,
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

int32_t fh_partition_get_status(int lpar_id, uint32_t *lpar_status,
	uint32_t *num_cpus, uint32_t *mem_size)
{
	uint32_t retbuf[3];
	long status;

	status = hcall_ret3(lpar_id,0,0,0,0,0,0,0,FH_PARTITION_GET_STATUS,&retbuf[0]);

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

int32_t fh_partition_send_dbell(uint32_t handle)
{
	long status;

	status = hcall_ret0(handle, 0, 0, 0, 0, 0, 0, 0, FH_PARTITION_SEND_DBELL);

	return status;
}


int32_t fh_byte_channel_receive(uint32_t handle,int32_t maxrecv, uint8_t *buf,int32_t *count)
{
	long status;
	uint32_t retbuf[5];
	uint32_t *wbuf = (uint32_t *)buf;

	status = hcall_ret5(handle,maxrecv,0,0,0,0,0,0,FH_BYTE_CHANNEL_RECEIVE,&retbuf[0]);

	*count = retbuf[0];
	wbuf[0] = retbuf[1];
	wbuf[1] = retbuf[2];
	wbuf[2] = retbuf[3];
	wbuf[3] = retbuf[4];

	return status;
}

int32_t fh_byte_channel_poll(uint32_t handle,uint32_t *rxavail,uint32_t *txavail)
{
	long status;
	uint32_t retbuf[2];

	status = hcall_ret2(handle,0,0,0,0,0,0,0,FH_BYTE_CHANNEL_POLL,&retbuf[0]);

	*rxavail = retbuf[0];
	*txavail = retbuf[1];

	return status;
}

int32_t fh_vmpic_set_mask(uint32_t intno, uint8_t mask)
{
	long status;

	status = hcall_ret0(intno,mask,0,0,0,0,0,0,FH_VMPIC_SET_MASK);

	return status;
}

int32_t fh_vmpic_set_int_config(uint32_t intno, uint8_t config,
		uint8_t prio, uint8_t destcpu)
{
	long status;

	status = hcall_ret0(intno,config,prio,destcpu,0,0,0,0,
		FH_VMPIC_SET_INT_CONFIG);

	return status;
}

int32_t fh_vmpic_iack(uint16_t *vector)
{
	long status;
	uint32_t retbuf[1];

	status = hcall_ret1(0,0,0,0,0,0,0,0,FH_VMPIC_IACK,&retbuf[0]);
	*vector = retbuf[0];

	return status;
}

int32_t fh_vmpic_eoi(uint32_t intno)
{
	long status;

	status = hcall_ret0(intno,0,0,0,0,0,0,0,FH_VMPIC_EOI);

	return status;
}
