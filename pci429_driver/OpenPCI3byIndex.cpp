//#include "stdafx.h"
#include <windows.h>
#include <setupapi.h>	
#include <winioctl.h>
#include <stdio.h>
#include <objbase.h>
#include <initguid.h>
#include "intrfacePCI3.h"	



HANDLE OpenDeviceByIndex(  DWORD Index, PDWORD pError )
{
	GUID ClassGuid = GUID_DEVINTERFACE_PCI429_3WIN;

	HANDLE hDev;
	HDEVINFO hDevinfo;
	SP_DEVICE_INTERFACE_DATA	DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pDetail = NULL;
	DWORD  ReqSize;


	hDevinfo = SetupDiGetClassDevs(	(LPGUID)&ClassGuid,
									NULL,
									NULL,
									DIGCF_DEVICEINTERFACE |DIGCF_PRESENT );

	if(hDevinfo == INVALID_HANDLE_VALUE)
	{
		*pError = GetLastError();
	printf("hARINC1\n");
		return INVALID_HANDLE_VALUE;
	}

	DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

	if (!SetupDiEnumDeviceInterfaces(	hDevinfo,
										NULL,
										(LPGUID)&ClassGuid,
										Index,
										&DeviceInterfaceData))
								
	{
		*pError = GetLastError();
	printf("hARINC2  %d\n",DeviceInterfaceData.cbSize);
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
	printf("hARINC3");
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
	printf("hARINC4");
		*pError = GetLastError();
		return INVALID_HANDLE_VALUE;
	}


	SetupDiDestroyDeviceInfoList(hDevinfo);


	hDev = CreateFile(
		pDetail->DevicePath,
		0,//GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (hDev == INVALID_HANDLE_VALUE)
	{
	printf("hARINC5");
		*pError = GetLastError();
		return INVALID_HANDLE_VALUE;
	}


		*pError = ERROR_SUCCESS;

	return hDev;
}
