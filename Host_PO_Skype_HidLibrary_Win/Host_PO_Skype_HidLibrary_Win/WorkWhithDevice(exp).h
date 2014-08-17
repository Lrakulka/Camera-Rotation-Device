#include "usbconfig.h"   // «десь пишем путь к usbconfig.h
#include "hidlibrary.h"  // Ѕиблиотека дл€ работы с Hid устройствами
#define Max 230

class Device{
   class dataexchange_t		// ќписание структуры дл€ передачи данных
	{
	public:
	   char b1;        // я решил написать структуру на 4 байта.
	   char b2;        // Ќа каждый байт подцепим ногу из PORTB.  онечно это
	   char b3;        // не рационально (всего то 4 бита нужно).
	   char b4;
	}data;
   class Position{
	public:
		float x;
		float y;
   }positionTransform, cameraPosition;     /* positionTransform - нужно дл€ хранени€ трансформированных 
                                             градусных значений в мою систему cameraPosition - записуетьс€ 
											 текущее положение камеры в градусах */
   HIDLibrary <dataexchange_t> hid; // создаем экземпл€р класса с типом нашей структуры
   unsigned char processTime;       // дл€ уменьшени€ количества ненужного обращени€ к устройству
  // FILE *f;                         // файл дл€ сохранени€ последнего положени€ камеры
   int SendData(dataexchange_t *);
public:
   bool Connect();
   void SetCameraPosition(short int *, short int *);
   bool MoveCamera();
   bool StopMoveCamera();
   Device()
   {
	   processTime = 10;
	   (int&)data = positionTransform.x = positionTransform.y = 0;
	  // f = fopen("CameraPosition.txt","r+");
	  // fscanf(f,"%f %f", &cameraPosition.x, &cameraPosition.y);
	   if(cameraPosition.x < -Max || cameraPosition.y < -Max || cameraPosition.y > Max || cameraPosition.x > Max)
		   cameraPosition.x = cameraPosition.y = 0;
   }
   ~Device()
   {   
	   //fclose(f);
   }
};

int Device::SendData(dataexchange_t * data) // ‘ункци€ провер€ет может ли камера поворачиватьс€ по этой команде, отправл€ет команду устройству если может
{
	int res = 1;
	if((cameraPosition.x > Max && data->b1) || (cameraPosition.x < -Max && data->b2))
	{
		positionTransform.x = 0;
		return res;
	}
	if((cameraPosition.y > Max && data->b3) || (cameraPosition.y < -Max && data->b4))
	{
		positionTransform.y = 0;
		return res;
	}
	res = (hid.SendData(data));
	if(res)
	{
		//rewind(f);
	  //  fprintf(f,"%f %f", cameraPosition.x, cameraPosition.y);
		//fflush(f);
	}
	return res;
}

bool Device::Connect()  // этой функцией будем подключатьс€ к устройству
{
   int n;
   bool res=0;
   positionTransform.x = positionTransform.y = 0;
   char  vendorName[]  = {USB_CFG_VENDOR_NAME, 0}; // дл€ того что бы знать как
   char  productName[] = {USB_CFG_DEVICE_NAME, 0}; // называетс€ наше устройство

   string exampleDeviceName;
   exampleDeviceName += vendorName;
   exampleDeviceName += " ";
   exampleDeviceName += productName;
   n = hid.EnumerateHIDDevices(); // узнаем все Hid устройства vid_16c0&pid_05df
                                  // vid и pid указаны в hidlibrary.h константой idstring
   for (int i=0; i<n; i++)            // ищем среди них наше
   {
      hid.Connect(i);

      // GetConnectedDeviceName() возвращает string,
      // где через пробел указаны vendor и product Name.
      // —равниваем, если совпало - значить устройство наше
      if ( hid.GetConnectedDeviceName() == exampleDeviceName )
      {
         res = 1;
         break;
      }
   }
   return res;
}

void Device::SetCameraPosition(short int *x, short int *y){
	if((*x > Max || *x < -Max))
		*x = Max;
	if((*y > Max || *y < -Max))
		*y = Max;
    // “рансформаци€ градусной координаты в мою по горизонтали(камера вращаетс€ после команды 2сек)
	if(*x > 0)
		positionTransform.x = (*x * 0.20832); // разна€ скорость вращени€ по часовой стрелке и против
	else positionTransform.x = (*x * 0.225);  
	// “рансформаци€ градусной координаты в мою по вертикали(камера вращаетс€ после команды 2сек)
	if(*y > 0)
		positionTransform.y = (*y * 0.20832); 
	else positionTransform.y = (*y * 0.225);  
	processTime = 10;
}

bool Device::MoveCamera()// ‘ункци€ передает команды устройству
{
	if (positionTransform.x || positionTransform.y)
	{
		dataexchange_t data = {0, 0, 0, 0};
		if(!hid.ReceiveData(&data)) // ”знать выполн€емую команду
			return false;
		if(processTime < 9)     //ѕроверка сколько времени прошло с последней команды
		  processTime++;
		else
			for(;processTime;) //‘ормируем команду дл€ устройства
			  if(!((int&)(data)))
				{
				  if(positionTransform.x)   //‘ормируем команду дл€ поворота камеры по горизонтали
					{
						if(positionTransform.x > 0)
						{
							data.b1 = 1;
							if(positionTransform.x < 1)   // ƒл€ поворота камеры если нужно меньше 2сек
							{
							   if(!SendData(&data))
								 return false;
							   Sleep(positionTransform.x * 2000);	
							   cameraPosition.x += positionTransform.x * 4.8;
							   data.b1 = positionTransform.x = 0;							   
							}
							else 
							{							   
							   positionTransform.x--; 
							   cameraPosition.x += 4.8;		
							}
						}
						else{
							data.b2 = 1;
							if(positionTransform.x > -1)
							{
							   if(!SendData(&data))
								 return false;
							   Sleep(abs(positionTransform.x) * 2000);							   
							   cameraPosition.x -= 4.4 * abs(positionTransform.x);
							   data.b2 = positionTransform.x = 0;
							}
							else 
							{
								positionTransform.x++; 
								cameraPosition.x -= 4.4;
							}
						}
					}
					else{
						if(positionTransform.y)
						  {
							if(positionTransform.y > 0)
							{
								data.b3 = 1;
								if(positionTransform.y < 1)
								{
								   if(!SendData(&data))
									 return false;
								   Sleep(positionTransform.y * 2000);								   
								   cameraPosition.y += positionTransform.y * 4.8;
								   data.b3 = positionTransform.y = 0;
								}
								else
								{
									positionTransform.y--; 
									cameraPosition.y += 4.8;
								}
							}
							else{
								data.b4 = 1;
								if(positionTransform.y > -1)
								{					   
								   if(!SendData(&data))
									 return false;
								   Sleep(abs(positionTransform.y) * 2000);								   
								   cameraPosition.y -= abs(positionTransform.y) * 4.4;
								   data.b4 = positionTransform.y = 0;
								}
								else 
								{
									positionTransform.y++; 	
									cameraPosition.y -= 4.4;
								}
							}					
						}
					} 
					processTime = 0;
					if(!SendData(&data)) //ќтправка команды
						return false;           
				}		
	}
	return true;
}

bool Device::StopMoveCamera()// ‘ункцией останавливает поворот камеры
{
	(int&)data = positionTransform.x = positionTransform.y = 0;
	if(!SendData(&data))
		return false;
	return true;
}