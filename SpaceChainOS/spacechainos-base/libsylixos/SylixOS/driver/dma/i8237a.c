/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: i8237a.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2016 �� 08 �� 26 ��
**
** ��        ��: Intel 8237A DMA ����֧��.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "../SylixOS/config/driver/drv_cfg.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if (LW_CFG_DMA_EN > 0) && (LW_CFG_DRV_DMA_I8237A > 0)
#include "i8237a.h"
/*********************************************************************************************************
  i8237 chan
*********************************************************************************************************/
typedef struct {
    LW_DMA_FUNCS    dmafuncs;
    I8237A_CTL      ctl;
} I8237A_DMA_FUNCS;
/*********************************************************************************************************
** ��������: i8237aReset
** ��������: i8237a ��������λ
** �䡡��  : uiChan        ͨ����
**           pdmafuncs     ������������
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static VOID  i8237aReset (UINT  uiChan, PLW_DMA_FUNCS  pdmafuncs)
{
    I8237A_DMA_FUNCS  *pi8237a = (I8237A_DMA_FUNCS *)pdmafuncs;
    
    (VOID)pi8237a;
}
/*********************************************************************************************************
** ��������: i8237aStatus
** ��������: ��� i8237a ������״̬
** �䡡��  : uiChan        ͨ����
**           pdmafuncs     ������������
** �䡡��  : ״̬
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  i8237aStatus (UINT  uiChan, PLW_DMA_FUNCS  pdmafuncs)
{
    I8237A_DMA_FUNCS  *pi8237a = (I8237A_DMA_FUNCS *)pdmafuncs;
    
    (VOID)pi8237a;
    
    return  (LW_DMA_STATUS_IDLE);
}
/*********************************************************************************************************
** ��������: i8237aTrans
** ��������: i8237a ����������һ�δ���
** �䡡��  : uiChan        ͨ����
**           pdmafuncs     ������������
**           pdmatMsg      ���������Ϣ
** �䡡��  : �Ƿ���ɹ�
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
static INT  i8237aTrans (UINT  uiChan, PLW_DMA_FUNCS  pdmafuncs, PLW_DMA_TRANSACTION  pdmatMsg)
{
    I8237A_DMA_FUNCS  *pi8237a = (I8237A_DMA_FUNCS *)pdmafuncs;
    
    (VOID)pi8237a;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: i8237aGetFuncs
** ��������: ��ʼ�� i8237a ��������
** �䡡��  : pctl       8237a ���ƿ�
** �䡡��  : DMA ��������
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
PLW_DMA_FUNCS  i8237aGetFuncs (I8237A_CTL *pctl)
{
    static I8237A_DMA_FUNCS    i8237a;
           PLW_DMA_FUNCS       pdmafuncs = &i8237a.dmafuncs;
    
    i8237a.ctl = *pctl;
    
    if (pdmafuncs->DMAF_pfuncReset == LW_NULL) {
        pdmafuncs->DMAF_pfuncReset  = i8237aReset;
        pdmafuncs->DMAF_pfuncTrans  = i8237aTrans;
        pdmafuncs->DMAF_pfuncStatus = i8237aStatus;
    }
    
    return  (&i8237a.dmafuncs);
}

#endif                                                                  /*  LW_CFG_DMA_EN > 0           */
                                                                        /*  LW_CFG_DRV_DMA_I8237A > 0   */
/*********************************************************************************************************
  END
*********************************************************************************************************/