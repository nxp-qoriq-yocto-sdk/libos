#ifndef LIBOS_HCALLS_H
#define LIBOS_HCALLS_H

#define FH_CPU_WHOAMI		1

#define FH_PARTITION_RESTART	5
#define FH_PARTITION_GET_STATUS	6
#define FH_PARTITION_START	7
#define FH_PARTITION_STOP	8
#define FH_PARTITION_MEMCPY	9
#define FH_VMPIC_SET_INT_CONFIG	10
#define FH_VMPIC_GET_INT_CONFIG	11

#define FH_VMPIC_SET_MASK	14
#define FH_VMPIC_GET_MASK	15
#define FH_VMPIC_GET_ACTIVITY	16
#define FH_VMPIC_EOI		17
#define FH_BYTE_CHANNEL_SEND	18
#define FH_BYTE_CHANNEL_RECEIVE	19
#define FH_BYTE_CHANNEL_POLL	20
#define FH_VMPIC_IACK		21
#define FH_GPIO_GET_CONFIG	22
#define FH_GPIO_SET_CONFIG	23
#define FH_GPIO_GET_GPDAT	24
#define FH_GPIO_SET_GPDAT	25
#define FH_GPIO_GET_GPIER	26
#define FH_GPIO_SET_GPIER	27
#define FH_GPIO_GET_GPIMR	28
#define FH_GPIO_SET_GPIMR	29

#define FH_PARTITION_SEND_DBELL	32

int32_t fh_cpu_whoami(uint32_t *cpu_index);

int32_t fh_partition_get_status(int lpar_id, uint32_t *lpar_status,
        uint32_t *num_cpus, uint32_t *mem_size);

int32_t fh_byte_channel_send(uint32_t handle, int32_t count,
	uint32_t str0, uint32_t str1, uint32_t str2, uint32_t str3);

int32_t fh_partition_send_dbell(uint32_t handle);

int32_t fh_byte_channel_receive(uint32_t handle,int32_t maxrecv,uint8_t *buf,int32_t *count);

int32_t fh_byte_channel_poll(uint32_t handle,uint32_t *rxavail,uint32_t *txavail);

int32_t fh_vmpic_set_mask(uint32_t intno, uint8_t mask);

int32_t fh_vmpic_set_int_config(uint32_t intno, uint8_t config,
		uint8_t prio, uint8_t destcpu);

int32_t fh_vmpic_iack(uint16_t *vector);

int32_t fh_vmpic_eoi(uint32_t intno);

#endif
