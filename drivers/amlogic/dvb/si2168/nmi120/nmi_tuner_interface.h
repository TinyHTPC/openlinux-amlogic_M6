
#ifndef NMI_TUNER_INTERFACE_H
#define NMI_TUNER_INTERFACE_H
#include "nmiioctl.h"
#include "nmicmn.h"

 
typedef struct {
	tNMI_ALLTVSTD tvstd;   //??��?
	tNmiOutput output;       //��?3?����D����?CVBS?��???D?�̨�?3?
	tNmiDacSel dacsel;       //DAC?�꨺?
	uint32_t freq;             //RF?��?��
	uint32_t if_freq;          //?D?��?��?��
	uint8_t freq_invert;       //RF ��?��?����?��
	uint8_t if_freq_invert;    //?D?�̨�?��?����?��
	uint32_t aif;               //ADUIO?D?��?��?��
	uint8_t is_stereo;         //��?��?���騬?����
	uint8_t ucBw;             //��??��
	bool_t  bretune;           //��?��?D����a??D????��
	tNmiSwrfliArg poll_param;
	uint8_t scan_aci[3];  //6mhz  7mhz  8mhz 
	uint8_t play_aci[3];  //6mhz  7mhz  8mhz 
}tNMI_TUNE_PARAM;

//���ô˺�����ʵ��NMI TUNER�ĳ�ʼ��������ֻ�ڿ�ʼʱִ��
extern uint32_t Nmi_Tuner_Interface_init_chip(tTnrInit* pcfg);

//����Ƶ�㣬�Լ����е���ʽ
extern uint8_t Nmi_Tuner_Interface_Tuning(tNMI_TUNE_PARAM* param);

//��ȡTUNER��AGC �Ƿ�LOCK
extern int Nmi_Tuner_Interface_GetLockStatus(void);

//SLEEPģʽ
extern void Nmi_Tuner_Interface_Sleep_Lt(void);

//wake up оƬ
extern void Nmi_Tuner_Interface_Wake_Up_Lt(void);

//��ȡCHIPID
extern uint32_t Nmi_Tuner_Interface_Get_ChipID(void);

//���üĴ���
extern void  Nmi_Tuner_Interface_Wreg(uint32_t addr,uint32_t value);

extern uint32_t  Nmi_Tuner_Interface_Rreg(uint32_t addr);

//��ȡRSSI
extern  int16_t Nmi_Tuner_Interface_GetRSSI(void);

//DEMOD֪ͨ TUNER  Ƶ���Ƿ�������DEMOD����Ƶ��󣬵��ô˺���
extern void Nmi_Tuner_Interface_Demod_Lock(void);

extern uint8_t Nmi_Tuner_Interface_CallBack(void);

#endif



   

