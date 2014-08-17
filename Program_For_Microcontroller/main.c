/*
 * ������: Web_turel_came ������� � �������������� hid-custom-rq(���� �: microsin.ru) � Hid_example_firmware(���� �: http://we.easyelectronics.ru/electro-and-pc/usb-dlya-avr-chast-2-hid-class-na-v-usb.html)
 * �����: Murk
 * ���� ��������: 2013-11-15
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * ��������: GNU GPL v2 (��. License.txt) ��� ������������� (CommercialLicense.txt) 
 * ��� ���������� ���������� ����� ����� //"����������".���������� ������� �������� �������� ����������� ���������� ��� // "����������" ��� /* "����������" 
 */


#include <avr/io.h>
#include <avr/wdt.h>         //���������� ��� ������ �� ���������� ��������
#include <avr/eeprom.h>
#include <avr/interrupt.h>  /* ��� sei() */
#include <util/delay.h>     /* ��� _delay_ms() */

#include <avr/pgmspace.h>   /* ����� ��� usbdrv.h */
#include "usbdrv\usbdrv.h"
#include "requests.h"       /* ������������ ������ custom request */

//� usbconfig.h �������� ��������� ���������� V-USB ��� ������ ����������
//(�������� ��� ������������� ����� ����� ����� ������������ ���������� 
//��� ������� � ������. � ���� ��� ����� 0 � 2 ����� �)

/* ------------------------------------------------------------------------- */
/* ----------------------------- ��������� USB ----------------------------- */
/* ------------------------------------------------------------------------- */

struct dataexchange_t       // �������� ��������� ��� �������� ������
{
    //������ ���� �������� �� ��������� ����� �� PORTB ��������������� .
   uchar b1;          //����� 1    
   uchar b2;          //����� 3
   uchar b3;          //����� 4
   uchar b4;          //����� 5
};                  


struct dataexchange_t pdata = {0, 0, 0, 0}; //�������� ���������

PROGMEM char usbHidReportDescriptor[22] = { // USB report descriptor         // ���������� ��������� ��������� ������ ������ ��� ������
    0x06, 0x00, 0xff,                       // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                             // USAGE (Vendor Usage 1)
    0xa1, 0x01,                             // COLLECTION (Application)
    0x15, 0x00,                             //    LOGICAL_MINIMUM (0)        // min. �������� ��� ������
    0x26, 0xff, 0x00,                       //    LOGICAL_MAXIMUM (255)      // max. �������� ��� ������, 255 ��� �� ��������, � ����� ��������� � 1 ����
    0x75, 0x08,                             //    REPORT_SIZE (8)            // ���������� ���������� ��������, ��� ������ ������ "�������" 8 ���
    0x95, sizeof(struct dataexchange_t),    //    REPORT_COUNT               // ���������� ������ (� ����� ������� = 4, ��������� ���� ��������� ���������� �� 4 �������)
    0x09, 0x00,                             //    USAGE (Undefined)
    0xb2, 0x02, 0x01,                       //    FEATURE (Data,Var,Abs,Buf)
    0xc0                                    // END_COLLECTION
};
/* ����� �� ������� ������ ���� report, ��-�� ���� �� ����� ������������ report-ID (�� ������ ���� ������ ������).
 * � ��� ������� ��������� 4 ����� ������ (������ ������ REPORT_SIZE = 8 ��� = 1 ����, �� ���������� REPORT_COUNT = 4).
 */


/* ------------------------------------------------------------------------- */
/* ��� ���������� ������ ������ ������� �������� */
static uchar    currentAddress;
static uchar    bytesRemaining;
unsigned int   i;//������ ������ ����� 1,3,4,5 
/* usbFunctionRead() ���������� ����� ���� ����������� ������ ������ �� ����������
 * ��� �������������� ���������� ��. ������������ � usbdrv.h
 */
uchar   usbFunctionRead(uchar *data, uchar len)
{
    if(len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;  //��������� �� ���������

    if(!currentAddress)        // �� ���� ����� ������ ��� �� ��������.
    {                          // �������� ��������� ��� ��������
        if ( PINB & _BV(1) )   //������ �������� 1 ����� � ���������
            pdata.b1 = 1;
        else
            pdata.b1 = 0;


        if ( PINB & _BV(3) )   //������ �������� ����� 3 � ���������
            pdata.b2 = 1;
        else
            pdata.b2 = 0;


        if ( PINB & _BV(4) )    //������ �������� ����� 4 � ���������
            pdata.b3 = 1;
        else
            pdata.b3 = 0;

		//������ �������� ����� 5 � ���������
		//(�� ��������� ����� 5 ������������� ��� ������ ��������(RESET) 
		//��� �������� ����� � ����� ������\������ � fuse ����� ���������� RSTDISBL � ����.
		// ����� �������� fuse �������� �������� ����� ������ �������������� �������������� 
        if ( PINB & _BV(5) )    
		    pdata.b4 = 1;
        else
            pdata.b4 = 0;
    }

    uchar j;
    for(j=0; j<len; j++)
        data[j] = buffer[j+currentAddress];

    currentAddress += len;
    bytesRemaining -= len;
    return len;
}


/* usbFunctionWrite() ���������� ����� ���� ���������� ������ ������ � ����������
 * ��� �������������� ���������� ��. ������������ � usbdrv.h
 */
uchar   usbFunctionWrite(uchar *data, uchar len)
{
    if(bytesRemaining == 0)
        return 1;               /* ����� �������� */

    if(len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;
    
    uchar j;
    for(j=0; j<len; j++)
        buffer[j+currentAddress] = data[j];

    currentAddress += len;
    bytesRemaining -= len;

    if(bytesRemaining == 0)     // ��� ������ ��������
    {                           // �������� �������� �� PORTB
        if ( pdata.b1 )         //������ �������� ����� 1 �� ���������
            PORTB |= _BV(1);
        else
            PORTB &= ~_BV(1);


        if ( pdata.b2 )         //������ �������� ����� 3 �� ���������
            PORTB |= _BV(3);
        else
            PORTB &= ~_BV(3);


        if ( pdata.b3 )         //������ �������� ����� 4 �� ���������
            PORTB |= _BV(4);
        else
            PORTB &= ~_BV(4);

        if ( pdata.b4 )         //������ �������� ����� 5 �� ���������
            PORTB |= _BV(5);
        else
            PORTB &= ~_BV(5);
    }
    i=0;                        //�������� ������ 
    return bytesRemaining == 0; /* 0 �������� ��� ���� ��� ������ */  
	//���� bytesRemaining!=0 �� return 0, ���� bytesRemaining==0 �� return 1
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)     /* HID ���������� */
	   {    
        if(rq->bRequest == USBRQ_HID_GET_REPORT)     /* wValue: ReportType (highbyte), ReportID (lowbyte) */
		  {  
            // � ��� ������ ���� ������������� �������, ����� ������������ report-ID
            bytesRemaining = sizeof(struct dataexchange_t);
            currentAddress = 0;
            return USB_NO_MSG;  // ���������� usbFunctionRead() ��� �������� ������ ����� 
          }else if(rq->bRequest == USBRQ_HID_SET_REPORT)
		       {
                 // � ��� ������ ���� ������������� �������, ����� ������������ report-ID
                 bytesRemaining = sizeof(struct dataexchange_t);
                 currentAddress = 0;
                 return USB_NO_MSG;  // ���������� usbFunctionWrite() ��� ��������� ������ �� ����� 
               }
       }else
	     {
            /* ��������� ������� �� ������ ���������� */
         }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* ------------------------ Oscillator Calibration ------------------------- */
/* ------------------------------------------------------------------------- */

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
static void calibrateOscillator(void)
{
uchar       step = 128;
uchar       trialValue = 0, optimumValue;
int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

    /* do a binary search: */
    do{
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    /* proportional to current real frequency */
        if(x < targetValue)             /* frequency still too low */
            trialValue += step;
        step >>= 1;
    }while(step > 0);
    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; /* this is certainly far away from optimum */
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0)
            x = -x;
        if(x < optimumDev){
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;
}

/*
Note: This calibration algorithm may try OSCCAL values of up to 192 even if
the optimum value is far below 192. It may therefore exceed the allowed clock
frequency of the CPU in low voltage designs!
You may replace this search algorithm with any other algorithm you like if
you have additional constraints such as a maximum CPU clock.
For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
both regions.
*/

void    usbEventResetReady(void)
{
    calibrateOscillator();
    eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}


/* ------------------------------------------------------------------------- */

int main(void)
{
DDRB = 0b111010;     //��������� ����� 1,3,4,5 �� ������, ����� 0,2 � ������
   uchar   calibrationValue;

   calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
   if(calibrationValue != 0xff)
   {
      OSCCAL = calibrationValue;
   }
   wdt_enable(WDTO_1S);                   //�������� ���������� ������ �� ������� ����� 1 �������
   /* ���� ���� �� �� ����������� ���������� ������ (watchdog), ��������� ��� �����. �� ����� ����� 
    *  ����������������� ��������� watchdog (���\����, ������) ����������� ����� �����!
    */
   usbInit();
   usbDeviceDisconnect();    /* ������������� ��������� ��-���������� (������� ���, ����� ���������� ���������!) */
   i = 20;
   while(--i)
   {                         /* ���������� USB ���������� �� ����� > 200 �� */
      wdt_reset();           
      _delay_ms(10);         //������ �������� �� �������� ���������� ����������.
   }
   usbDeviceConnect();      //���������� ���������
   sei();                   //�������� ��� ����������. �� ����������� ����� �������, ����� ��� ������ ��������.
   i=0;
   for(;;){                /* ���� ������� main */
      wdt_reset();         //����� ����������� �������
      usbPoll();           //������� ������� usbFunctionWrite, usbFunctionRead /* ���������� �������� �� ���� ��� 50 ms � �������� �����, ��� ���� ���������� �� ���� USB ��� ����� � ���� ������ ���� (������ � ������) */
	  i++;
	  _delay_ms(10);       
	  if(i==200)           //����� 1,3,4,5 ����� � 0 ����� 2���
	    {
		   PORTB &= ~_BV(1);
		   PORTB &= ~_BV(3);
		   PORTB &= ~_BV(4);
		   PORTB &= ~_BV(5);
		}
   }
   return 0;
}

/* ------------------------------------------------------------------------- */
