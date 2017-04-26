/******************************************************************************
| includes
|--------------------------------------------------------------------------------------------*/
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

/******************************************************************************
| defines
|--------------------------------------------------------------------------------------------*/
#define DABperiod				7500  // 20KHz对应时钟数，TBCLK = SYSCLKOUT
//#define Flegperiod				30000  // 5KHz对应时钟数，TBCLK = SYSCLKOUT
#define halfperiod				3750  // 20KHz对应时钟数，TBCLK = SYSCLKOUT
#define prediv				0  // 预分频
#define DeadZone				150   // 死区

/******************************************************************************
| variables
|--------------------------------------------------------------------------------------------*/
extern Uint16 PhaSft;

/******************************************************************************
| local functions prototypes
|--------------------------------------------------------------------------------------------*/
void InitPWM(void);
