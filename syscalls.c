#include <unistd.h>
#include <time.h>
#include "riscv.h"

void init_syscall_table(syscall_t *syscall_table) {
    syscall_table[64] = riscv_write;
}

/* ret val = a0 = x10, ret val2 = a1, args are a0 to a5*/
void riscv_write(riscv_cpu_t *cpu) {
    u32 fd = cpu->x[10];
    u32 guest_addr = cpu->x[11];
    u32 count = cpu->x[12];

    const void *buf = cpu->memory + guest_addr;

    cpu->x[10] = (u32)write(fd, buf, count);
}
