#import "Skype4COM.dll" 
#include "ATLComTime.h"
#include "WorkWhithDevice(exp).h"

using namespace SKYPE4COMLib;

void ControlDevice(ISkypePtr, char*, int&, IChatMessagePtr&);

bool ComandToDevice(char *, Device *, ISkypePtr, char *, int *, ICallPtr);

void main()
{ 
  char comandName[] = {"test.vebcamera"};  // ����� ������� 
  char comandStart[] = {"0000"};           // ������� ������ 
  int timeWait = 500;                      // ������������ ����� ����������� �������
  
	 // �������������� COM ���������� 
 CoInitialize(NULL); 
 
  // ������� Skype ������ 
  SKYPE4COMLib::ISkypePtr pSkype(__uuidof(SKYPE4COMLib::Skype)); 
 
  // ����������� � Skype API 
  pSkype->Attach(6,VARIANT_TRUE); 

  _bstr_t comand;
  IChatMessagePtr curMessage, privMessage = NULL;
  COleDateTime startTime(time(NULL));
  
  for(;;)// ����� ��� ������� ������ ��������
  {
	  curMessage = pSkype->GetMessages((_bstr_t)comandName)->GetItem(1); // �������� ��������� ��������� �� �������
	  comand = curMessage->GetBody();  // �������� ����� ���������
	  if(!strcmp((char*)comand,comandStart) && (privMessage == NULL || 
		  privMessage->GetId() != curMessage->GetId()) && (startTime.m_dt < curMessage->GetTimestamp()))
		{
			privMessage = curMessage;
			curMessage->PutSeen(true); // �������� ���������� ��������� ��� �����������
			ControlDevice(pSkype, comandName, timeWait, privMessage);		
		}
		else Sleep(5000);
  }

  // ������� ���������� �� ����� 
  pSkype = NULL; 
//  CoUninitialize(); 
}

void ControlDevice(ISkypePtr pSkype, char *comandName, int &timeWait, IChatMessagePtr &privMessage)
{
  _bstr_t comand;
  IChatMessagePtr curMessage;
  int timeToSendMessage = (timeWait/100)*80;
  Device device;
  ICallPtr call;
  // �������� (���� ������ �������, �� �������� ������)
  for(int i = pSkype->GetActiveCalls()->GetCount(); i != 0 ; i--)
  {
	  call = pSkype->GetActiveCalls()->GetItem(i); // �������� ������� ������
	  if(!strcmp((char*)(call->GetPartnerHandle()),comandName))
	  {
		  call->Finish(); // �������� ������
		  Sleep(2000);
		  break;
	  }
  }
  call = pSkype->PlaceCall(_bstr_t(comandName), L"", L"", L"");  // ������ �������
  pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"�'������� ���������")); 
  if(!device.Connect())                                                 // ������������ � ����������
    pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"������� ��������� �� ��������"));	
  for(int i = 0;i < timeWait && call->Status != clsInProgress;i++)       // �������� �������� ������ ��������
	  Sleep(20);
  if(call->Status == clsInProgress)
  {
    Sleep(2000);
	call->StartVideoSend();  // �������� �����(������ �� ��������)
	for(int time = 0; call->Status == clsInProgress;time++)
	{
		if(time >= timeWait)  // ���� ����� �������� ��������� - �������� ������
		{
			call->Finish();
			break;
		}
		curMessage = pSkype->GetMessages((_bstr_t)comandName)->GetItem(1);
		if(curMessage->GetId() != privMessage->GetId()) // ����� ��������� � ����
		{
		  privMessage = curMessage;
		  if(time > 0)
	        time = 0;
		  if(!ComandToDevice((char*)curMessage->GetBody(),&device,pSkype,comandName,&time,call))// �������� ��������� ��� ���������
			  pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"������� �� ��������")); 
		  curMessage->PutSeen(true);
		}
		Sleep(200);
		if(!device.MoveCamera())
		{
			pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"�������� � �������� ���������"));	
			device.StopMoveCamera();
		}
		if(timeToSendMessage == time)
			pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"��� ���������� ������� ������")); 
	}
  }
  
  pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"�'������� ���������")); 
}

bool ComandToDevice(char *comand, Device *device, ISkypePtr pSkype, char *comandName, int *time, ICallPtr call)// ��������� ��������� � ����
{
	if(!strcmp(comand,"StopMove")&&!device->StopMoveCamera())
		return false;
	if(strstr(comand,"MoveTo") != NULL)
	{
		short int x = 0, y = 0;
		char pX[5], pY[5], *posa = strstr(comand,"@");
		if(posa != NULL)
		{
			strncpy(pY,(posa + 1),strlen(posa) - 2);
			strncpy(pX,&comand[7],strlen(comand) - strlen(posa) - 7);
			x = atoi(pX);
			y = atoi(pY);
			device->SetCameraPosition(&x,&y);
		}
		else return false;
	}
	if(strstr(comand,"Wait") != NULL)
	{
		short int wait = 0;
		char pWait[6];
		strncpy(pWait,&comand[4],strlen(comand) - 4);
		wait = atoi(pWait);
		*time = -1 * wait / 0.2;
	}
	if(!strcmp(comand,"VideoOn"))
	{		
		call->StartVideoSend();
	}
	if(!strcmp(comand,"VideoOff"))
	{
		call->StopVideoSend();
	}
	if(!strcmp(comand,"Help"))
		pSkype->SendMessage(_bstr_t(comandName), _bstr_t(L"StopMove  MoveTo('X'@'Y')  VideoOn  VideoOff  Wait'TimeInSec'")); 
}