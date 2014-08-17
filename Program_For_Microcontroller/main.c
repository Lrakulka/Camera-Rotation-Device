/*
 * Проект: Web_turel_came сделано с использованием hid-custom-rq(взял с: microsin.ru) и Hid_example_firmware(взял с: http://we.easyelectronics.ru/electro-and-pc/usb-dlya-avr-chast-2-hid-class-na-v-usb.html)
 * Автор: Murk
 * Дата создания: 2013-11-15
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * Лицензия: GNU GPL v2 (см. License.txt) или проприетарная (CommercialLicense.txt) 
 * мои коментарии начинаютса сразу после //"Коментарий".коментарии авторов проектов каторыми пользавался начинаютса так // "Коментарий" или /* "Коментарий" 
 */


#include <avr/io.h>
#include <avr/wdt.h>         //библиотека для работы со сторожевым таймером
#include <avr/eeprom.h>
#include <avr/interrupt.h>  /* для sei() */
#include <util/delay.h>     /* для _delay_ms() */

#include <avr/pgmspace.h>   /* нужен для usbdrv.h */
#include "usbdrv\usbdrv.h"
#include "requests.h"       /* используемые номера custom request */

//В usbconfig.h записаны настройки библиотеки V-USB для роботи устройства
//(например там прописаваютса какие ножки нужно использавать библиотеке 
//для общения с хостом. У меня это ножки 0 и 2 порта В)

/* ------------------------------------------------------------------------- */
/* ----------------------------- интерфейс USB ----------------------------- */
/* ------------------------------------------------------------------------- */

struct dataexchange_t       // Описание структуры для передачи данных
{
    //каждый байт отвичает за состояние ножки из PORTB микроконтролера .
   uchar b1;          //ножка 1    
   uchar b2;          //ножка 3
   uchar b3;          //ножка 4
   uchar b4;          //ножка 5
};                  


struct dataexchange_t pdata = {0, 0, 0, 0}; //создание структуры

PROGMEM char usbHidReportDescriptor[22] = { // USB report descriptor         // Дескриптор описывает структуру пакета данных для обмена
    0x06, 0x00, 0xff,                       // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                             // USAGE (Vendor Usage 1)
    0xa1, 0x01,                             // COLLECTION (Application)
    0x15, 0x00,                             //    LOGICAL_MINIMUM (0)        // min. значение для данных
    0x26, 0xff, 0x00,                       //    LOGICAL_MAXIMUM (255)      // max. значение для данных, 255 тут не случайно, а чтобы уложиться в 1 байт
    0x75, 0x08,                             //    REPORT_SIZE (8)            // информация передается порциями, это размер одного "репорта" 8 бит
    0x95, sizeof(struct dataexchange_t),    //    REPORT_COUNT               // количество порций (у нашем примере = 4, описанная выше структура передастся за 4 репорта)
    0x09, 0x00,                             //    USAGE (Undefined)
    0xb2, 0x02, 0x01,                       //    FEATURE (Data,Var,Abs,Buf)
    0xc0                                    // END_COLLECTION
};
/* Здесь мы описали только один report, из-за чего не нужно использовать report-ID (он должен быть первым байтом).
 * С его помощью передадим 4 байта данных (размер одного REPORT_SIZE = 8 бит = 1 байт, их количество REPORT_COUNT = 4).
 */


/* ------------------------------------------------------------------------- */
/* Эти переменные хранят статус текущей передачи */
static uchar    currentAddress;
static uchar    bytesRemaining;
unsigned int   i;//таймер сброса ножек 1,3,4,5 
/* usbFunctionRead() вызывается когда хост запрашивает порцию данных от устройства
 * Для дополнительной информации см. документацию в usbdrv.h
 */
uchar   usbFunctionRead(uchar *data, uchar len)
{
    if(len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;  //Указатель на структуру

    if(!currentAddress)        // Ни один кусок данных еще не прочитан.
    {                          // Заполним структуру для передачи
        if ( PINB & _BV(1) )   //Запись сотояние 1 ножки в структуру
            pdata.b1 = 1;
        else
            pdata.b1 = 0;


        if ( PINB & _BV(3) )   //Запись сотояние ножки 3 в структуру
            pdata.b2 = 1;
        else
            pdata.b2 = 0;


        if ( PINB & _BV(4) )    //Запись сотояние ножки 4 в структуру
            pdata.b3 = 1;
        else
            pdata.b3 = 0;

		//Запись сотояние ножки 5 в структуру
		//(По умолчанию ножка 5 придназначена для сброса прошивки(RESET) 
		//для перехода ножки в режим записи\чтения в fuse нужно установить RSTDISBL в ноль.
		// После прошивки fuse изменить прошивку можно только высоковольтным программатором 
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


/* usbFunctionWrite() вызывается когда хост отправляет порцию данных к устройству
 * Для дополнительной информации см. документацию в usbdrv.h
 */
uchar   usbFunctionWrite(uchar *data, uchar len)
{
    if(bytesRemaining == 0)
        return 1;               /* конец передачи */

    if(len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;
    
    uchar j;
    for(j=0; j<len; j++)
        buffer[j+currentAddress] = data[j];

    currentAddress += len;
    bytesRemaining -= len;

    if(bytesRemaining == 0)     // Все данные получены
    {                           // Выставим значения на PORTB
        if ( pdata.b1 )         //Запись сотояния ножки 1 из структуры
            PORTB |= _BV(1);
        else
            PORTB &= ~_BV(1);


        if ( pdata.b2 )         //Запись сотояния ножки 3 из структуры
            PORTB |= _BV(3);
        else
            PORTB &= ~_BV(3);


        if ( pdata.b3 )         //Запись сотояния ножки 4 из структуры
            PORTB |= _BV(4);
        else
            PORTB &= ~_BV(4);

        if ( pdata.b4 )         //Запись сотояния ножки 5 из структуры
            PORTB |= _BV(5);
        else
            PORTB &= ~_BV(5);
    }
    i=0;                        //обнулить таймер 
    return bytesRemaining == 0; /* 0 означает что есть еще данные */  
	//если bytesRemaining!=0 то return 0, если bytesRemaining==0 то return 1
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)     /* HID устройство */
	   {    
        if(rq->bRequest == USBRQ_HID_GET_REPORT)     /* wValue: ReportType (highbyte), ReportID (lowbyte) */
		  {  
            // у нас только одна разновидность репорта, можем игнорировать report-ID
            bytesRemaining = sizeof(struct dataexchange_t);
            currentAddress = 0;
            return USB_NO_MSG;  // используем usbFunctionRead() для отправки данных хосту 
          }else if(rq->bRequest == USBRQ_HID_SET_REPORT)
		       {
                 // у нас только одна разновидность репорта, можем игнорировать report-ID
                 bytesRemaining = sizeof(struct dataexchange_t);
                 currentAddress = 0;
                 return USB_NO_MSG;  // используем usbFunctionWrite() для получения данных от хоста 
               }
       }else
	     {
            /* остальные запросы мы просто игнорируем */
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
DDRB = 0b111010;     //Установка ножек 1,3,4,5 на запись, ножки 0,2 в чтение
   uchar   calibrationValue;

   calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
   if(calibrationValue != 0xff)
   {
      OSCCAL = calibrationValue;
   }
   wdt_enable(WDTO_1S);                   //включаем сторожевой таймер со сбросом через 1 секунду
   /* Даже если Вы не используете сторожевой таймер (watchdog), выключите его здесь. На более новых 
    *  микроконтроллерах состояние watchdog (вкл\выкл, период) СОХРАНЯЕТСЯ ЧЕРЕЗ СБРОС!
    */
   usbInit();
   usbDeviceDisconnect();    /* принудительно запускаем ре-энумерацию (делайте это, когда прерывания запрещены!) */
   i = 20;
   while(--i)
   {                         /* иммитируем USB дисконнект на время > 200 мс */
      wdt_reset();           
      _delay_ms(10);         //делает задержку на указаное количество милисекунд.
   }
   usbDeviceConnect();      //Подключить устройсво
   sei();                   //включает все прерывания. Ее обязательно нужно вызвать, чтобы они начали работать.
   i=0;
   for(;;){                /* цикл событий main */
      wdt_reset();         //сброс сторожевого таймера
      usbPoll();           //вызываєт функций usbFunctionWrite, usbFunctionRead /* необходимо вызывать не реже чем 50 ms — сообщает хосту, что наше устройство на шине USB ещё живое и ждет своего часа (готово к работе) */
	  i++;
	  _delay_ms(10);       
	  if(i==200)           //сброс 1,3,4,5 ножки в 0 через 2сек
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
