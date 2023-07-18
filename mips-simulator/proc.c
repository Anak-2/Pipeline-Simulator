/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   proc.c                                                    */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <malloc.h>

#include "proc.h"
#include "mem.h"
#include "util.h"

#define rs_location 6
#define rt_location 11
#define rd_location 16
#define shamt_location 21
#define funct_location 26
#define imm_location 16
#define target_location 6

/***************************************************************/
/* System (CPU and Memory) info.                                             */
/***************************************************************/
struct MIPS32_proc_t g_processor;

/***************************************************************/
/* Fetch an instruction indicated by PC                        */
/***************************************************************/
int fetch(uint32_t pc)
{
    return mem_read_32(pc);
}

/***************************************************************/
/* TODO: Decode the given encoded 32bit data (word)            */
/***************************************************************/
struct inst_t decode(int word)
{
    struct inst_t inst;
    char *buffer = dec_to_bin(word);

    // Read opcode
    char *field_for_6bits = (char *)malloc(sizeof(char) * 7); // opcode, funct 와 같은 6 비트 필드를 위한 char 배열

    int opcode_length = 6;
    strncpy(field_for_6bits, buffer, opcode_length);

    field_for_6bits[opcode_length] = '\0';

    // assign instr.opcode
    inst.opcode = str_to_int(field_for_6bits);

    // rs, rt, rd 등 5 비트 필드를 담을 배열 공간
    char *field_for_5bits = (char *)malloc(sizeof(char) * 6);

    // target address 26 비트를 담을 배열 공간
    char *field_for_target = (char *)malloc(sizeof(char) * 27);

    // immediate 16 비트를 담을 배열 공간
    char *field_for_imm = (char *)malloc(sizeof(char) * 17);

    switch (inst.opcode)
    {
    // R-format
    case 0x0: //(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR

        // Read rs field
        strncpy(field_for_5bits, buffer + rs_location, 5);
        field_for_5bits[5] = '\0';
        inst.r_t.r_i.rs = str_to_int(field_for_5bits);

        // Read rt field
        strncpy(field_for_5bits, buffer + rt_location, 5);
        field_for_5bits[5] = '\0';
        inst.r_t.r_i.rt = str_to_int(field_for_5bits);

        // Read rd field
        strncpy(field_for_5bits, buffer + rd_location, 5);
        field_for_5bits[5] = '\0';
        inst.r_t.r_i.r_i.r.rd = str_to_int(field_for_5bits);

        // Read shamt field
        strncpy(field_for_5bits, buffer + shamt_location, 5);
        field_for_5bits[5] = '\0';
        inst.r_t.r_i.r_i.r.shamt = str_to_int(field_for_5bits);

        // Read funct field
        strncpy(field_for_6bits, buffer + funct_location, 6);
        field_for_6bits[6] = '\0';
        inst.func_code = str_to_int(field_for_6bits);

        break;

    // J-format
    case 0x2: //(0x000010)J
    case 0x3: //(0x000011)JAL

        // Read address field
        strncpy(field_for_target, buffer + target_location, 26);
        field_for_target[26] = '\0';
        inst.r_t.target = str_to_int(field_for_target);

        break;

    // I-format
    case 0x9:  //(0x001001)ADDIU
    case 0xc:  //(0x001100)ANDI
    case 0xf:  //(0x001111)LUI
    case 0xd:  //(0x001101)ORI
    case 0xb:  //(0x001011)SLTIU
    case 0x23: //(0x100011)LW
    case 0x2b: //(0x101011)SW
    case 0x4:  //(0x000100)BEQ
    case 0x5:  //(0x000101)BNE

        // Read rs field
        strncpy(field_for_5bits, buffer + rs_location, 5);
        field_for_5bits[5] = '\0';
        inst.r_t.r_i.rs = str_to_int(field_for_5bits);

        // Read rt field
        strncpy(field_for_5bits, buffer + rt_location, 5);
        field_for_5bits[5] = '\0';
        inst.r_t.r_i.rt = str_to_int(field_for_5bits);

        // Read immediate field
        strncpy(field_for_imm, buffer + imm_location, 16);
        field_for_imm[16] = '\0';
        inst.r_t.r_i.r_i.imm = str_to_int(field_for_imm);

        break;

    default:
        printf("Not available instruction\n");
        assert(0);
    }

    char tmp_buffer[33];
    memcpy(tmp_buffer, buffer, 33); // copy 32 bytes of buffer to tmp_buffer

    // Deallocate memory
    free(field_for_5bits);
    free(field_for_6bits);
    free(field_for_target);
    free(field_for_imm);

    // 인스트럭션의 32 비트 바이너리 값
    inst.value = str_to_int(tmp_buffer);

    return inst;
}

/***************************************************************/
/* TODO: Execute the decoded instruction                       */
/***************************************************************/
void execute(struct inst_t inst)
{
    if ((g_processor.pc - MEM_TEXT_START) >= (g_processor.input_insts << 2))
    {
        g_processor.running = FALSE;
    }

    // opcode를 통해서 포맷 구분
    switch (inst.opcode)
    {
    // R-format
    case 0x0:
        // funct 로 인스트럭션 구분
        switch (inst.func_code)
        {
        case 0x0: //(0x000000)SLL
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = g_processor.regs[inst.r_t.r_i.rt] << inst.r_t.r_i.r_i.r.shamt;
            break;
        case 0x2: //(0x000010)SRL
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = g_processor.regs[inst.r_t.r_i.rt] >> inst.r_t.r_i.r_i.r.shamt;
            break;
        case 0x08: //(0x001000)JR
            g_processor.pc = g_processor.regs[inst.r_t.r_i.rs];
            break;
        case 0x21: //(0x100001)ADDU
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = g_processor.regs[inst.r_t.r_i.rs] + g_processor.regs[inst.r_t.r_i.rt];
            break;
        case 0x23: //(0x100011)SUBU
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = g_processor.regs[inst.r_t.r_i.rs] - g_processor.regs[inst.r_t.r_i.rt];
            break;
        case 0x24: //(0x100100)AND
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = g_processor.regs[inst.r_t.r_i.rs] & g_processor.regs[inst.r_t.r_i.rt];
            break;
        case 0x25: //(0x100101)OR
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = g_processor.regs[inst.r_t.r_i.rs] | g_processor.regs[inst.r_t.r_i.rt];
            break;
        case 0x27: //(0x100111)NOR
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = ~(g_processor.regs[inst.r_t.r_i.rs] | g_processor.regs[inst.r_t.r_i.rt]);
            break;
        case 0x2b: //(0x101011)SLTU
            g_processor.regs[inst.r_t.r_i.r_i.r.rd] = (g_processor.regs[inst.r_t.r_i.rs] < g_processor.regs[inst.r_t.r_i.rt]) ? 1 : 0;
            break;
        default:
            break;
        }
        break;

    // J-format
    case 0x2: //(0x000010)J
        g_processor.pc = inst.r_t.target << 2;
        break;
    case 0x3:                                                   //(0x000011)JAL
        g_processor.regs[31] = g_processor.pc + BYTES_PER_WORD; // $ra
        g_processor.pc = inst.r_t.target << 2;
        break;

    // I-format
    case 0x9: //(0x001001)ADDIU
        g_processor.regs[inst.r_t.r_i.rt] = g_processor.regs[inst.r_t.r_i.rs] + inst.r_t.r_i.r_i.imm;
        break;
    case 0xc: //(0x001100)ANDI
        g_processor.regs[inst.r_t.r_i.rt] = g_processor.regs[inst.r_t.r_i.rs] & inst.r_t.r_i.r_i.imm;
        break;
    case 0xf: //(0x001111)LUI
        g_processor.regs[inst.r_t.r_i.rt] = inst.r_t.r_i.r_i.imm << 16;
        break;
    case 0xd: //(0x001101)ORI
        g_processor.regs[inst.r_t.r_i.rt] = g_processor.regs[inst.r_t.r_i.rs] | inst.r_t.r_i.r_i.imm;
        break;
    case 0xb: //(0x001011)SLTIU
        g_processor.regs[inst.r_t.r_i.rt] = (g_processor.regs[inst.r_t.r_i.rs] < inst.r_t.r_i.r_i.imm) ? 1 : 0;
        break;
    case 0x23: //(0x100011)LW
        g_processor.regs[inst.r_t.r_i.rt] = mem_read_32(g_processor.regs[inst.r_t.r_i.rs] + inst.r_t.r_i.r_i.imm);
        break;
    case 0x2b: //(0x101011)SW
        mem_write_32(g_processor.regs[inst.r_t.r_i.rs] + inst.r_t.r_i.r_i.imm, g_processor.regs[inst.r_t.r_i.rt]);
        break;
    case 0x4: //(0x000100)BEQ
        if (g_processor.regs[inst.r_t.r_i.rs] == g_processor.regs[inst.r_t.r_i.rt])
        {
            g_processor.pc += (inst.r_t.r_i.r_i.imm << 2);
        }
        break;
    case 0x5: //(0x000101)BNE
        if (g_processor.regs[inst.r_t.r_i.rs] != g_processor.regs[inst.r_t.r_i.rt])
        {
            g_processor.pc += (inst.r_t.r_i.r_i.imm << 2);
        }
        break;
    default:
        break;
    }
}

/***************************************************************/
/* Advance a cycle                                             */
/***************************************************************/
void cycle()
{
    int inst_reg;
    struct inst_t inst;

    // 1. fetch
    inst_reg = fetch(g_processor.pc);
    g_processor.pc += BYTES_PER_WORD;

    // 2. decode
    inst = decode(inst_reg);

    // 3. execute
    execute(inst);

    // 4. update stats
    g_processor.num_insts++;
    g_processor.ticks++;
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump()
{
    int k;

    printf("\n[INFO] Current register values :\n");
    printf("-------------------------------------\n");
    printf("PC: 0x%08x\n", g_processor.pc);
    printf("Registers:\n");
    for (k = 0; k < MIPS_REGS; k++)
        printf("R%d: 0x%08x\n", k, g_processor.regs[k]);
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate MIPS for n cycles                      */
/*                                                             */
/***************************************************************/
void run(int num_cycles)
{
    int i;

    if (g_processor.running == FALSE)
    {
        printf("[ERROR] Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("[INFO] Simulating for %d cycles...\n", num_cycles);
    for (i = 0; i < num_cycles; i++)
    {
        if (g_processor.running == FALSE)
        {
            printf("[INFO] Simulator halted\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate MIPS until HALTed                      */
/*                                                             */
/***************************************************************/
void go()
{
    if (g_processor.running == FALSE)
    {
        printf("[ERROR] Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("[INFO] Simulating...\n");
    while (g_processor.running)
        cycle();
    printf("[INFO] Simulator halted\n");
}
