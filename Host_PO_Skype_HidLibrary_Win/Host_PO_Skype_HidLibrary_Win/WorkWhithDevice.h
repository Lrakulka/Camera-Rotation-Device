#include "usbconfig.h"   // ����� ����� ���� � usbconfig.h
#include "hidlibrary.h"  // ���������� ��� ������ � Hid ������������
#define Max 230

class Device{
   class dataexchange_t		// �������� ��������� ��� �������� ������
	{
	public:
	   char b1;        // � ����� ��� ������� �������� ��������� �� 4 �����.
	   char b2;        // �� ������ ���� �������� ���� �� PORTB. ������� ���
	   char b3;        // �� ����������� (����� �� 4 ���� �����).
	   char b4;
	}data;
   class Position{
	public:
		float x;
		float y;
   }positionTransform;     /* positionTransform - ����� ��� �������� ������������������ 
                                             ��������� �������� � ��� ������� */
   HIDLibrary <dataexchange_t> hid; // ������� ��������� ������ � ����� ����� ���������
   unsigned char processTime;       // ��� ���������� ���������� ��������� ��������� � ����������
public:
   bool Connect();
   void SetCameraPosition(short int *, short int *);
   bool MoveCamera();
   bool StopMoveCamera();
   Device()
   {
	   processTime = 10;
	   (int&)data = positionTransform.x = positionTransform.y = 0;
   }
};


bool Device::Connect()  // ���� �������� ����� ������������ � ����������
{
   int n;
   bool res=0;
   positionTransform.x = positionTransform.y = 0;
   char  vendorName[]  = {USB_CFG_VENDOR_NAME, 0}; // ��� ���� ��� �� ����� ���
   char  productName[] = {USB_CFG_DEVICE_NAME, 0}; // ���������� ���� ����������

   string exampleDeviceName;
   exampleDeviceName += vendorName;
   exampleDeviceName += " ";
   exampleDeviceName += productName;
   n = hid.EnumerateHIDDevices(); // ������ ��� Hid ���������� vid_16c0&pid_05df
                                  // vid � pid ������� � hidlibrary.h ���������� idstring
   for (int i=0; i<n; i++)            // ���� ����� ��� ����
   {
      hid.Connect(i);

      // GetConnectedDeviceName() ���������� string,
      // ��� ����� ������ ������� vendor � product Name.
      // ����������, ���� ������� - ������� ���������� ����
      if ( hid.GetConnectedDeviceName() == exampleDeviceName )
      {
         res = 1;
         break;
      }
   }
   return res;
}

void Device::SetCameraPosition(short int *x, short int *y)// ������� ��������������� ��������� ������� �������� ������ � ��� �������
{
	if((*x > Max || *x < -Max))
		*x = Max;
	if((*y > Max || *y < -Max))
		*y = Max;
    // ������������� �������� � ��� ������� �� �����������(������ ��������� ����� ������� 2���)
	if(*x > 0)
		positionTransform.x = (*x * 0.20832); // ������ �������� �������� �� ������� �������� � ������
	else positionTransform.x = (*x * 0.225);  
	// ������������� �������� � ��� ������� �� ���������(������ ��������� ����� ������� 2���)
	if(*y > 0)
		positionTransform.y = (*y * 0.20832); 
	else positionTransform.y = (*y * 0.225);  
	processTime = 10;
}

bool Device::MoveCamera()// ������� ��������� ������� � �������� �� ����������
{
	if (positionTransform.x || positionTransform.y)
	{
		dataexchange_t data = {0, 0, 0, 0};
		if(!hid.ReceiveData(&data)) // ������ ����������� �������
			return false;
		if(processTime < 9)         //�������� ������� ������� ������ � ��������� �������
		  processTime++;
		else
			for(;processTime;) //��������� ������� ��� ����������
			  if(!((int&)(data)))
				{
				  if(positionTransform.x)        //��������� ������� ��� �������� ������ �� ���������
					{
						if(positionTransform.x > 0)
						{
							data.b1 = 1;
							if(positionTransform.x < 1)    // ��� �������� ������ ���� ����� ������ 2���
							{
							   if(!hid.SendData(&data))  
								 return false;
							   Sleep(positionTransform.x * 2000);							   
							   data.b1 = positionTransform.x = 0;
							}
							else positionTransform.x--; 
						}
						else{
							data.b2 = 1;
							if(positionTransform.x > -1)
							{
							   if(!hid.SendData(&data))
								 return false;
							   Sleep(abs(positionTransform.x) * 2000);
							   data.b2 = positionTransform.x = 0;
							}
							else positionTransform.x++; 
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
								   if(!hid.SendData(&data))
									 return false;
								   Sleep(positionTransform.y * 2000);
								   data.b3 = positionTransform.y = 0;
								}
								else positionTransform.y--; 
							}
							else{
								data.b4 = 1;
								if(positionTransform.y > -1)
								{					   
								   if(!hid.SendData(&data))
									 return false;
								   Sleep(abs(positionTransform.y) * 2000);
								   data.b4 = positionTransform.y = 0;
								}
								else positionTransform.y++; 	
							}					
						}
					} 
					processTime = 0;
					if(!hid.SendData(&data)) //�������� �������
						return false;           
				}		
	}
	return true;
}

bool Device::StopMoveCamera() // �������� ������������� ������� ������
{
	(int&)data = positionTransform.x = positionTransform.y = 0;
	if(!hid.SendData(&data))
		return false;
	return true;
}