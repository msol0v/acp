//#include "stdafx.h"
#include <windows.h>
#include <setupapi.h>	
#include <winioctl.h>
#include <stdio.h>
#include <objbase.h>
#include <initguid.h>
#include "intrfacePCI3.h"	
#include "ioctlPCI3.h"	


USHORT	bufOutput;

HANDLE OpenDeviceBySN(  USHORT SerialNumber, PDWORD pError )
{
	GUID ClassGuid = GUID_DEVINTERFACE_PCI429_3WIN;
	HANDLE hDev;
	HDEVINFO hDevinfo;
	SP_DEVICE_INTERFACE_DATA	DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pDetail = NULL;
	DWORD  ReqSize
		,nOutput;
	int index= 0;
	do{


		hDevinfo = SetupDiGetClassDevs(	(LPGUID)&ClassGuid,
										NULL,
										NULL,
										DIGCF_DEVICEINTERFACE |DIGCF_PRESENT );

		if(hDevinfo == INVALID_HANDLE_VALUE)
		{
			*pError = GetLastError();
			return INVALID_HANDLE_VALUE;
		}

		DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

		if (!SetupDiEnumDeviceInterfaces(	hDevinfo,
											NULL,
											(LPGUID)&ClassGuid,
											index,
											&DeviceInterfaceData))
								
		{
			*pError = GetLastError();
			return INVALID_HANDLE_VALUE;
		}


			SetupDiGetDeviceInterfaceDetail(	hDevinfo,
												&DeviceInterfaceData,
												NULL,
												0,
												&ReqSize,
												NULL );

			pDetail = PSP_INTERFACE_DEVICE_DETAIL_DATA(new char[ReqSize]);

			if ( !pDetail )
			{
				*pError = ERROR_NOT_ENOUGH_MEMORY;
				return INVALID_HANDLE_VALUE;
			}
			pDetail->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

		if (! SetupDiGetDeviceInterfaceDetail(	hDevinfo,
												&DeviceInterfaceData,
												pDetail,
												ReqSize,
												&ReqSize,
												NULL )
												)
		{
			*pError = GetLastError();
			return INVALID_HANDLE_VALUE;
		}


		SetupDiDestroyDeviceInfoList(hDevinfo);


		hDev = CreateFile(
			pDetail->DevicePath,
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (hDev == INVALID_HANDLE_VALUE)
		{
			*pError = GetLastError();
			return INVALID_HANDLE_VALUE;
		}

		DeviceIoControl(hDev,
					 DRVNT_PCI429,
					 NULL,               
					 0,					
					 &bufOutput,
					 2,
					 &nOutput,
					 NULL);


		index++;

		delete pDetail;
	}
	while (SerialNumber!=bufOutput);



		*pError = ERROR_SUCCESS;

	return hDev;
}
