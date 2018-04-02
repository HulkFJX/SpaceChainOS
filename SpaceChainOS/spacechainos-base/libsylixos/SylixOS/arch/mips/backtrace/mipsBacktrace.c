/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: mipsBacktrace.c
**
** ��   ��   ��: Ryan.Xin (�Ž���)
**
** �ļ���������: 2013 �� 12 �� 09 ��
**
** ��        ��: MIPS ��ϵ���ܶ�ջ���� (��Դ�� glibc).
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "stdlib.h"
#include "system/signal/signalPrivate.h"
/*********************************************************************************************************
  Only GCC support now.
*********************************************************************************************************/
#ifdef   __GNUC__
#include "mipsBacktrace.h"
/*********************************************************************************************************
  ָ���
*********************************************************************************************************/
#define ADDUI_SP_INST           0x27bd0000
#define SW_RA_INST              0xafbf0000
#define JR_RA_INST              0x03e00008

#define INST_OP_MASK            0xffff0000
#define INST_OFFSET_MASK        0x0000ffff
/*********************************************************************************************************
** ��������: getEndStack
** ��������: ��ö�ջ������ַ
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static PVOID  getEndStack (VOID)
{
    PLW_CLASS_TCB  ptcbCur;

    LW_TCB_GET_CUR_SAFE(ptcbCur);

    return  ((PVOID)ptcbCur->TCB_pstkStackTop);
}
/*********************************************************************************************************
** ��������: backtrace
** ��������: ��õ�ǰ�������ջ
** �䡡��  : array     ��ȡ����
**           size      �����С
** �䡡��  : ��ȡ����Ŀ
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  backtrace (void **array, int size)
{
    unsigned long  *addr;
    unsigned long  *ra;
    unsigned long  *sp;
    unsigned long  *fp;
    unsigned long  *end_stack;
    unsigned long  *low_stack;
    unsigned long   inst;
    unsigned int    stack_size;
    unsigned int    ra_offset;
    unsigned int    cnt;

    if (!array || (size < 0)) {
        return  (-1);
    }

    __asm__ __volatile__("move %0, $ra\n"                               /*  ��� RA �� SP               */
                         "move %1, $sp\n"
                         :"=r"(ra), "=r"(low_stack));

    end_stack = getEndStack();                                          /*  ��ö�ջ������ַ            */

    stack_size = 0;
    for (addr = (unsigned long *)backtrace; ; addr++) {                 /*  �� backtrace ����������     */
        inst = *addr;
        if ((inst & INST_OP_MASK) == ADDUI_SP_INST) {                   /*  �ҵ����� ADDUI SP, SP, -40  */
                                                                        /*  �Ķ�ջ����ָ��              */
            stack_size = abs((short)(inst & INST_OFFSET_MASK));         /*  ȡ�� backtrace ������ջ��С */
            if (stack_size != 0) {
                break;
            }
        } else if (inst == JR_RA_INST) {                                /*  ������ JR RA ָ��           */
            return  (0);                                                /*  ֱ�ӷ���                    */
        } else {

        }
    }

    sp = (unsigned long *)((unsigned long)low_stack + stack_size);      /*  �����ߵ� SP                 */

    if (sp[-1] != ((unsigned long)ra)) {                                /*  backtrace ����� RA ����    */
        return  (0);
    }

    fp = (unsigned long *)sp[-2];
    if (fp != sp) {                                                     /*  backtrace ����� FP ����    */
        return  (0);                                                    /*  �� Release �汾û�б��� FP  */
    }

    if ((sp >= end_stack) || (sp <= low_stack)) {                       /*  SP ���Ϸ�                   */
        return  (0);
    }

    for (cnt = 0; cnt < size; cnt++) {                                  /*  backtrace                   */

        ra_offset  = 0;
        stack_size = 0;

        for (addr = ra;                                                 /*  �ڷ��ص�λ��������          */
             (ra_offset == 0) || (stack_size == 0);
             addr--) {

            inst = *addr;
            switch (inst & INST_OP_MASK) {

            case SW_RA_INST:                                            /*  �������� SW RA, 4(SP) ��ָ��*/
                ra_offset = abs((short)(inst & INST_OFFSET_MASK));      /*  ��ñ��� RA ʱ��ƫ����      */
                break;

            case ADDUI_SP_INST:                                         /*  �ҵ����� ADDUI SP, SP, -40  */
                                                                        /*  �Ķ�ջ����ָ��              */
                stack_size = abs((short)(inst & INST_OFFSET_MASK));     /*  ȡ���ú�����ջ��С          */
                break;

            default:
                break;
            }
        }

        ra = (unsigned long *)sp[ra_offset >> 2];                       /*  ȡ������� RA               */
        if (ra == 0) {                                                  /*  ���һ���޷��غ���          */
            break;
        }

        array[cnt] = ra;

        fp = (unsigned long *)sp[(ra_offset >> 2) - 1];                 /*  ȡ������� FP               */

        sp = (unsigned long *)((unsigned long)sp + stack_size);         /*  ��������ߵ� SP             */

        if (fp != sp) {                                                 /*  ����� SP �뱣��� FP ����ͬ*/
            sp = fp;                                                    /*  �� FP Ϊ׼                  */
        }

        if ((sp >= end_stack) || (sp <= low_stack)) {                   /*  SP ���Ϸ�                   */
            break;
        }
    }

    return  (cnt);
}

#endif                                                                  /*  __GNUC__                    */
/*********************************************************************************************************
  END
*********************************************************************************************************/