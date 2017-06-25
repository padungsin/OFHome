//Home Control Project Header

#use fast_io(A)
#use fast_io(B)
#use fast_io(C)
#use fast_io(D)
#use fast_io(E)

#BYTE PA=5
#BYTE PB=6
#BYTE PC=7
#BYTE PD=8
#BYTE PE=9



#define Alert1			PIN_A1
#define Alert2			PIN_A2
#define Alert3 			PIN_A3

#define SIREN   		PIN_A4			//Output

#define WIFI_RESET   	PIN_E0
#define ESP8266_RESET   PIN_E1
#define ESP8266_Connect PIN_E2			//Input

#define OP1			PIN_D0
#define OP2			PIN_D1
#define OP3			PIN_D2
#define OP4			PIN_C3
#define OP5			PIN_C4
#define OP6			PIN_C5
#define OP7			PIN_D4
#define OP8			PIN_D5

#define COMMAND_SIZE 500
#define INTERNAL 'I'
#define WIRELESS 'W'
#define LIVOLO 'L'

#define SWITCH_ON 'N'
#define SWITCH_OFF 'F'
#define SWITCH_TOGGLE 'T'


int internalSwitch[6] = {PIN_D0, PIN_D1, PIN_D2, PIN_C3,PIN_C4, PIN_C5};
int internalSwitchStatus[10] = {0,0,0,0,0,0,0,0,0,0};
int wirelessSwitchStatus[10] = {0,0,0,0,0,0,0,0,0,0};
int livoloSwitchStatus[10] = {0,0,0,0,0,0,0,0,0,0};

int saveInternalSwitchStatusAddress[10] = {21,22,23,24,25,26,27,28,29,30};
int saveWirelessSwitchStatusAddress[10] = {31,32,33,34,35,36,37,38,39,40};
int saveLivoloSwitchStatusAddress[10] = {41,42,43,44,45,46,47,48,49,50};








int16 lastTrigDate[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int8 waitingSlaveCounter[10] = {0,0,0,0,0,0,0,0,0,0};
char* uuids[20] ;
//BYTE SW_flag[NO_OF_SW] = {0,0,0,0,0,0,0,0};

int i;
int32 connection_cnt = 0;

int16 Request_Update_Info = 0;
int1 MQTTConnect_Flag = true;
int1 UpdateRTC_flag = true;

//Variable Temp_DS1820
//====================
int temp;
unsigned long raw_temp;
char temp_Display[2];
float temperature; 

float voltage = 0;

int1 Boot_Flag = false;
int1 WaitingWireless_Flag = false;
int1 Siren_Flag = false;
int Siren_cnt = 0;

long Alert1_Cnt = 0;
long Alert2_Cnt = 0;
long Alert3_Cnt = 0;

int Alert1_Flag = false;
int Alert2_Flag = false;
int Alert3_Flag = false;
int1 HomeLock_Flag = false;
int1 DoAlert_Flag = false;



int timerEnableEEPROM[20] = {501,502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,520};
int timerSwitchNumberEEPROM[20] = {521,522,523,524,525,526,527,528,529,530,531,532,533,534,535,536,537,538,539,540};
int timerDeviceTypeEEPROM[20] = {541,542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,557,558,559,560};
int timerControlModeEEPROM[20] = {561,562,563,564,565,566,567,568,569,570,571,572,573,574,575,576,577,578,579,580};
int timerHourEEPROM[20] = {581,582,583,584,585,586,587,588,589,590,591,592,593,594,595,596,597,598,599,600};
int timerMinuteEEPROM[20] = {601,602,603,604,605,606,607,608,609,610,611,612,613,614,615,616,617,618,619,620};
int timerRecurrentEEPROM[20] = {621,622,623,624,625,626,627,628,629,630,631,632,633,634,635,636,637,638,639,640};
//{641,642,643,644,645,646,647,648,649,650,651,652,653,654,655,656,657,658,659,660};
//{661,662,663,664,665,666,667,668,669,670,671,672,673,674,675,676,677,678,679,680};
//{681,682,683,684,685,686,687,688,689,690,691,692,693,694,695,696,697,698,699,700};
//{701,702,703,704,705,706,707,708,709,710,711,712,713,714,715,716,717,718,719,720};


char activeTimerSwitch[22] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};



char day_Display[2];
char mth_Display[2];
char year_Display[2];

char hour_Display[2];
char min_Display[2];
char sec_Display[2];

int SETSW_Secure[10];

#define FWVERSIONADDR1 1
#define FWVERSIONADDR2 2
#define FWVERSIONADDR3 3
#define FWVERSIONADDR4 4
#define FWVERSIONADDR5 5

//===================================
//Reserve Address for IR Save 61 - 84
//===================================

#define HomeSecureAddress	95
//===================================
//Reserve Address for SETSWSC 100 - 110
//===================================
#define SETSWSCAddress 100

#define TempHighAddress		115

#define VLOWAddress			116
#define VLOWPointAddress	117

#define VHIGHAddress		118
#define VHIGHPointAddress	119

#define UPGRADE_ADDRESS		255