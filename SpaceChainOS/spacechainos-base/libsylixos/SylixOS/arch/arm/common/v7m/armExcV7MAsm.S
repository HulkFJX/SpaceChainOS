;/*********************************************************************************************************
;**
;**                                    �й�������Դ��֯
;**
;**                                   Ƕ��ʽʵʱ����ϵͳ
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------�ļ���Ϣ--------------------------------------------------------------------------------
;**
;** ��   ��   ��: armExcV7MAsm.S
;**
;** ��   ��   ��: Jiao.JinXing (������)
;**
;** �ļ���������: 2017 �� 11 �� 14 ��
;**
;** ��        ��: ARMv7M ��ϵ�����쳣����.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>

#if defined(__SYLIXOS_ARM_ARCH_M__)

    FILE_BEGIN()

;/*********************************************************************************************************
;  ARMv7M ��ϵ��������쳣��������
;*********************************************************************************************************/

    IMPORT_LABEL(archResetHandle)
    IMPORT_LABEL(armv7mNMIIntHandle)
    IMPORT_LABEL(armv7mHardFaultHandle)
    IMPORT_LABEL(armv7mMemFaultHandle)
    IMPORT_LABEL(armv7mBusFaultHandle)
    IMPORT_LABEL(armv7mUsageFaultHandle)
    IMPORT_LABEL(armv7mDebugMonitorEntry)
    IMPORT_LABEL(armv7mPendSVEntry)
    IMPORT_LABEL(armv7mSysTickIntEntry)
    IMPORT_LABEL(armv7mSvcHandle)
    IMPORT_LABEL(armv7mReservedIntHandle)
    IMPORT_LABEL(armv7mIntHandle)

    EXPORT_LABEL(archNMIIntEntry)
    EXPORT_LABEL(archHardFaultEntry)
    EXPORT_LABEL(archMemFaultEntry)
    EXPORT_LABEL(archBusFaultEntry)
    EXPORT_LABEL(archUsageFaultEntry)
    EXPORT_LABEL(archDebugMonitorEntry)
    EXPORT_LABEL(archPendSVEntry)
    EXPORT_LABEL(archSysTickIntEntry)
    EXPORT_LABEL(archSvcEntry)
    EXPORT_LABEL(archReservedIntEntry)
    EXPORT_LABEL(vector)

;/*********************************************************************************************************
;  ������쳣������ vector ����Ϊ������, BSP ��������ʵ���쳣������
;*********************************************************************************************************/

    WEAK(vector)

;/*********************************************************************************************************
;  �쳣/�ж���ں�
;*********************************************************************************************************/

MACRO_DEF(IRQ_ENTRY  handle)
    ;/*
    ; *  R3 ��¼ BASEPRI, �����ж�
    ; *  �������ȼ������ж�, ����ʹ�� SVC
    ; */
    MRS     R3 , BASEPRI                                        ;/*  Set prio int mask to mask all       */
    MOV     R2 , #LW_CFG_CORTEX_M_INT_MASK                      ;/*  mask prio >= 2                      */
    MSR     BASEPRI,  R2

    ;/*
    ; *  R2 �����쳣����ǰʹ�õ� SP
    ; */
    TST     LR , #0x04                                          ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MRSEQ   R2 , MSP
    MRSNE   R2 , PSP

    ;/*
    ; *  �������µ� R3-11, R14 �Ĵ������쳣����ǰʹ�õ� SP
    ; */
    SUBS    R2 , R2, #0x28
    STM     R2 , {R3-R11, LR}

    ;/*
    ; *  �쳣����ǰʹ�õ� SP ���� R2
    ; */
    TST     LR , #0x04                                          ;/*  nonzero if context on process stack */
    IT      NE                                                  ;/*  change by hx , should use if then   */
    MSRNE   PSP, R2

    ;/*
    ; *  ����ʹ�õ��� MSP, ����������ζ����� MSP
    ; */
    MSR     MSP, R2

    ;/*
    ; *  R0, R2 ���õ� R4, R5
    ; */
    MOV     R4 , R0
    MOV     R5 , R2

    ;/*
    ; *  ���� API_InterEnter
    ; */
    LDR     R1 , =API_InterEnter
    BLX     R1

    ;/*
    ; *  ��������ж�Ƕ��, ��ת�� 2
    ; */
    CMP     R0 , #1
    BNE     1f

    ;/*
    ; *  ���� API_InterStackBaseGet, ��õ�ǰ CPU �ж϶�ջջ�������� MSP
    ; */
    LDR     R1 , =API_InterStackBaseGet
    BLX     R1
    MSR     MSP, R0

    ;/*
    ; *  R4, R5 �ָ��� R0, R1
    ; */
1:
    MOV     R0 , R4
    MOV     R1 , R5

    ;/*
    ; *  ���� handle(UINT32  uiVector, ARCH_REG_CTX  *pregctx)
    ; */
    LDR     R2 , =\handle
    BLX     R2

    ;/*
    ; *  �ָ� R3-11, R14 �Ĵ���
    ; */
    LDM     R0 , {R3-R11, LR}
    ADDS    R0 , R0, #0x28

    TST     LR , #0x04                                          ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MSREQ   MSP, R0
    MSRNE   PSP, R0

    MSR     BASEPRI, R3

    BX      LR
    MACRO_END()

;/*********************************************************************************************************
;  NMI �ж���ں���
;*********************************************************************************************************/

FUNC_DEF(archNMIIntEntry)
    ;/*
    ; *  R0 ���ڼ�¼�쳣��
    ; */
    MOV         R0 , #2
    IRQ_ENTRY   armv7mNMIIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  Hard Fault��ں���
;*********************************************************************************************************/

FUNC_DEF(archHardFaultEntry)
    MOV         R0 , #3
    IRQ_ENTRY   armv7mHardFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Mem Fault��ں���
;*********************************************************************************************************/

FUNC_DEF(archMemFaultEntry)
    MOV         R0 , #4
    IRQ_ENTRY   armv7mMemFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Bus Fault��ں���
;*********************************************************************************************************/

FUNC_DEF(archBusFaultEntry)
    MOV         R0 , #5
    IRQ_ENTRY   armv7mBusFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Usage Fault��ں���
;*********************************************************************************************************/

FUNC_DEF(archUsageFaultEntry)
    MOV         R0 , #6
    IRQ_ENTRY   armv7mUsageFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Debug Monitor ��ں���
;*********************************************************************************************************/

FUNC_DEF(archDebugMonitorEntry)
    MOV         R0 , #12
    IRQ_ENTRY   armv7mDebugMonitorHandle
    FUNC_END()

;/*********************************************************************************************************
;  PendSV �ж���ں���
;*********************************************************************************************************/

FUNC_DEF(archPendSVEntry)
    MOV         R0 , #14
    IRQ_ENTRY   armv7mPendSVHandle
    FUNC_END()

;/*********************************************************************************************************
;  SysTick �ж���ں���
;*********************************************************************************************************/

FUNC_DEF(archSysTickIntEntry)
    MOV         R0 , #15
    IRQ_ENTRY   armv7mSysTickIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  Reserved �ж���ں���
;*********************************************************************************************************/

FUNC_DEF(archReservedIntEntry)
    MOV         R0 , #7
    IRQ_ENTRY   armv7mReservedIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  16 ~ 255 ���ж���ں���
;*********************************************************************************************************/

FUNC_DEF(archIntEntryCommon)
    IRQ_ENTRY   armv7mIntHandle
    FUNC_END()

    .irp        Id,                                   \
                                   16,  17,  18,  19, \
     20,  21,  22,  23,  24,  25,  26,  27,  28,  29, \
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39, \
     40,  41,  42,  43,  44,  45,  46,  47,  48,  49, \
     50,  51,  52,  53,  54,  55,  56,  57,  58,  59, \
     60,  61,  62,  63,  64,  65,  66,  67,  68,  69, \
     70,  71,  72,  73,  74,  75,  76,  77,  78,  79, \
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89, \
     90,  91,  92,  93,  94,  95,  96,  97,  98,  99, \
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, \
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, \
    230, 231, 232, 233, 234, 235, 236, 237, 238, 239, \
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, \
    250, 251, 252, 253, 254, 255
    EXPORT_LABEL(archIntEntry\Id)
FUNC_DEF(archIntEntry\Id)
    MOV         R0 , #\Id
    B           archIntEntryCommon
    FUNC_END()
    .endr

;/*********************************************************************************************************
;  SVC ��ں���
;*********************************************************************************************************/

FUNC_DEF(archSvcEntry)
    ;/*
    ; *  R0 ���ڼ�¼�쳣��
    ; */

    ;/*
    ; *  R3 ��¼ BASEPRI, �����ж�
    ; *  �������ȼ������ж�, ����ʹ�� SVC
    ; */
    MRS     R3 , BASEPRI                                        ;/*  Set prio int mask to mask all       */
    MOV     R2 , #LW_CFG_CORTEX_M_INT_MASK                      ;/*  mask prio >= 2                      */
    MSR     BASEPRI,  R2

    ;/*
    ; *  R2 �����쳣����ǰʹ�õ� SP
    ; */
    TST     LR , #0x04                                          ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MRSEQ   R2 , MSP
    MRSNE   R2 , PSP

    ;/*
    ; *  �������µ� R3-11, R14 �Ĵ������쳣����ǰʹ�õ� SP
    ; */
    SUBS    R2 , R2, #0x28
    STM     R2 , {R3-R11, LR}

    ;/*
    ; *  �쳣����ǰʹ�õ� SP ���� R2
    ; */
    TST     LR , #0x04                                          ;/*  nonzero if context on process stack */
    IT      NE
    MSRNE   PSP, R2

    ;/*
    ; *  ����ʹ�õ��� MSP, ����������ζ����� MSP
    ; */
#if LW_CFG_SMP_EN > 0
    MOV     R9 , R0
    LDR     R2 , =_SchedSafeStack                               ;/*  _SchedSafeStack();                  */
    BLX     R2
    MSR     MSP, R0
    MOV     R0 , R9
#else
    MSR     MSP, R2
#endif

    ;/*
    ; *  R2 ���õ� R1
    ; */
    MOV     R1 , R2

    ;/*
    ; *  ���� armv7mSvcHandle(UINT32  uiVector, ARCH_REG_CTX  *pregctx)
    ; */
    LDR     R2 , =armv7mSvcHandle
    BLX     R2

    ;/*
    ; *  �ָ� R3-11, R14 �Ĵ���
    ; */
    LDM     R0 , {R3-R11, LR}
    ADDS    R0 , R0, #0x28

    TST     LR , #0x04                                          ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MSREQ   MSP, R0
    MSRNE   PSP, R0

    MSR     BASEPRI, R3

    BX      LR
    FUNC_END()

;/*********************************************************************************************************
;  �쳣������
;*********************************************************************************************************/

    SECTION(.vector)

FUNC_DEF(vector)
    .long   __stack_end                                         ;/*  0  - Reset stack pointer            */
    .long   archResetHandle                                     ;/*  1  - Reset                          */
    .long   archNMIIntEntry                                     ;/*  2  - NMI                            */
    .long   archHardFaultEntry                                  ;/*  3  - HardFault                      */
    .long   archMemFaultEntry                                   ;/*  4  - MemManage                      */
    .long   archBusFaultEntry                                   ;/*  5  - BusFault                       */
    .long   archUsageFaultEntry                                 ;/*  6  - UsageFault                     */
    .long   archReservedIntEntry                                ;/*  7  - Reserved                       */
    .long   archReservedIntEntry                                ;/*  8  - Reserved                       */
    .long   archReservedIntEntry                                ;/*  9  - Reserved                       */
    .long   archReservedIntEntry                                ;/*  10 - Reserved                       */
    .long   archSvcEntry                                        ;/*  11 - SVCall                         */
    .long   archDebugMonitorEntry                               ;/*  12 - Debug Monitor                  */
    .long   archReservedIntEntry                                ;/*  13 - Reserved                       */
    .long   archPendSVEntry                                     ;/*  14 - PendSV                         */
    .long   archSysTickIntEntry                                 ;/*  15 - SysTick                        */
    .irp    Id,                                       \
                                   16,  17,  18,  19, \
     20,  21,  22,  23,  24,  25,  26,  27,  28,  29, \
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39, \
     40,  41,  42,  43,  44,  45,  46,  47,  48,  49, \
     50,  51,  52,  53,  54,  55,  56,  57,  58,  59, \
     60,  61,  62,  63,  64,  65,  66,  67,  68,  69, \
     70,  71,  72,  73,  74,  75,  76,  77,  78,  79, \
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89, \
     90,  91,  92,  93,  94,  95,  96,  97,  98,  99, \
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, \
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, \
    230, 231, 232, 233, 234, 235, 236, 237, 238, 239, \
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, \
    250, 251, 252, 253, 254, 255
    .long   archIntEntry\Id
    .endr
    FUNC_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/