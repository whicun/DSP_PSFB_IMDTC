/******************************************************************************
| includes                          
|----------------------------------------------------------------------------*/
#include "imcontrol.h"

/******************************************************************************
| local variable definitions                          
|----------------------------------------------------------------------------*/
double angle = 0;  // ������������н�
int period_count = 0;  // PWM�жϴ���
Uint16 sector = 0;  // SVM����

/******************************************************************************
| global variable definitions                          
|----------------------------------------------------------------------------*/
/* �۲�ֵ */
  // ��ѹ
double Udc_IN = 0;
PHASE_ABC uabc = {0, 0, 0};
PHASE_ALBE ualbe = {0, 0};
PHASE_DQ udq = {0, 0};
  // ����
PHASE_ABC iabc = {0, 0, 0};
PHASE_ALBE ialbe = {0, 0};
PHASE_DQ idq = {0, 0};
  // ����
double lambdar = 0;
PHASE_ALBE lambdaralbe = {0, 0};
double lambdas = 0;
PHASE_ALBE lambdasalbe = {0, 0};
double theta = 0;
  // ת��
double Te = 0;
  // ת��
double speed = 0;
double wr = 0;

/* ����ֵ */
  // ��ѹ
double u_cmd = 0;  // �ߵ�ѹ��ֵ
PHASE_ALBE ualbe_cmd = {0, 0};
PHASE_DQ udq_cmd = {0, 0};
  // ����
PHASE_DQ idq_cmd = {0, 0};
  // ����
double lambdas_cmd = 0.6;
  // ת��
double Te_cmd = 0;
  // ռ�ձ�
PHASE_ABC Dabc = {0, 0, 0};
  // ת��
double spd_cmd = 0;  // ת�ٸ���
double spd_req = 500;  // ת���趨

/* PI ���� */
double Kp_Tecmd = 0.02;
double Ki_Tecmd = 1;
double intgrt_Tecmd = 0;

/*==============================================================================
=========================== Coordinate Transform ===============================
==============================================================================*/

/******************************************************************************
@brief   3s/2r ����任

@param   abc -- ��������
         dq -- ������ת������*ָ�봫��*��
         theta -- �ϳ������Ƕ�

@return  N/A
******************************************************************************/
void S3toR2(PHASE_ABC abc, PHASE_DQ *dq, double theta)
{
  // sqrt(2) = 1.414213562, 1.0/6.0*pi = 0.52359878
  dq->d = 1.414213562 * (cos(theta - 0.52359878) * abc.a + sin(theta) * abc.b);
  dq->q = -1.414213562 * (sin(theta - 0.52359878) * abc.a - cos(theta) * abc.b);
}

/******************************************************************************
@brief   3s/2s ����任

@param   abc -- ��������
         albe -- ���ྲֹ������*ָ�봫��*��

@return  N/A
******************************************************************************/
void S3toS2(PHASE_ABC abc, PHASE_ALBE *albe)
{
  // sqrt(2.0/3.0) = 0.816496580927726, 1.0/sqrt(2) = 0.7071067812, sqrt(2) = 1.414213562, sqrt(3)/2 = 0.866
//  albe->al = 1.22474487 * abc.a;
//  albe->be = 0.7071067812 * abc.a + 1.414213562 * abc.b;
	albe->al = 0.816496581 * (abc.a - 0.5 * abc.b - 0.5 * abc.c);
	albe->be = 0.7071067812 * (abc.b - abc.c);
}

/******************************************************************************
@brief   2s/2r ����任

@param   albe -- ���ྲֹ����
         dq -- ������ת������*ָ�봫��*��
         theta -- �ϳ������Ƕ�

@return  N/A
******************************************************************************/
void S2toR2(PHASE_ALBE albe, PHASE_DQ *dq, double cosIn, double sinIn)
{
  dq->d = cosIn * albe.al + sinIn * albe.be;
  dq->q = -sinIn * albe.al + cosIn * albe.be;
}

/******************************************************************************
@brief   2r/3s ����任

@param   dq -- ������ת����
         abc -- ����������*ָ�봫��*��
         theta -- �ϳ������Ƕ�

@return  N/A
******************************************************************************/
void R2toS3(PHASE_DQ dq, PHASE_ABC *abc, double theta)
{
  // sqrt(2.0/3.0) = 0.81649658, 2.0/3.0*pi = 2.094395102
  abc->a = 0.81649658 * (cos(theta) * dq.d - sin(theta) *dq.q);
  abc->b = 0.81649658 * (cos(theta - 2.094395102) * dq.d - sin(theta - 2.094395102) *dq.q);
  abc->c = 0.81649658 * (cos(theta + 2.094395102) * dq.d - sin(theta + 2.094395102) *dq.q);
}

/******************************************************************************
@brief   2s/3s ����任

@param   albe -- ���ྲֹ����
         abc -- ����������*ָ�봫��*��

@return  N/A
******************************************************************************/
void S2toS3(PHASE_ALBE albe, PHASE_ABC *abc)
{
  // sqrt(2.0/3.0) = 0.81649658, sqrt(3)/2.0 = 0.8660254
  abc->a = 0.81649658 * albe.al;
  abc->b = -0.40824829 * albe.al + 0.70710678 * albe.be; // abc->b = sqrt(2.0/3.0) * (-0.5 * albe.al + sqrt(3)/2.0 * albe.be)
  abc->c = -0.40824829 * albe.al - 0.70710678 * albe.be; // sqrt(2.0/3.0) * (-0.5 * albe.al - sqrt(3)/2.0 * albe.be);
}

/******************************************************************************
@brief   2r/2s ����任

@param   dq -- ������ת����
         albe -- ���ྲֹ������*ָ�봫��*��
         theta -- �ϳ������Ƕ�

@return  N/A
******************************************************************************/
void R2toS2(PHASE_DQ dq, PHASE_ALBE *albe, double cosIn, double sinIn)
{
  albe->al = cosIn * dq.d - sinIn * dq.q;
  albe->be = sinIn * dq.d + cosIn * dq.q;
}
/******************************************************************************
@brief   Rotor Flux Calculation
******************************************************************************/
/* calculate lamdar */  
double lambdar_cal(double lambdar, double ism)
{
  return (1.0 - Ts/Tr) * lambdar + Lm*Ts/Tr * ism;
}

void lambdaralbe_cal(PHASE_ALBE ialbe, PHASE_ALBE *lambdaralbe, double wr)
{
	// 3.531654573104190e-04 = Ts / Tr
	double tempal = 0, tempbe = 0;
	tempal = Lm * (ialbe.al) - wr * Tr * (lambdaralbe->be);
	tempbe = Lm * (ialbe.be) + wr * Tr * (lambdaralbe->al);
	lambdaralbe->al = (lambdaralbe->al + 3.531654573104190e-04 * tempal) / (1 + 3.531654573104190e-04);
	lambdaralbe->be = (lambdaralbe->be + 3.531654573104190e-04 * tempbe) / (1 + 3.531654573104190e-04);
}

/******************************************************************************
@brief   Stator Flux Calculation
******************************************************************************/
void lambdasalbe_voltage(PHASE_ALBE ualbe, PHASE_ALBE ialbe, PHASE_ALBE *lambdasalbe)
{
	lambdasalbe->al = LPfilter2(ualbe.al - Rs * ialbe.al, lambdasalbe->al, 1, Ts);
	lambdasalbe->be = LPfilter2(ualbe.be - Rs * ialbe.be, lambdasalbe->be, 1, Ts);
	//lambdasalbe->al = Integrator(ualbe.al - Rs * ialbe.al, lambdasalbe->al, Ts);
	//lambdasalbe->be = Integrator(ualbe.be - Rs * ialbe.be, lambdasalbe->be, Ts);
}
void lambdasalbe_current(PHASE_ALBE lambdaralbe, PHASE_ALBE *lambdasalbe, PHASE_ALBE ialbe)
{
	// 0.033926908804678 = (Ls - Lm*Lm/Lr), 0.961092994490048 = Lm / Lr;
	lambdasalbe->al = 0.033926908804678 * ialbe.al + 0.961092994490048 * lambdaralbe.al;
	lambdasalbe->be = 0.033926908804678 * ialbe.be + 0.961092994490048 * lambdaralbe.be;
}

/******************************************************************************
@brief   PI Module 
******************************************************************************/
double PImodule(double Kp, double Ki, double err, double *intgrt, double Uplim, double Downlim, double _Ts)
{
	double output = 0;

	*intgrt += Ki * _Ts * err;
	if (*intgrt >= Uplim)
	{
		*intgrt = Uplim;
		return Uplim;
	}
	else if (*intgrt <= Downlim)
	{
		*intgrt = Downlim;
		return Downlim;
	}
	else
	{
		output = Kp * err + *intgrt;

		if (output >= Downlim && output <= Uplim)
			return output;
		else if (output > Uplim)
			return Uplim;
		else
			return Downlim;
	}
}

double Integrator(double paramin, double sum, double ts)
{
  return paramin * ts + sum;
}

double LPfilter1(double paramin, double lasty, double wc, double ts)
{
  return (lasty + ts * wc * paramin) / (1 + ts * wc);
  //return (lasty + ts * paramin) / (1 + ts * wc);
}

double LPfilter2(double paramin, double lasty, double wc, double ts)
{
  //return (lasty + ts * wc * paramin) / (1 + ts * wc);
  return (lasty + ts * paramin) / (1 + ts * wc);
}

/******************************************************************************
@brief   Relay
******************************************************************************/
int Relay_2Level(double input, double Uplim, double Downlim, int lastout)
{
	if(input > Uplim)
		return 1;
	else if(input < Downlim)
		return -1;
	else
		return lastout;
}
int Relay_3Level(double input, double Uplim, double Downlim, int lastout)
{
	if(input > Uplim)
		return 1;
	else if (input < Downlim)
		return -1;
	else if (input > 0 && lastout < 0)
		return 0;
	else if (input < 0 && lastout > 0)
		return 0;
	else
		return lastout;
}

/******************************************************************************
@brief   SVM 
******************************************************************************/ 
int sector1(double alpha, double beta)
{
	double k = beta / alpha;
	if(beta >= 0 && (  k < 1.7320508 && k >= 0))
		return 1;
	else if(beta > 0 && (k >= 1.7320508 || k < -1.7320508))
		return 2;
	else if(beta > 0 && (k >= -1.7320508 && k < 0))
		return 3;
	else if(beta <= 0 && (k >= 0 && k < 1.7320508))
		return 4;
	else if(beta < 0 && (k >= 1.7320508 || k < -1.7320508))
		return 5;
	else if(beta < 0 && (k < 0 && k >= -1.7320508))
		return 6;
	else
		return 1;
}

int sector2(double alpha, double beta)
{
	// sqrt(3) = 1.7320508
	double k = alpha / beta;
	if(alpha > 0 && (k <= -1.7320508 || k > 1.7320508))
	    return 1;
	else if(alpha > 0 && (k > 0 && k <= 1.7320508))
		return 2;
	else if(alpha <= 0 && (k > -1.7320508 && k <= 0))
		return 3;
	else if(alpha < 0 && (k > 1.7320508 || k <= -1.7320508))
		return 4;
	else if(alpha < 0 && (k > 0 && k <= 1.7320508))
		return 5;
	else if(alpha >= 0 && (k > -1.7320508 && k <= 0))
		return 6;
	else
		return 1;
}

/******************************************************************************
@brief   SVM
******************************************************************************/
void ualbeSVM(double Ual, double Ube, double Ud, PHASE_ABC *Dabc)
{
  double dm, dn, d0;
  double k = Ube / Ual;
  double reciUd = 1.0 / Ud;
  
  /* �����жϼ�ռ�ձȼ��� */
  // sqrt(3) = 1.7320508, sqrt(3) / 2.0 = 0.8660254044
  // 1 / sqrt(3) = 0.57735027
  if (Ual > 0 && Ube >= 0 && k >= 0 && k < 1.7320508)
  {
    sector = 1;
    dm = 0.8660254044 * (Ual - Ube * 0.57735027) * reciUd;
    dn = Ube * reciUd;
  }
  else if (Ube > 0 && (k >= 1.7320508 || k < -1.7320508))
  {
    sector = 2;
    dm = 0.8660254044 * (Ual + Ube * 0.57735027) * reciUd;
    dn = 0.8660254044 * (-Ual + Ube * 0.57735027) * reciUd;
  }
  else if (Ual < 0 && Ube > 0 && k >= -1.7320508 && k < 0)
  {
    sector = 3;
    dm = Ube * reciUd;
    dn = 0.8660254044 * (-Ual - Ube * 0.57735027) * reciUd;
  }
  else if (Ual < 0 && Ube <= 0 && k >= 0 && k < 1.7320508)
  {
    sector = 4;
    dm = 0.8660254044 * (-Ual + Ube * 0.57735027) * reciUd;
    dn = -Ube * reciUd;
  }
  else if (Ube < 0 && (k >= 1.7320508 || k < -1.7320508))
  {
    sector = 5;
    dm = 0.8660254044 * (-Ual - Ube * 0.57735027) * reciUd;
    dn = 0.8660254044 * (Ual - Ube * 0.57735027) * reciUd;
  }
  else if (Ual > 0 && Ube < 0 && k >= -1.7320508 && k < 0)
  {
    sector = 6;
    dm = -Ube * reciUd;
    dn = 0.8660254044 * (Ual + Ube * 0.57735027) * reciUd;
  }
  else
  {
    sector = 1;
    dm = 0;
    dn = 0;
  }

  if (dm + dn >= 1)
  {
    double temp = dm / (dm + dn);
    dn = dn / (dm + dn);
    dm = temp;
    d0 = 0;
  }
  else
    d0 = 0.5 * (1 - dm - dn);
  
  switch (sector)
  {
  case 1:
    {
      Dabc->a = dm + dn + d0;
      Dabc->b = dn + d0;
      Dabc->c = d0;
      break;
    }
  case 2:
    {
		Dabc->a = dm + d0;
		Dabc->b = dm + dn + d0;
		Dabc->c = d0;
      break;
    }
  case 3:
    {
		Dabc->a = d0;
		Dabc->b = dm + dn + d0;
		Dabc->c = dn + d0;
      break;
    }
  case 4:
    {
		Dabc->a = d0;
		Dabc->b = dm + d0;
		Dabc->c = dm + dn + d0;
      break;
    }
  case 5:
    {
		Dabc->a = dn + d0;
		Dabc->b = d0;
		Dabc->c = dm + dn + d0;
      break;
    }
  case 6:
    {
		Dabc->a = dm + dn + d0;
		Dabc->b = d0;
		Dabc->c = dm + d0;
      break;
    }
  default:
    {
		Dabc->a = 1.1;
		Dabc->b = 1.1;
		Dabc->c = 1.1;
    }
  }
}

/******************************************************************************
@brief   RAMP -- ����ʽб�º���

@param   ramp -- б��
         initial -- Ӧ������ʼֵ
         increment -- �Ա�������
         Hlimit -- ����
         Llimit -- ����

@return  Ӧ������ֵ
******************************************************************************/
double RAMP(double ramp, double initial, double increment, double Hlimit, double Llimit)
{
  double temp = ramp * increment + initial;
  if (temp > Hlimit)
    return Hlimit;
  else if (temp < Llimit)
    return Llimit;
  else
    return temp;
}

/******************************************************************************
@brief   roundn -- ������ȡָ��λ��

@param   input -- ����
         digit -- ����С�����λ��

@return  ����ָ��λ�����ֵ
******************************************************************************/
double roundn(double input, int _digit)
{
  double temp;
  temp = input * _digit;
  temp = floor(temp);
  temp = temp / _digit;
  return temp;
}
