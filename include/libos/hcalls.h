#ifndef LIBOS_HCALLS_H
#define LIBOS_HCALLS_H

#define FH_CPU_WHOAMI		0x0
#define FH_LPAR_GET_STATUS	0x7
#define FH_BYTE_CHANNEL_SEND	0x10

int32_t fh_cpu_whoami(uint32_t *cpu_index);

int32_t fh_lpar_get_status(int lpar_id, uint32_t *lpar_status,
        uint32_t *num_cpus, uint32_t *mem_size);

int32_t fh_byte_channel_send(uint32_t handle, int32_t count,
	uint32_t str0, uint32_t str1, uint32_t str2, uint32_t str3);

#endif
