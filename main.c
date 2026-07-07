#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMORY_SIZE (16 * 1024 * 1024)
typedef uint32_t u32;
typedef uint32_t u16;
typedef uint8_t u8;

typedef int32_t i32;
typedef int32_t i16;
typedef int8_t i8;

#define DEBUG 1

#if DEBUG
#define debug_print(...) fprintf(stderr, __VA_ARGS__)
#else 
#define debug_print(...)
#endif

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("no file input\n");
        return 0;
    }

    if (argc > 2) {
        printf("too many arguments\n");
    }

    char *filename = argv[1];

    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    unsigned long len = ftell(fp);
    rewind(fp);

    if (len > MEMORY_SIZE) {
        printf("file is larger than allocated memory\n");
        fclose(fp);
        return 1;
    }

    u8 *memory = calloc(MEMORY_SIZE, sizeof(u8));
    assert(memory != NULL);

    size_t bytesRead = fread(memory, 1, MEMORY_SIZE, fp);

    fclose(fp);

    if (bytesRead != len) {
        printf("failed to store file in memory\n");
        goto exit_failure;
    }


    u32 pc = 0;
    u32 x[32] = {0};

    while (1) {
        debug_print("pc: 0x%08x\n", pc);
        u32 instruction = 
            (u32)memory[pc] 
            | ((u32)memory[pc+1] << 8) 
            | ((u32)memory[pc+2] << 16) 
            | ((u32)memory[pc+3] << 24);

        // printf("instruction: 0x%08x\n", instruction);

        if (instruction == 0) {
            break;
        }

        u32 opcode = instruction & 0x7F;

        u32 next_pc = pc + 4;

        x[0] = 0;

        switch (opcode) {
            /* R-type arithmetic */
            case 0b0110011: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 funct3 = (instruction >> 12) & 0b111;
                u32 rs1 = (instruction >> 15) & 0b11111;
                u32 rs2 = (instruction >> 20) & 0b11111;
                u32 funct7 = (instruction >> 25) & 0x7F;

                switch (funct3) {
                    /* add and sub */
                    case 0x0:
                        if (funct7 == 0x00) {
                            x[rd] = ((i32)x[rs1] + (i32)x[rs2]);
                            debug_print("add x%u, x%u, x%u\n", rd, rs1, rs2);
                        } else if (funct7 == 0x20) {
                            debug_print("sub x%u, x%u, x%u\n", rd, rs1, rs2);
                            x[rd] = ((i32)x[rs1] - (i32)x[rs2]);
                        }
                        break;
                    /* xor */
                    case 0x4:
                        debug_print("xor x%u, x%u, x%u\n", rd, rs1, rs2);
                        x[rd] = x[rs1] ^ x[rs2];
                        break;
                    /* or */
                    case 0x6:
                        debug_print("or x%u, x%u, x%u\n", rd, rs1, rs2);
                        x[rd] = x[rs1] | x[rs2];
                        break;
                    /* and */
                    case 0x7:
                        debug_print("and x%u, x%u, x%u\n", rd, rs1, rs2);
                        x[rd] = x[rs1] & x[rs2];
                        break;
                    /* sll */
                    case 0x1:
                        debug_print("sll x%u, x%u, x%u\n", rd, rs1, rs2);
                        x[rd] = x[rs1] << x[rs2];
                        break;
                    /* shift right */
                    case 0x5:
                        if (funct7 == 0x00) {
                            /* logical srl */
                            debug_print("srl x%u, x%u, x%u\n", rd, rs1, rs2);
                            x[rd] = x[rs1] >> x[rs2];
                        } else if (funct7 == 0x20) {
                            /* arith sra */
                            debug_print("sra x%u, x%u, x%u\n", rd, rs1, rs2);
                            x[rd] = (u32)((i32)x[rs1] >> x[rs2]);
                        }
                        break;
                    /* slt */
                    case 0x2:
                        debug_print("slt x%u, x%u, x%u\n", rd, rs1, rs2);
                        x[rd] = ((i32)x[rs1] < (i32)x[rs2]) ? 1 : 0;
                        break;
                    /* sltu */
                    case 0x3:
                        debug_print("sltu x%u, x%u, x%u\n", rd, rs1, rs2);
                        x[rd] = ((u32)x[rs1] < (u32)x[rs2]) ? 1 : 0;
                        break;
                    default:
                        printf("invalid funct3 for R-type arithmetic\n");
                        goto exit_failure;
                }


                break;
            }
            /* I-type arithmetic */
            case 0b0010011: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 funct3 = (instruction >> 12) & 0b111;
                u32 rs1 = (instruction >> 15) & 0b11111;
                i32 imm = ((i32)instruction) >> 20;

                switch (funct3) {
                    /* addi */
                    case 0x0:
                        debug_print("addi x%u, x%u, %d\n", rd, rs1, imm);
                        x[rd] = (i32)x[rs1] + imm;
                        break;
                    /* xori */
                    case 0x4:
                        debug_print("xori x%u, x%u, %d\n", rd, rs1, imm);
                        x[rd] = x[rs1] ^ imm;
                        break;
                    /* ori */
                    case 0x6:
                        debug_print("ori x%u, x%u, %d\n", rd, rs1, imm);
                        x[rd] = x[rs1] | imm;
                        break;
                    /* andi */
                    case 0x7:
                        debug_print("andi x%u, x%u, %d\n", rd, rs1, imm);
                        x[rd] = x[rs1] & imm;
                        break;
                    /* slli */
                    case 0x1:
                        debug_print("slli x%u, x%u, %d\n", rd, rs1, imm);
                        x[rd] = x[rs1] << (imm & 0b11111);
                        break;
                    /* shift right */
                    case 0x5:
                        if ((imm >> 5) == 0x00) {
                            /* logical srli */
                            debug_print("srli x%u, x%u, %d\n", rd, rs1, imm);
                            x[rd] = x[rs1] >> (imm & 0b11111);
                        } else if ((imm >> 5) == 0x20) {
                            /* arith srai */
                            debug_print("srai x%u, x%u, %d\n", rd, rs1, imm);
                            x[rd] = (u32)((i32)x[rs1] >> (imm & 0b11111));
                        }
                        break;
                    /* slti */
                    case 0x2:
                        debug_print("slti x%u, x%u, %d\n", rd, rs1, imm);
                        x[rd] = ((i32)x[rs1] < imm) ? 1 : 0;
                        break;
                    /* sltiu */
                    case 0x3:
                        debug_print("sltiu x%u, x%u, %u\n", rd, rs1, imm);
                        x[rd] = (x[rs1] < (u32)imm) ? 1 : 0;
                        break;
                    default:
                        printf("invalid funct3 for I-type arithmetic\n");
                        goto exit_failure;
                }


                break;
            }
            /* I-type load */
            case 0b0000011: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 funct3 = (instruction >> 12) & 0b111;
                u32 rs1 = (instruction >> 15) & 0b11111;
                i32 imm = ((i32)instruction) >> 20;

                u32 addr = x[rs1] + imm;

                switch (funct3) {
                    case 0x0:
                        debug_print("lb x%u, %d(x%u)\n", rd, imm, rs1);
                        x[rd] = (i32)((i8)memory[addr]);
                        break;
                    case 0x1:
                        debug_print("lh x%u, %d(x%u)\n", rd, imm, rs1);
                        x[rd] = (i32)(i16)(((u16)(memory[addr+1]) << 8) | ((u8)memory[addr]));
                        break;
                    case 0x2:
                        debug_print("lw x%u, %d(x%u)\n", rd, imm, rs1);
                        x[rd] = ((u32)(memory[addr+3]) << 24) 
                                | ((u32)(memory[addr+2]) << 16) 
                                | ((u32)(memory[addr+1]) << 8) 
                                | ((u32)(memory[addr]));
                        break;
                    case 0x4:
                        debug_print("lbu x%u, %d(x%u)\n", rd, imm, rs1);
                        x[rd] = (u32)memory[addr];
                        break;
                    case 0x5:
                        debug_print("lhu x%u, %d(x%u)\n", rd, imm, rs1);
                        x[rd] = ((u32)(memory[addr+1]) << 8) | (u32)memory[addr];
                        break;
                    default:
                        printf("invalid funct3 for I-type load\n");
                        goto exit_failure;
                
                }
                
                break;
            }
            /* S-type store */
            case 0b0100011: {
                u32 imm12 = ((instruction >> 7) & 0b11111) | (((instruction >> 25) & 0x7F) << 5);
                i32 imm = (i32)(imm12 << 20) >> 20;
                u32 funct3 = (instruction >> 12) & 0b111;
                u32 rs1 = (instruction >> 15) & 0b11111;
                u32 rs2 = (instruction >> 20) & 0b11111;

                u32 addr = x[rs1] + imm;

                switch (funct3) {
                    case 0x0:
                        debug_print("sb x%u, %d(x%u)\n", rs2, imm, rs1);
                        memory[addr] = (x[rs2] & 0xFF);
                        break;
                    case 0x1:
                        debug_print("sh x%u, %d(x%u)\n", rs2, imm, rs1);
                        memory[addr] = (u8)(x[rs2] & 0xFF);
                        memory[addr+1] = (u8)((x[rs2] >> 8) & 0xFF);
                        break;
                    case 0x2:
                        debug_print("sw x%u, %d(x%u)\n", rs2, imm, rs1);
                        memory[addr] = (u8)(x[rs2] & 0xFF);
                        memory[addr+1] = (u8)((x[rs2] >> 8) & 0xFF);
                        memory[addr+2] = (u8)((x[rs2] >> 16) & 0xFF);
                        memory[addr+3] = (u8)((x[rs2] >> 24) & 0xFF);
                        break;
                    default:
                        printf("invalid funct3 for S-type store\n");
                        goto exit_failure;
                
                }

                break;
            }
            /* B-type branch */
            case 0b1100011: {
                u32 imm13 = (((instruction >> 7) & 0b1) << 11) 
                    | (((instruction >> 8) & 0b1111) << 1)
                    | (((instruction >> 25) & 0b111111) << 5)
                    | (((instruction >> 31) & 0b1) << 12);
                i32 imm = (i32)(imm13 << 19) >> 19;
                u32 funct3 = (instruction >> 12) & 0b111;
                u32 rs1 = (instruction >> 15) & 0b11111;
                u32 rs2 = (instruction >> 20) & 0b11111;

                switch (funct3) {
                    case 0x0:
                        debug_print("beq x%u, x%u, %d\n", rs1, rs2, imm);
                        if (x[rs1] == x[rs2]) next_pc = pc + imm;
                        break;
                    case 0x1:
                        debug_print("bne x%u, x%u, %d\n", rs1, rs2, imm);
                        if (x[rs1] != x[rs2]) next_pc = pc + imm;
                        break;
                    case 0x4:
                        debug_print("blt x%u, x%u, %d\n", rs1, rs2, imm);
                        if ((i32)x[rs1] < (i32)x[rs2]) next_pc = pc + imm;
                        break;
                    case 0x5:
                        debug_print("bge x%u, x%u, %d\n", rs1, rs2, imm);
                        if ((i32)x[rs1] >= (i32)x[rs2]) next_pc = pc + imm;
                        break;
                    case 0x6:
                        debug_print("bltu x%u, x%u, %d\n", rs1, rs2, imm);
                        if (x[rs1] < x[rs2]) next_pc = pc + imm;
                        break;
                    case 0x7:
                        debug_print("bgeu x%u, x%u, %d\n", rs1, rs2, imm);
                        if (x[rs1] >= x[rs2]) next_pc = pc + imm;
                        break;
                    default:
                        printf("invalid funct3 for B-type branch\n");
                        goto exit_failure;
                }

                break;
            }
            /* J-type jump and link */
            case 0b1101111: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 imm21 = 
                    ((instruction >> 12) & 0xFF) << 12
                    | ((instruction >> 20) & 0x1) << 11
                    | ((instruction >> 21) & 0x3FF) << 1
                    | ((instruction >> 31) & 0x1) << 20;
                i32 imm = (i32)(imm21 << 11) >> 11;

                debug_print("jal x%u, %d\n", rd, imm);

                x[rd] = pc+4;
                next_pc = pc + imm;

                break;
            }
            /* I-type jump and link reg */
            case 0b1100111: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 funct3 = (instruction >> 12) & 0b111;
                u32 rs1 = (instruction >> 15) & 0b11111;
                i32 imm = ((i32)instruction) >> 20;
                assert(funct3 == 0x0);

                debug_print("jalr x%u, x%u, %d\n", rd, rs1, imm);
                x[rd] = pc+4;
                next_pc = (x[rs1] + imm) & ~1;
                break;
            }
            /* U-type load upper imm */
            case 0b0110111: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 imm = instruction >> 12;

                debug_print("lui x%u, 0x%08x\n", rd, imm);

                x[rd] = imm << 12;

                break;
            }
            /* U-type add upper imm to pc */
            case 0b0010111: {
                u32 rd = (instruction >> 7) & 0b11111;
                u32 imm = instruction >> 12;

                debug_print("auipc x%u, 0x%08x\n", rd, imm);

                x[rd] = pc + (imm << 12);

                break;
            }
            /* I-type environment */
            case 0b1110011: {
                u32 imm = (instruction) >> 20;

                if (imm == 0x0) {
                    debug_print("ecall\n");
                } else if (imm == 0x1) {
                    debug_print("ebreak\n");
                } else {
                    printf("invalid imm for I-type environment\n");
                    free(memory);
                    return 1;
                }

                goto exit_success;
                break;
            }
            /* FENCE */
            case 0b0001111:
                debug_print("fence\n");
                break;
            default:
                printf("invalid opcode\n");
                goto exit_failure;
        }

        x[0] = 0;

#if DEBUG
        debug_print("x = {\n");
        for (int i=0; i<31; ++i) {
            debug_print("%08x ", x[i]);
            if ((i+1)%4 == 0) {
                debug_print("\n");
            }
        }
        debug_print("%08x\n}\n", x[31]);
#endif
    
        pc = next_pc;
    }

exit_success:
    free(memory);

    return 0;

exit_failure:
    free(memory);

    return 1;
}
