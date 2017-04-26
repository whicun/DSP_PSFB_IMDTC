/******************************************************************************
| includes
|-------------------------------------------------------------------------------------------*/
#include "ysQEP.h"

/******************************************************************************
| variables
|--------------------------------------------------------------------------------------------*/

/******************************************************************************
| functions
|-------------------------------------------------------------------------------------------*/
void InitQEP1(void)
{
	/**********************QDU1�������뵥Ԫ********************/
	EQep1Regs.QDECCTL.bit.QSRC = 0;	//00=�������� 01=ֱ�Ӽ��� 10=���ϼ��� 11=���¼���
	EQep1Regs.QDECCTL.bit.SOEN = 0;  // ��ֹͬ���Ƚ����
	//EQep1Regs.QDECCTL.bit.SPSEL = 0;  // index�������������Ĭ�ϣ�
	EQep1Regs.QDECCTL.bit.XCR = 0;  // ���±��ؼ���
	EQep1Regs.QDECCTL.bit.SWAP = 0;  // ����ʱ�ӽ�����ֹ
	EQep1Regs.QDECCTL.bit.IGATE = 0;  //0=��ֹ�����¼� 1=���������¼�
	EQep1Regs.QDECCTL.bit.QAP = 0;	//0=ֱ������ 1=��������
	EQep1Regs.QDECCTL.bit.QBP = 0;	//0=ֱ������ 1=��������
	EQep1Regs.QDECCTL.bit.QIP = 0;	//0=ֱ������ 1=��������
	EQep1Regs.QDECCTL.bit.QSP = 0;	//0=ֱ������ 1=��������

	//******************PCCU1λ�ü�����&�������Ƶ�Ԫ************////////////
	EQep1Regs.QEPCTL.bit.FREE_SOFT = 0;  //00=����ֹͣ 01=��������  1x=����Ӱ��
	EQep1Regs.QEPCTL.bit.PCRM = 1;  //00=�����¼� 01=���λ�� 10=�״����� 11=��λʱ���¼�
	EQep1Regs.QEPCTL.bit.SEI = 0;  // �����¼�����ʼ��λ�ü�����
	EQep1Regs.QEPCTL.bit.IEI = 0;  //00,01=������ 10=QEPI1�����ؼ�������ʼ��ΪQPOSINIT 11=QEPI1�½��ؼ�������ʼ��ΪQPOSINIT
	//EQep1Regs.QEPCTL.bit.SWI = 0;  // �����ʼ��λ�ü���
	EQep1Regs.QEPCTL.bit.IEL= 00;		//00==���� 01=QEPI1�����ؼ�����ֵ���浽QPOSILAT 10=QEPI1�����ؼ�����ֵ���浽QPOSILAT 11=����������
	EQep1Regs.QEPCTL.bit.QPEN= 1;	//0=�����λQEP 1=ʹ��
	EQep1Regs.QEPCTL.bit.QCLM = 0;  // CPU��������������
	EQep1Regs.QEPCTL.bit.UTE = 0;  // ��ֹ��λ��ʱ��
	EQep1Regs.QEPCTL.bit.WDE = 0;  // ��ֹ���Ź�

	EQep1Regs.QPOSINIT=0;  //��ʼ��λ�ü�����
	EQep1Regs.QPOSMAX=65000;  // ������
	EQep1Regs.QEPSTS.bit.QDF= 0;
	EQep1Regs.QCAPCTL.bit.CEN = 1;  // ʹ��QEP��׽��Ԫ
}
