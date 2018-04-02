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
** ��   ��   ��: lwip_flowctl.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2017 �� 12 �� 08 ��
**
** ��        ��: ioctl ��������.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0 && LW_CFG_NET_FLOWCTL_EN > 0
#include "lwip/inet.h"
#include "net/flowctl.h"
/*********************************************************************************************************
** ��������: __rtIoctlInet
** ��������: SIOCADDRT / SIOCDELRT ������ӿ�
** �䡡��  : iFamily    AF_INET / AF_INET6
**           iCmd       SIOCADDRT / SIOCDELRT
**           pvArg      struct arpreq
** �䡡��  : ERROR or OK
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
INT  __fcIoctlInet (INT  iFamily, INT  iCmd, PVOID  pvArg)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}

#endif                                                                  /*  LW_CFG_NET_EN > 0           */
                                                                        /*  LW_CFG_NET_FLOWCTL_EN > 0   */
/*********************************************************************************************************
  END
*********************************************************************************************************/