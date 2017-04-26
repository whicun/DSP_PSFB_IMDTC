/******************************************************************************
| includes
|----------------------------------------------------------------------------*/
#include "math.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "ysPWM.h"

/******************************************************************************
| defines
|----------------------------------------------------------------------------*/
/* IM parameters */
#define Rs 4.6  //4.4717
#define Ls 0.289368
#define Rr 3.3278
#define Lr 0.289368
#define Lm 0.27325
#define Tr 0.0869557
#define np 2

/* control period */
#define Ts 5e-5

/* lamdar��Сֵ���� */
#define lamdarlimit_L 0.01

/* PI parameters */
  // �ٶȱջ�
//#define Kp_Te 2
//#define Ki_Te 0
#define Tecmdlimit_H 5
#define Tecmdlimit_L -5

/* speed ramp */
#define spdramp 100  // б��
#define spdlimit_H 1000  // ת������
#define spdlimit_L 0  // ת������

/* V/spd curve */
//#define VSpdramp 0.38386  // б��
#define VSpdramp 0.17  // б��
#define Voltlimit_H 100  // ��ѹ����
#define Voltlimit_L 10  // ��ѹ����

/* DTC */
#define band_lambdas 0.02
#define band_Te 0.05


//#define VSpdramp 0.38386  // б��
#define VSpdramp 0.17  // б��
#define Voltlimit_H 100  // ��ѹ����
#define Voltlimit_L 10  // ��ѹ����

/* auxiliary */
#define pi 3.1415926
#define Z 1024  // �����������
#define digit 1e6  // roundn����

/******************************************************************************
| types
|----------------------------------------------------------------------------*/
typedef struct
{
  double a, b, c;
} PHASE_ABC;

typedef struct
{
  double al, be;
} PHASE_ALBE;

typedef struct
{
  double d,q;
} PHASE_DQ;

/******************************************************************************
| global variables
|----------------------------------------------------------------------------*/
/* �۲�ֵ */
  // ��ѹ
extern double Ud;
extern PHASE_ABC uabc;
extern PHASE_ALBE ualbe;
extern PHASE_DQ udq;
  // ����
extern PHASE_ABC iabc;
extern PHASE_ALBE ialbe;
extern PHASE_DQ idq;
  // ����
extern double lambdar;
extern double lambdas;  // lambdas^2
extern PHASE_ALBE lambdasalbe;
extern double theta;
  // ת��
extern double Te;
  // ת��
extern double speed;

/* ����ֵ */
  // ��ѹ
extern double u_cmd;
extern PHASE_ALBE ualbe_cmd;
extern PHASE_DQ udq_cmd;
  // ����
extern PHASE_DQ idq_cmd;
  // ����
extern double lambdas_cmd;
  // ת��
extern double Te_cmd;
  // ռ�ձ�
extern PHASE_ABC Dabc;
  // ת��
extern double spd_cmd;  // ת�ٸ���
extern double spd_req;  // ת���趨

/* PI ���� */
extern double Kp_Tecmd;
extern double Ki_Tecmd;
extern double intgrt_Tecmd;

/******************************************************************************
| local functions prototypes
|----------------------------------------------------------------------------*/
double roundn(double input, int _digit);

/******************************************************************************
| exported functions
|----------------------------------------------------------------------------*/
/* Forward conversion */  
extern void S3toR2(PHASE_ABC abc, PHASE_DQ *dq, double theta);
extern void S3toS2(PHASE_ABC abc, PHASE_ALBE *albe);
extern void S2toR2(PHASE_ALBE albe, PHASE_DQ *dq, double cosIn, double sinIn);

/* Backward conversion */  
extern void R2toS3(PHASE_DQ dq, PHASE_ABC *abc, double theta);
extern void S2toS3(PHASE_ALBE albe, PHASE_ABC *abc);
extern void R2toS2(PHASE_DQ dq, PHASE_ALBE *albe, double cosIn, double sinIn);

/* calculate lamdar */  
extern double lambdarCal(double lambdar, double ism);

/* calculate lamdar */
extern void lambdasalbeCal(PHASE_ALBE ualbe, PHASE_ALBE ialbe, PHASE_ALBE *lambdasalbe);

/* calculate position and speed */  
extern double wrCal_M();
extern double positonCal(double wr, double lambdar, double ist, double theta);

/* PI module */  
extern double PImodule(double Kp, double Ki, double err, double *intgrt, double Uplim, double Downlim, double _Ts);
extern double Integrator(double paramin, double sum, double ts);
extern double LPfilter1(double paramin, double lasty, double wc, double ts);
extern double LPfilter2(double paramin, double lasty, double wc, double ts);

/* Relay */
extern int Relay_2Level(double input, double Uplim, double Downlim, int lastout);
extern int Relay_3Level(double input, double Uplim, double Downlim, int lastout);

/* Sector */
extern int sector1(double alpha, double beta);
extern int sector2(double alpha, double beta);

/* SVM */  
extern void positionSVM();
extern void ualbeSVM(double Ual, double Ube, double Ud, PHASE_ABC *Dabc);

/* Auxiliary Function */
extern double RAMP(double ramp, double initial, double increment, double Hlimit, double Llimit);
extern double roundn(double input, int _digit);
