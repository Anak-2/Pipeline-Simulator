/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   sce212sim.c                                               */
/*   Adapted from CS311@KAIST                                  */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*    DO NOT MODIFY THIS FILE!                                  */
/*    You should only modify the run.c, run.h and util.h file!  */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "util.h"
#include "parse.h"
#include "run.h"

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE *prog;
    int ii, word;
    char buffer[33];
    //to notifying data & text segment size
    int flag = 0;
    int text_index = 0;
    int data_index = 0;

    /* Open program file. */
    prog = fopen(program_filename, "r");

    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    ii = 0;

    //read 32bits + '\0' = 33
    while (fgets(buffer,33,prog) != NULL) {

        if (flag == 0) {

            //check text segment size
            text_size = fromBinary(buffer);
            NUM_INST = text_size/4;
            //initial memory allocation of text segment
            INST_INFO = malloc(sizeof(instruction)*NUM_INST);
            init_inst_info();

        } else if(flag == 1) {

            //check data segment size
            data_size = fromBinary(buffer);

        } else {

            if (ii < text_size) {
                INST_INFO[text_index++] = parsing_instr(buffer, ii);
            } else if(ii < text_size + data_size) {
                parsing_data(buffer, ii-text_size);
            } else {
                //Do not enter this case
                //assert(0);
                //However, there is a newline in the input file
            }
            ii += 4;
        }
        flag++;
    }

    CURRENT_STATE.PC = MEM_TEXT_START;
    //printf("Read %d words from program into memory.\n\n", ii/4);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename) {
    int i;

    init_memory();
    load_program(program_filename);

    INSTRUCTION_COUNT = 0;
    CYCLE_COUNT = 0;
    BR_BIT = TRUE;
    FORWARDING_BIT = TRUE; 

    for (i = 0; i < PIPE_STAGE; i++){
        CURRENT_STATE.PIPE[i] = 0;
        CURRENT_STATE.PIPE_STALL[i] = FALSE;
    }

    CURRENT_STATE.IF_ID.Instr = 0;
    CURRENT_STATE.IF_ID.NPC = 0;
    CURRENT_STATE.ID_EX.NPC = 0;
    CURRENT_STATE.ID_EX.REG1 = 0;
    CURRENT_STATE.ID_EX.REG2 = 0;
    CURRENT_STATE.ID_EX.IMM = 0;
    CURRENT_STATE.EX_MEM.ALU_OUT = 0;
    CURRENT_STATE.EX_MEM.BR_TARGET = 0;
    CURRENT_STATE.MEM_WB.ALU_OUT = 0;
    

    RUN_BIT = TRUE;
    FETCH_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    char** tokens;
    int count = 1;
    int addr1 = 0;
    int addr2 = 0;
    int num_inst = 0;
    int i = 100;		//for loop

    int mem_dump_set = 0;
    int debug_set = 0;
    int num_inst_set = 0;
    int pipe_dump_set = 0;

    /* Error Checking */
    if (argc < 2) {
        printf("Usage: %s [-nobp] [-nof] [-m addr1:addr2] [-d] [-p] [-n num_instr] inputBinary\n", argv[0]);
        exit(1);
    }

    initialize(argv[argc-1]);

    //for checking parse result

    while (count != argc-1) {
        if (strcmp(argv[count], "-m") == 0) {
            tokens = str_split(argv[++count], ':');

            addr1 = (int)strtol(*(tokens), NULL, 16);
            addr2 = (int)strtol(*(tokens+1), NULL, 16);
            mem_dump_set = 1;
        } else if (strcmp(argv[count], "-d") == 0) {
            debug_set = 1;
        } else if (strcmp(argv[count], "-n") == 0) {
            num_inst = (int)strtol(argv[++count], NULL, 10);
            num_inst_set = 1;
        } else if (strcmp(argv[count], "-p") == 0) {
//		num_inst = (int)strtol(argv[++count], NULL, 10);
	        pipe_dump_set = 1;	
        } else {
            printf("Usage: %s [-m addr1:addr2] [-d] [-p] [-n num_instr] inputBinary\n", argv[0]);
            exit(1);
        }
        count++;
    }

    if (num_inst_set) {
        MAX_INSTRUCTION_NUM = num_inst;
    } else {
        MAX_INSTRUCTION_NUM = i;
    }

    if (num_inst_set && num_inst <= 0) {
        printf("Error: The number of instructnons should be positive integer\n");
        return -1;
    }

    if (debug_set || pipe_dump_set) {

        printf("Simulating for %llu instructions...\n\n", MAX_INSTRUCTION_NUM);

        while (RUN_BIT) {
            cycle();

            if (pipe_dump_set) {
                pdump();
            }

            if (debug_set) {
                rdump();	
            }
        }

        if(!debug_set) {
            rdump();
        }

        if(mem_dump_set) {
            mdump(addr1, addr2);
        }
        printf("Simulator halted after %llu cycles\n\n", CYCLE_COUNT);

    } else {
        run();
        rdump();

        if(mem_dump_set) {
            mdump(addr1, addr2);
        }
    }

    return 0;
}
