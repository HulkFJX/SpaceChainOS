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
** ��   ��   ��: lwip_ifparam.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2015 �� 09 �� 20 ��
**
** ��        ��: ����ӿ����ò�����ȡ.
**
** BUG:
2016.10.24  �����ӡ��Ϣ, �ɲ鿴��������.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  �ü�����
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
/*********************************************************************************************************
  ��������ļ���ʽ���� /etc/ifparam.ini

  [dm9000a]
  enable=1
  ipaddr=192.168.1.2
  netmask=255.255.255.0
  gateway=192.168.1.1
  default=1
  mac=00:11:22:33:44:55
  ipv6_auto_cfg=1         (����� SylixOS ��Ϊ IPv6 ·����, �� ipv6_auto_cfg=0)
  
  ����
  
  [dm9000a]
  enable=1
  dhcp=1
  mac=00:11:22:33:44:55

  resolver ��������ļ����� /etc/resolv.conf

  nameserver x.x.x.x
*********************************************************************************************************/
/*********************************************************************************************************
  �����ļ�λ��
*********************************************************************************************************/
#define LW_IFPARAM_PATH         "/etc/ifparam.ini"
#define LW_RESCONF_PATH         "/etc/resolv.conf"
#define LW_IFPARAM_ENABLE       "enable"
#define LW_IFPARAM_IPADDR       "ipaddr"
#define LW_IFPARAM_MASK         "netmask"
#define LW_IFPARAM_GW           "gateway"
#define LW_IFPARAM_MAC          "mac"
#define LW_IFPARAM_DEFAULT      "default"
#define LW_IFPARAM_DHCP         "dhcp"
#define LW_IFPARAM_IPV6_ACFG    "ipv6_auto_cfg"
/*********************************************************************************************************
  ini ����
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE     INIK_lineManage;
    PCHAR            INIK_pcKey;
    PCHAR            INIK_pcValue;
} LW_INI_KEY;
typedef LW_INI_KEY  *PLW_INI_KEY;

typedef struct {
    PLW_LIST_LINE    INIS_plineKey;
} LW_INI_SEC;
typedef LW_INI_SEC  *PLW_INI_SEC;
/*********************************************************************************************************
** ��������: __iniLoadSec
** ��������: װ��һ�������ļ�ָ����
** �䡡��  : pinisec       INI ���
**           fp            �ļ����
** �䡡��  : INI ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __iniLoadSec (PLW_INI_SEC  pinisec, FILE  *fp)
{
#define LW_INI_BUF_SZ       128

#define __IS_WHITE(c)       (c == ' ' || c == '\t' || c == '\r' || c == '\n')
#define __IS_END(c)         (c == PX_EOS)
#define __SKIP_WHITE(str)   while (__IS_WHITE(*str)) {  \
                                str++;  \
                            }
#define __NEXT_WHITE(str)   while (!__IS_WHITE(*str) && !__IS_END(*str)) { \
                                str++;  \
                            }

    PLW_INI_KEY     pinikey;
    CHAR            cBuf[LW_INI_BUF_SZ];
    PCHAR           pcLine;
    PCHAR           pcEnd;
    PCHAR           pcEqu;
    
    PCHAR           pcKey;
    size_t          stKeyLen;
    PCHAR           pcValue;
    size_t          stValueLen;
    
    for (;;) {
        pcLine = fgets(cBuf, LW_INI_BUF_SZ, fp);
        if (!pcLine) {
            break;
        }
        
        __SKIP_WHITE(pcLine);
        if (__IS_END(*pcLine) || (*pcLine == ';') || (*pcLine == '#')) {
            continue;
        }
        
        pcEnd = pcLine;
        __NEXT_WHITE(pcEnd);
        *pcEnd = PX_EOS;
        
        if (*pcLine == '[') {                                           /*  �Ѿ�����һ����              */
            break;
        }
        
        pcEqu = lib_strchr(pcLine, '=');
        if (!pcEqu) {
            continue;
        }
        
        *pcEqu = PX_EOS;
        pcEqu++;
        
        pcKey   = pcLine;
        pcValue = pcEqu;
        
        stKeyLen   = lib_strlen(pcKey);
        stValueLen = lib_strlen(pcValue);
        
        pinikey = (PLW_INI_KEY)__SHEAP_ALLOC(sizeof(LW_INI_KEY) + stKeyLen + stValueLen + 2);
        if (!pinikey) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            break;
        }
        
        pinikey->INIK_pcKey = (PCHAR)pinikey + sizeof(LW_INI_KEY);
        lib_strcpy(pinikey->INIK_pcKey, pcKey);
        
        pinikey->INIK_pcValue = pinikey->INIK_pcKey + stKeyLen + 1;
        lib_strcpy(pinikey->INIK_pcValue, pcValue);
        
        _List_Line_Add_Tail(&pinikey->INIK_lineManage, &pinisec->INIS_plineKey);
    }
}
/*********************************************************************************************************
** ��������: __iniLoad
** ��������: װ��һ�������ļ�
** �䡡��  : pcFile        �ļ���
**           pcSec         ��
** �䡡��  : INI ���
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static PLW_INI_SEC  __iniLoad (CPCHAR  pcFile, CPCHAR  pcSec)
{
    PLW_INI_SEC   pinisec;
    FILE         *fp;
    CHAR          cSec[LW_INI_BUF_SZ];
    CHAR          cBuf[LW_INI_BUF_SZ];
    PCHAR         pcLine;
    PCHAR         pcEnd;
    
    if (lib_strlen(pcSec) > (LW_INI_BUF_SZ - 3)) {
        return  (LW_NULL);
    }
    
    pinisec = (PLW_INI_SEC)__SHEAP_ALLOC(sizeof(LW_INI_SEC));
    if (!pinisec) {
        return  (LW_NULL);
    }
    lib_bzero(pinisec, sizeof(LW_INI_SEC));
    
    fp = fopen(pcFile, "r");
    if (!fp) {
        __SHEAP_FREE(pinisec);
        return  (LW_NULL);
    }
    
    snprintf(cSec, LW_INI_BUF_SZ, "[%s]", pcSec);
    
    for (;;) {
        pcLine = fgets(cBuf, LW_INI_BUF_SZ, fp);
        if (!pcLine) {
            goto    __error_handle;
        }
        
        __SKIP_WHITE(pcLine);
        if (__IS_END(*pcLine) || (*pcLine == ';') || (*pcLine == '#')) {
            continue;
        }
        
        pcEnd = pcLine;
        __NEXT_WHITE(pcEnd);
        *pcEnd = PX_EOS;
        
        if (lib_strcmp(cSec, pcLine)) {
            continue;
        
        } else {
            break;
        }
    }
    
    __iniLoadSec(pinisec, fp);
    
    fclose(fp);
    
    return  ((PLW_INI_SEC)pinisec);
    
__error_handle:
    fclose(fp);
    __SHEAP_FREE(pinisec);
    return  (LW_NULL);
}
/*********************************************************************************************************
** ��������: __iniUnload
** ��������: ж��һ�������ļ�
** �䡡��  : pinisec       INI ���
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static VOID  __iniUnload (PLW_INI_SEC  pinisec)
{
    PLW_INI_KEY     pinikey;
    
    while (pinisec->INIS_plineKey) {
        pinikey = _LIST_ENTRY(pinisec->INIS_plineKey, LW_INI_KEY, INIK_lineManage);
        pinisec->INIS_plineKey = _list_line_get_next(pinisec->INIS_plineKey);
        
        __SHEAP_FREE(pinikey);
    }
    
    __SHEAP_FREE(pinisec);
}
/*********************************************************************************************************
** ��������: __iniGetInt
** ��������: ���ָ������������ֵ
** �䡡��  : pinisec       INI ���
**           pcKey         ָ����
**           iDefault      Ĭ��ֵ
** �䡡��  : ��ȡ��ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static INT  __iniGetInt (PLW_INI_SEC  pinisec, CPCHAR  pcKey, INT  iDefault)
{
    PLW_INI_KEY     pinikey;
    PLW_LIST_LINE   pline;
    INT             iRet = iDefault;
    
    for (pline  = pinisec->INIS_plineKey;
         pline != LW_NULL;
         pline  = _list_line_get_next(pline)) {
         
        pinikey = _LIST_ENTRY(pline, LW_INI_KEY, INIK_lineManage);
        if (lib_strcmp(pinikey->INIK_pcKey, pcKey) == 0) {
            iRet = lib_atoi(pinikey->INIK_pcValue);
            break;
        }
    }
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: __iniGetStr
** ��������: ���ָ���������ַ���
** �䡡��  : pinisec       INI ���
**           pcKey         ָ����
**           pcDefault     Ĭ��ֵ
** �䡡��  : ��ȡ��ֵ
** ȫ�ֱ���:
** ����ģ��:
*********************************************************************************************************/
static CPCHAR  __iniGetStr (PLW_INI_SEC  pinisec, CPCHAR  pcKey, CPCHAR  pcDefault)
{
    PLW_INI_KEY     pinikey;
    PLW_LIST_LINE   pline;
    PCHAR           pcRet = (PCHAR)pcDefault;
    
    for (pline  = pinisec->INIS_plineKey;
         pline != LW_NULL;
         pline  = _list_line_get_next(pline)) {
         
        pinikey = _LIST_ENTRY(pline, LW_INI_KEY, INIK_lineManage);
        if (lib_strcmp(pinikey->INIK_pcKey, pcKey) == 0) {
            pcRet = (PCHAR)pinikey->INIK_pcValue;
            break;
        }
    }
    
    return  ((CPCHAR)pcRet);
}
/*********************************************************************************************************
** ��������: if_param_load
** ��������: װ��ָ������ӿ�����
** �䡡��  : name          ��������
** �䡡��  : ���þ��
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
void  *if_param_load (const char *name)
{
    PLW_INI_SEC  pinisec;

    if (!name) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }

    pinisec = __iniLoad(LW_IFPARAM_PATH, name);
    if (!pinisec) {
        fprintf(stderr, "[ifparam]No network parameter for [%s] from %s"
                        ", default parameters will be used.\n", name, LW_IFPARAM_PATH);
        return  (LW_NULL);
    }

    return  ((void *)pinisec);
}
/*********************************************************************************************************
** ��������: if_param_unload
** ��������: ж��ָ������ӿ�����
** �䡡��  : pifparam      ���þ��
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
void  if_param_unload (void *pifparam)
{
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec) {
        return;
    }

    __iniUnload(pinisec);
}
/*********************************************************************************************************
** ��������: if_param_getenable
** ��������: ��ȡ�����Ƿ�ʹ������. (���δ�ҵ�����Ĭ��Ϊʹ��)
** �䡡��  : pifparam      ���þ��
**           enable        �Ƿ�ʹ��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getenable (void *pifparam, int *enable)
{
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !enable) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    *enable = __iniGetInt(pinisec, LW_IFPARAM_ENABLE, 1);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_getdefault
** ��������: ��ȡ�����Ƿ�ΪĬ��·������. (���δ�ҵ�����Ĭ��Ϊʹ��)
** �䡡��  : pifparam      ���þ��
**           def           �Ƿ�ΪĬ��·��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getdefault (void *pifparam, int *def)
{
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !def) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    *def = __iniGetInt(pinisec, LW_IFPARAM_DEFAULT, 1);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_getdhcp
** ��������: ��ȡ�����Ƿ�Ϊ dhcp (���δ�ҵ�����Ĭ��Ϊ�� DHCP)
** �䡡��  : pifparam      ���þ��
**           def           �Ƿ�ΪĬ��·��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getdhcp (void *pifparam, int *dhcp)
{
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !dhcp) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    *dhcp = __iniGetInt(pinisec, LW_IFPARAM_DHCP, 0);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_ipv6autocfg
** ��������: ��ȡ�����Ƿ�ʹ�� IPv6 ��ַ�Զ����� (���δ�ҵ�����Ĭ��������ʼ����������)
** �䡡��  : pifparam      ���þ��
**           def           �Ƿ�ΪĬ��·��
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
** ע  ��  : IPv6 ·������Ӧʹ�ܴ�ѡ��
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_ipv6autocfg (void *pifparam, int *autocfg)
{
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;
    INT          iRet;

    if (!pinisec || !autocfg) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    iRet = __iniGetInt(pinisec, LW_IFPARAM_IPV6_ACFG, PX_ERROR);
    if (iRet < 0) {
        return  (PX_ERROR);
    }
    
    *autocfg = iRet;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_getipaddr
** ��������: ��ȡ IP ��ַ����.
** �䡡��  : pifparam      ���þ��
**           ipaddr        IP ��ַ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getipaddr (void *pifparam, ip4_addr_t *ipaddr)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !ipaddr) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    value = __iniGetStr(pinisec, LW_IFPARAM_IPADDR, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (ip4addr_aton(value, ipaddr)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getipaddr
** ��������: ��ȡ IP ��ַ����.
** �䡡��  : pifparam      ���þ��
**           inaddr        IP ��ַ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getinaddr (void *pifparam, struct in_addr *inaddr)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !inaddr) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    value = __iniGetStr(pinisec, LW_IFPARAM_IPADDR, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (inet_aton(value, inaddr)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getnetmask
** ��������: ��ȡ������������.
** �䡡��  : pifparam      ���þ��
**           mask          ��������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getnetmask (void *pifparam, ip4_addr_t *mask)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !mask) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    value = __iniGetStr(pinisec, LW_IFPARAM_MASK, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (ip4addr_aton(value, mask)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getinnetmask
** ��������: ��ȡ������������.
** �䡡��  : pifparam      ���þ��
**           mask          ��������
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getinnetmask (void *pifparam, struct in_addr *mask)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !mask) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    value = __iniGetStr(pinisec, LW_IFPARAM_MASK, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (inet_aton(value, mask)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getgw
** ��������: ��ȡ������������.
** �䡡��  : pifparam      ���þ��
**           gw            ���ص�ַ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getgw (void *pifparam, ip4_addr_t *gw)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !gw) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    value = __iniGetStr(pinisec, LW_IFPARAM_GW, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (ip4addr_aton(value, gw)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getingw
** ��������: ��ȡ������������.
** �䡡��  : pifparam      ���þ��
**           gw            ���ص�ַ
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getingw (void *pifparam, struct in_addr *gw)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !gw) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    value = __iniGetStr(pinisec, LW_IFPARAM_GW, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    if (inet_aton(value, gw)) {
        return  (ERROR_NONE);

    } else {
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: if_param_getmac
** ��������: ��ȡ MAC ����.
** �䡡��  : pifparam      ���þ��
**           mac           MAC ��ַ�ַ���
**           sz            ��������С
** �䡡��  : ERROR or OK
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
int  if_param_getmac (void *pifparam, char *mac, size_t  sz)
{
    const char  *value;
    PLW_INI_SEC  pinisec = (PLW_INI_SEC)pifparam;

    if (!pinisec || !mac) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    value = __iniGetStr(pinisec, LW_IFPARAM_MAC, LW_NULL);
    if (!value) {
        return  (PX_ERROR);
    }

    lib_strlcpy(mac, value, sz);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: if_param_syncdns
** ��������: ͬ�� DNS ����.
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���:
** ����ģ��:
                                           API ����
*********************************************************************************************************/
LW_API
void  if_param_syncdns (void)
{
#define MATCH(line, name) \
    (!lib_strncmp(line, name, sizeof(name) - 1) && \
    (line[sizeof(name) - 1] == ' ' || \
     line[sizeof(name) - 1] == '\t'))

    FILE  *fp = fopen(LW_RESCONF_PATH, "r");
    char   buf[128];
    char  *cp;
    u8_t   numdns = 0;

    if (!fp) {
        return;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        if (*buf == ';' || *buf == '#') {
            continue;
        }
        if (MATCH(buf, "nameserver")) {
            cp = buf + sizeof("nameserver") - 1;
            while (*cp == ' ' || *cp == '\t'){
                cp++;
            }

            cp[lib_strcspn(cp, ";# \t\n")] = '\0';
            if ((*cp != '\0') && (*cp != '\n')) {
                ip_addr_t   addr;
                if (ipaddr_aton(cp, &addr)) {
                    if (numdns < DNS_MAX_SERVERS) {
                        dns_setserver(numdns, &addr);
                        numdns++;
                    }
                }
            }
        }
    }

    fclose(fp);
}

#endif                                                                  /*  LW_CFG_NET_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/