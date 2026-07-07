#ifndef RISCV_H
#define RISCV_H

#include <stdint.h>

#define MEMORY_SIZE (16 * 1024 * 1024)
typedef uint32_t u32;
typedef uint32_t u16;
typedef uint8_t u8;

typedef int32_t i32;
typedef int32_t i16;
typedef int8_t i8;

#define DEBUG 0

#if DEBUG
#define debug_print(...) fprintf(stderr, __VA_ARGS__)
#else 
#define debug_print(...)
#endif

typedef struct {
    u32 x[32];
    u32 pc;
    u8 *memory;
} riscv_cpu_t;

typedef void (*syscall_t)(riscv_cpu_t *cpu);

void init_syscall_table(syscall_t *syscall_table);
void riscv_write(riscv_cpu_t *cpu);

#endif
