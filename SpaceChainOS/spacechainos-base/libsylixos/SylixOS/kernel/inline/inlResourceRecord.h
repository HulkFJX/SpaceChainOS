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
** ��   ��   ��: inlResourceRecord.h
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2006 �� 12 �� 20 ��
**
** ��        ��: ����ϵͳռ����Դ�̵߳ǼǺ���

** BUG
2007.09.12  ����ɲü������ơ�
2007.11.21  �޸�ע��.
*********************************************************************************************************/

#ifndef  __INLRESOURCERECORD_H
#define  __INLRESOURCERECORD_H

/*********************************************************************************************************
  Long Wing �ں�ͨ����Դ������Դ����ŵ�˳��˳������Ա��������ķ���
*********************************************************************************************************/

#if (LW_CFG_EVENT_EN > 0) && (LW_CFG_MAX_EVENTS > 0)

static LW_INLINE VOID  _ResourceRecord (PLW_CLASS_EVENT  pevent)
{
    LW_TCB_GET_CUR(pevent->EVENT_pvTcbOwn);                             /*  ��¼ʹ���߳�                */
}

#endif                                                                  /*  (LW_CFG_EVENT_EN > 0)       */
                                                                        /*  (LW_CFG_MAX_EVENTS > 0)     */
#endif                                                                  /*  __INLRESOURCERECORD_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/