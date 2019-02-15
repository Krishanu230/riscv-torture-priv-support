// See LICENSE for license details.

#ifndef _ENV_PHYSICAL_SINGLE_CORE_H
#define _ENV_PHYSICAL_SINGLE_CORE_H

#include "../encoding.h"

//-----------------------------------------------------------------------
// Begin Macro
//-----------------------------------------------------------------------

#define RVTEST_RV64U                                                    \
  .macro init;                                                          \
  .endm

#define RVTEST_RV64UF                                                   \
  .macro init;                                                          \
  RVTEST_FP_ENABLE;                                                     \
  .endm

#define RVTEST_RV32U                                                    \
  .macro init;                                                          \
  .endm

#define RVTEST_RV32UF                                                   \
  .macro init;                                                          \
  RVTEST_FP_ENABLE;                                                     \
  .endm

#define RVTEST_RV64M                                                    \
  .macro init;                                                          \
  RVTEST_ENABLE_MACHINE;                                                \
  .endm

#define RVTEST_RV64S                                                    \
  .macro init;                                                          \
  RVTEST_ENABLE_SUPERVISOR;                                             \
  .endm

#define RVTEST_RV32M                                                    \
  .macro init;                                                          \
  RVTEST_ENABLE_MACHINE;                                                \
  .endm

#define RVTEST_RV32S                                                    \
  .macro init;                                                          \
  RVTEST_ENABLE_SUPERVISOR;                                             \
  .endm

#if __riscv_xlen == 64
# define CHECK_XLEN li a0, 1; slli a0, a0, 31; bgez a0, 1f; RVTEST_PASS; 1:
#else
# define CHECK_XLEN li a0, 1; slli a0, a0, 31; bltz a0, 1f; RVTEST_PASS; 1:
#endif

#define INIT_PMP                                                        \
  la t0, 1f;                                                            \
  csrw mtvec, t0;                                                       \
  li t0, -1;        /* Set up a PMP to permit all accesses */           \
  csrw pmpaddr0, t0;                                                    \
  li t0, PMP_NAPOT | PMP_R | PMP_W | PMP_X;                             \
  csrw pmpcfg0, t0;                                                     \
  .align 2;                                                             \
1:

#define INIT_SPTBR                                                      \
  la t0, 1f;                                                            \
  csrw mtvec, t0;                                                       \
  csrwi sptbr, 0;                                                       \
  .align 2;                                                             \
1:

#define DELEGATE_NO_TRAPS                                               \
  la t0, 1f;                                                            \
  csrw mtvec, t0;                                                       \
  csrwi medeleg, 0;                                                     \
  csrwi mideleg, 0;                                                     \
  csrwi mie, 0;                                                         \
  .align 2;                                                             \
1:

#define RVTEST_ENABLE_SUPERVISOR                                        \
  li a0, MSTATUS_MPP & (MSTATUS_MPP >> 1);                              \
  csrs mstatus, a0;                                                     \
  li a0, SIP_SSIP | SIP_STIP;                                           \
  csrs mideleg, a0;                                                     \

#define RVTEST_ENABLE_MACHINE                                           \
  li a0, MSTATUS_MPP;                                                   \
  csrs mstatus, a0;                                                     \

#define RVTEST_FP_ENABLE                                                \
  li a0, MSTATUS_FS & (MSTATUS_FS >> 1);                                \
  csrs mstatus, a0;                                                     \
  csrwi fcsr, 0;

#define RISCV_MULTICORE_DISABLE                                         \
  csrr a0, mhartid;                                                     \
  1: bnez a0, 1b;

#define EXTRA_TVEC_USER
#define EXTRA_TVEC_MACHINE
#define EXTRA_INIT
#define EXTRA_INIT_TIMER

#define INTERRUPT_HANDLER j other_exception /* No interrupts should occur */

#define RVTEST_CODE_BEGIN                                               \
        .section .text.init;                                            \
        .align  6;                                                      \
        .weak stvec_handler;                                            \
        .weak mtvec_handler;                                            \
        .globl _start;                                                  \
_start:                                                                 \
        /* reset vector */                                              \
        j reset_vector;                                                 \
        .align 2;                                                       \
trap_vector:                                                            \
        /* test whether the test came from pass/fail */                 \
        la ra, xreg_output_data;                                        \
        sd t5, 8(ra);                                                   \
        sd t6, 16(ra);                                                  \
        csrr t5, mcause;                                                \
        li t6, CAUSE_USER_ECALL;                                        \
        beq t5, t6, check_1;                                            \
        li t6, CAUSE_SUPERVISOR_ECALL;                                  \
        beq t5, t6, write_tohost;                                       \
        li t6, CAUSE_MACHINE_ECALL;                                     \
        beq t5, t6, write_tohost;                                       \
        /* if an mtvec_handler is defined, jump to it */                \
        la t5, mtvec_handler;                                           \
        beqz t5, 1f;                                                    \
        jr t5;                                                          \
        /* was it an interrupt or an exception? */                      \
  1:    csrr t5, mcause;                                                \
        bgez t5, handle_exception;                                      \
        INTERRUPT_HANDLER;                                              \
handle_exception:                                                       \
        /* we don't know how to handle whatever the exception was */    \
  check_1:                                                              \
        ld t6, 0(ra);                                                   \
        bnez t6, save_reg;                                              \
        j write_tohost;                                                 \
  other_exception:                                                      \
        /* some unhandlable exception occurred */                       \
  1:    ori TESTNUM, TESTNUM, 1337;                                     \
  write_tohost:                                                         \
        sw TESTNUM, tohost, t5;                                         \
        j write_tohost;                                                 \
  save_reg :                                                            \
        sd gp, 24(ra);                                                  \
        sd tp, 32(ra);                                                  \
        sd t0, 40(ra);                                                  \
        sd t1, 48(ra);                                                  \
        sd t2, 56(ra);                                                  \
        sd t3, 64(ra);                                                  \
        sd t4, 72(ra);                                                  \
        sd s0, 80(ra);                                                  \
        sd s1, 88(ra);                                                  \
        sd s2, 96(ra);                                                  \
        sd s3, 104(ra);                                                 \
        sd s4, 112(ra);                                                 \
        sd s5, 120(ra);                                                 \
        sd s6, 128(ra);                                                 \
        sd s7, 136(ra);                                                 \
        sd s8, 144(ra);                                                 \
        sd s9, 152(ra);                                                 \
        sd s10, 160(ra);                                                \
        sd a0, 168(ra);                                                 \
        sd a1, 176(ra);                                                 \
        sd a2, 184(ra);                                                 \
        sd a3, 192(ra);                                                 \
        sd a4, 200(ra);                                                 \
        sd a5, 208(ra);                                                 \
        sd a6, 216(ra);                                                 \
        sd a7, 224(ra);                                                 \
        sd s11, 232(ra);                                                \
        sd sp, 240(ra);                                                 \
        j stan_ecall;                                                   \
    stan_ecall:                                                         \
        csrr t6, mepc;                                                  \
        addi t6, t6, 4;                                                 \
        csrw mepc, t6;                                                  \
        j pseg_1;                                                       \
    .align 8;                                                           \
    load_reg:                                                           \
        ld gp, 24(ra);                                                  \
        ld tp, 32(ra);                                                  \
        ld t0, 40(ra);                                                  \
        ld t1, 48(ra);                                                  \
        ld t2, 56(ra);                                                  \
        ld t3, 64(ra);                                                  \
        ld t4, 72(ra);                                                  \
        ld s0, 80(ra);                                                  \
        ld s1, 88(ra);                                                  \
        ld s2, 96(ra);                                                  \
        ld s3, 104(ra);                                                 \
        ld s4, 112(ra);                                                 \
        ld s5, 120(ra);                                                 \
        ld s6, 128(ra);                                                 \
        ld s7, 136(ra);                                                 \
        ld s8, 144(ra);                                                 \
        ld s9, 152(ra);                                                 \
        ld s10, 160(ra);                                                \
        ld a0, 168(ra);                                                 \
        ld a1, 176(ra);                                                 \
        ld a2, 184(ra);                                                 \
        ld a3, 192(ra);                                                 \
        ld a4, 200(ra);                                                 \
        ld a5, 208(ra);                                                 \
        ld a6, 216(ra);                                                 \
        ld a7, 224(ra);                                                 \
        ld s11, 232(ra);                                                \
        ld sp, 240(ra);                                                 \
        ld t5, 8(ra);                                                   \
        ld t6, 16(ra);                                                  \
        mret;                                                           \
reset_vector:                                                           \
        RISCV_MULTICORE_DISABLE;                                        \
        INIT_SPTBR;                                                     \
        INIT_PMP;                                                       \
        DELEGATE_NO_TRAPS;                                              \
        li TESTNUM, 0;                                                  \
        la t0, trap_vector;                                             \
        csrw mtvec, t0;                                                 \
        CHECK_XLEN;                                                     \
        /* if an stvec_handler is defined, delegate exceptions to it */ \
        la t0, stvec_handler;                                           \
        beqz t0, 1f;                                                    \
        csrw stvec, t0;                                                 \
        li t0, (1 << CAUSE_LOAD_PAGE_FAULT) |                           \
               (1 << CAUSE_STORE_PAGE_FAULT) |                          \
               (1 << CAUSE_FETCH_PAGE_FAULT) |                          \
               (1 << CAUSE_MISALIGNED_FETCH) |                          \
               (1 << CAUSE_USER_ECALL) |                                \
               (1 << CAUSE_BREAKPOINT);                                 \
        csrw medeleg, t0;                                               \
        csrr t1, medeleg;                                               \
        bne t0, t1, other_exception;                                    \
1:      csrwi mstatus, 0;                                               \
        init;                                                           \
        EXTRA_INIT;                                                     \
        EXTRA_INIT_TIMER;                                               \
        la t0, 1f;                                                      \
        csrw mepc, t0;                                                  \
        csrr a0, mhartid;                                               \
        mret;                                                           \
1:

//-----------------------------------------------------------------------
// End Macro
//-----------------------------------------------------------------------

#define RVTEST_CODE_END                                                 \
        unimp

//-----------------------------------------------------------------------
// Pass/Fail Macro
//-----------------------------------------------------------------------

#define RVTEST_PASS                                                     \
        fence;                                                          \
        li TESTNUM, 1;                                                  \
        li x31, 0;                                                      \
        la x1, xreg_output_data;                                        \
      	sd x31, 0(x1);                                                  \
        ecall;

#define TESTNUM gp
#define RVTEST_FAIL                                                     \
        nop;                                                            \
        nop;                                                            \
        fence;                                                          \
1:      beqz TESTNUM, 1b;                                               \
        sll TESTNUM, TESTNUM, 1;                                        \
        or TESTNUM, TESTNUM, 1;                                         \
        ecall;

//-----------------------------------------------------------------------
// Data Section Macro
//-----------------------------------------------------------------------

#define EXTRA_DATA

#define RVTEST_DATA_BEGIN                                               \
        EXTRA_DATA                                                      \
        .pushsection .tohost,"aw",@progbits;                            \
        .align 6; .global tohost; tohost: .dword 0;                     \
        .align 6; .global fromhost; fromhost: .dword 0;                 \
        .popsection;                                                    \
        .align 4; .global begin_signature; begin_signature:

#define RVTEST_DATA_END .align 4; .global end_signature; end_signature:

#endif