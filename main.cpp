
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

const int MAX_PATH = 256;

extern "C" int Box_Main();

extern "C" int Pyramids_Main();

extern "C" int AntTSP_Main();

extern "C" void * operator new[] (unsigned long size) {
	return (void*) AllocatePool ((UINTN) size);
}

extern "C" void operator delete (void *buffer) {
	FreePool((VOID*) buffer);
}

extern "C" void __cxa_throw_bad_array_new_length() {
}

extern EFI_GUID Tcp4Protocol;

extern EFI_GUID Tcp4ServiceBindingProtocol;

extern EFI_GUID gEfiSimpleNetworkProtocolGuid;

extern EFI_GUID NullGuid;

static int volatile cb_status = -1;

static EFIAPI void tcp_cb(EFI_EVENT ev, void *context)
{
    EFI_TCP4_COMPLETION_TOKEN *token = context;
    (void)ev;
    if (token->Status == EFI_SUCCESS)
	cb_status = 0;
    else
	cb_status = 1;
	
	Print (L"TCP_CB\r\n");
}

static EFI_IPv4_ADDRESS gLocalAddress = { 192, 168, 0, 12 };
static EFI_IPv4_ADDRESS gSubnetMask = { 255, 255, 255, 0 };

static EFI_IPv4_ADDRESS gRemoteAddress = { 217, 69, 139, 200 };
static UINT16 gRemotePort = 80;

extern "C"
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);
	
	Print(L"Welcome to LavoroL!\r\n");
	
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface = NULL;
	
	EFI_STATUS status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &FileSystemProtocol, NULL, (VOID**)&FileInterface);

	CHAR16 szCurrentPath[MAX_PATH];

	StrCpy(szCurrentPath, L"\\");

	if (!EFI_ERROR(status)) {
		EFI_FILE *file;
		
		status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in opening volume: %d\r\n", status);
		} else {
			EFI_FILE_SYSTEM_INFO *info = LibFileSystemInfo(file);
			
			if (info) {
				Print(L"\r\nVolume size (Mb): %d\r\n", (unsigned int) (info->VolumeSize / 1024 / 1024));
				Print(L"Volume label: %s\r\n", info->VolumeLabel);
			}
		}
	} else {
		Print(L"\r\nError in loading file interface: %d\r\n", status);
	}

	EFI_SIMPLE_NETWORK_PROTOCOL *hSimpleNetworkProtocol = NULL;
	
	EFI_HANDLE* handle = NULL, *handleService = NULL, *handleTCP4 = NULL;

	status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &gEfiSimpleNetworkProtocolGuid, NULL, (VOID**)&hSimpleNetworkProtocol);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating simple protocol: %d\r\n", status);
	}
	
	if (!hSimpleNetworkProtocol) {
		status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 5, &handle,
			&gEfiSimpleNetworkProtocolGuid, &hSimpleNetworkProtocol,
			&NullGuid, NULL
			);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in installing locating protocol (0): %d\r\n", status);
		}
		
		status = uefi_call_wrapper(
			hSimpleNetworkProtocol->Start, 1, hSimpleNetworkProtocol);
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in shutdown simple protocol: %d\r\n", status);
		}
	}
	
	EFI_SERVICE_BINDING *ServiceBinding = NULL;
	
	status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &Tcp4ServiceBindingProtocol, NULL, (VOID**)&ServiceBinding);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating protocol: %d\r\n", status);
	}
	
	handleService = handle;
	
	if (!ServiceBinding) {
		status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 5, &handleService,
			&Tcp4ServiceBindingProtocol, &ServiceBinding,
			&NullGuid, NULL
			);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in installing locating protocol (1): %d\r\n", status);
		}
	}

	EFI_HANDLE                      *HandleBuffer;
	UINTN                           NumHandles;
		
	status = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
                  ByProtocol,
                  &Tcp4ServiceBindingProtocol,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating buffer: %d\r\n", status);
	}
	
	Print (L"PP %d\r\n", NumHandles);

	status =  uefi_call_wrapper(BS->OpenProtocol, 6,
                    HandleBuffer[0],
                    &Tcp4ServiceBindingProtocol,
                    (VOID **) &ServiceBinding,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in opening protocol: %d\r\n", status);
	}

	EFI_TCP4 *TCP4 = NULL;

	EFI_HANDLE mTCP4 = NULL;
	
	status = uefi_call_wrapper(ServiceBinding->CreateChild, 2, ServiceBinding, &mTCP4);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in creating child: %d\r\n", status);
	}
	
	Print(L"ok3\r\n");
	
	status = uefi_call_wrapper(BS->OpenProtocol, 6,
                   mTCP4,
                   &Tcp4Protocol,
                   (VOID **)&TCP4,
                   ImageHandle,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in creating child (2): %d\r\n", status);
	}

	/*
	status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &Tcp4Protocol, NULL, (VOID**)&TCP4);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating TCP protocol: %d\r\n", status);
	}

	Print(L"\r\nHandle: %d\r\n", (int) ImageHandle);

	if (!TCP4) {
		status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 5, &handleTCP4,
			&Tcp4Protocol, &TCP4,
			&NullGuid, NULL
			);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in installing locating protocol (2): %d\r\n", status);
		}
	}
	*/
	
	/*
	status = uefi_call_wrapper(
		hSimpleNetworkProtocol->Shutdown, 1, hSimpleNetworkProtocol);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in shutdown simple protocol: %d\r\n", status);
	}
	
	status = uefi_call_wrapper(
		hSimpleNetworkProtocol->Start, 1, hSimpleNetworkProtocol);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in shutdown simple protocol: %d\r\n", status);
	}
	*/
	
	/*
	status = uefi_call_wrapper(
		hSimpleNetworkProtocol->Reset, 2, hSimpleNetworkProtocol, TRUE);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in reset simple protocol: %d\r\n", status);
	}
	*/
		
	/*
	
	*/
	
	/*
	Print(L"ok1\r\n");
	
	status =  uefi_call_wrapper(BS->OpenProtocol, 6,
                    handle,
                    &Tcp4ServiceBindingProtocol,
                    (VOID **) &ServiceBinding,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in opening protocol: %d\r\n", status);
	}
	*/
	
	/*
	Print(L"ok2\r\n");
	
	EFI_HANDLE mTCP4 = NULL;
	
	status = uefi_call_wrapper(ServiceBinding->CreateChild, 2, ServiceBinding, &mTCP4);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in creating child: %d\r\n", status);
	}
	
	Print(L"ok3\r\n");
	*/
	
	/*
	status = uefi_call_wrapper(BS->OpenProtocol, 6,
                   handle,
                   &Tcp4Protocol,
                   (VOID **)&TCP4,
                   ImageHandle,
                   NULL,
                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                   );
	*/
	
	Print(L"ok4\r\n");

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in opening TCP protocol: %d\r\n", status);
	}
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in installing protocols: %d\r\n", status);
	} else {
		Print(L"\r\nHandle: %d, %d\r\n", (int) handle, (int) TCP4);
		
		/*		
		status = uefi_call_wrapper(
			ST->BootServices->LocateProtocol,
			3, &gEfiSimpleNetworkProtocolGuid, NULL, (VOID**)&hSimpleNetworkProtocol);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in locating simple protocol: %d\r\n", status);
		}

		Print (L"a1\r\n");
		*/
		
		/*
		status = uefi_call_wrapper(
			hSimpleNetworkProtocol->Start, 1, hSimpleNetworkProtocol);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in starting simple protocol: %d\r\n", status);
		}
		*/

		//Print (L"a12\r\n");
		
		/*
		
		status = uefi_call_wrapper(
			ST->BootServices->LocateProtocol,
			3, &Tcp4ServiceBindingProtocol, NULL, (VOID**)&ServiceBinding);
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in locating protocol: %d\r\n", status);
		}
		
		Print (L"a2\r\n");
		
		*/
		
		/*
		status = uefi_call_wrapper(
			ST->BootServices->LocateProtocol,
			3, &Tcp4Protocol, NULL, (VOID**)&TCP4);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in locating TCP protocol: %d\r\n", status);
		}

		Print (L"a3\r\n");
		*/
		
		/*
		status = uefi_call_wrapper(
			ServiceBinding->CreateChild,
			2, ServiceBinding, (EFI_HANDLE*) &TCP4);
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating child: %d\r\n", status);
		}
		*/
		
		Print(L"\r\nHandle (2): %d, %d\r\n", (int) handle, (int) TCP4);
		
		EFI_TCP4_CONNECTION_STATE       Tcp4State;
    	EFI_TCP4_CONFIG_DATA            Tcp4ConfigData;
    	EFI_IP4_MODE_DATA               Ip4ModeData;
    	EFI_MANAGED_NETWORK_CONFIG_DATA MnpConfigData;
    	EFI_SIMPLE_NETWORK_MODE         SnpModeData;
    	
		EFI_TCP4_CONNECTION_TOKEN token;
	    EFI_TCP4_ACCESS_POINT *ap;
    	EFI_TCP4_CONFIG_DATA tdata;
    	EFI_TCP4 *tcp = (EFI_TCP4 *) TCP4;

		Print (L"a4\r\n");
	    
		status = uefi_call_wrapper(tcp->GetModeData, 6, tcp, &Tcp4State, &Tcp4ConfigData, &Ip4ModeData, &MnpConfigData, &SnpModeData);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError getting mode data: %d\r\n", status);
		}

		Print(L"TCP4 State: %d\r\n", Tcp4State);
		Print(L"TTL: %d\r\n", Tcp4ConfigData.TimeToLive);
		Print(L"ToS: %d\r\n", Tcp4ConfigData.TypeOfService);
		
    	Print (L"e1\r\n");
    	
    	ZeroMem(&tdata, sizeof(tdata));

    	//Print (L"e12\r\n");
    	
    	//UINT32 ip = ((((((217) << 8) | 69) << 8) | 139) << 8) | 200;
    	//UINT32 ip = ((((((127) << 8) | 0) << 8) | 0) << 8) | 1;
    	
    	//Print (L"e13\r\n");

    	ap = &tdata.AccessPoint;
    	tdata.TypeOfService = 0;
	    ap->UseDefaultAddress = TRUE;
		CopyMem(&ap->RemoteAddress, &gRemoteAddress, sizeof(gRemoteAddress));
		CopyMem(&ap->SubnetMask, &gSubnetMask, sizeof (gSubnetMask));
		ap->StationPort = 100;
		ap->RemotePort = 80;
		ap->ActiveFlag = TRUE;
		
		CopyMem(&ap->StationAddress, &gLocalAddress, sizeof (gLocalAddress));
		tdata.ControlOption = Tcp4ConfigData.ControlOption;
		
		//Print (L"e14\r\n");

		tdata.TimeToLive = 800;

		//Print (L"e15\r\n");
		
		//Print (L"e16\r\n");
		
		status = uefi_call_wrapper(tcp->Configure, 2, tcp, &tdata);
		
		while (status == EFI_NO_MAPPING) {
			status = uefi_call_wrapper(tcp->Configure, 2, tcp, &tdata);
			
			int r = 0;
			for (int i = 0; i < 1000000; ++i) {
				r += 10 + i;
			}
		}
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in configuring TCP protocol: %d\r\n", status);
		}
		
		Print (L"e17\r\n");
						
		status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) tcp_cb, &token.CompletionToken, &token.CompletionToken.Event);
		
		//Print (L"e3\r\n");
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
		}
		
		status = uefi_call_wrapper(tcp->Connect, 2, tcp, &token);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in connecting: %d\r\n", status);
		}
		
		//Print (L"e4\r\n");
		
		while (cb_status == -1) {
			status = uefi_call_wrapper(tcp->Poll, 1, tcp);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
			
			//break;
		}
		
		cb_status = -1;
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, token.CompletionToken.Event);

    	//Print (L"e5\r\n");

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event: %d\r\n", status);
		}
		
		EFI_TCP4_IO_TOKEN iotoken;
    	EFI_TCP4_TRANSMIT_DATA txdata;
		EFI_TCP4_RECEIVE_DATA rxdata;
    	EFI_TCP4_FRAGMENT_DATA *frag;
    	
    	ZeroMem(&txdata, sizeof(txdata));
    	ZeroMem(&iotoken, sizeof(iotoken));    	
    	
    	CHAR8* transmitData = "GET / HTTP/1.0\r\n\r\n";
    	
    	//Print (L"e6\r\n");
    	
    	txdata.DataLength = 20;
	    txdata.FragmentCount = 1;
	    frag = &txdata.FragmentTable[0];
	    frag->FragmentLength = 20;
	    frag->FragmentBuffer = (void*)transmitData;
	    
	    iotoken.Packet.TxData = &txdata;

		status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) tcp_cb, &iotoken.CompletionToken, &iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event (2): %d\r\n", status);
		}
		
		//Print (L"e7\r\n");
		
		status = uefi_call_wrapper(tcp->Transmit, 2, tcp, &iotoken);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError transmitting data: %d\r\n", status);
		}
		
		while (cb_status == -1) {
			status = uefi_call_wrapper(tcp->Poll, 1, tcp);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
			
			//break;
		}
		
		//Print (L"e8\r\n");
		
		cb_status = -1;
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event (2): %d\r\n", status);
		}
		
		//Print (L"e9\r\n");
		
		ZeroMem(&rxdata, sizeof(rxdata));
    	ZeroMem(&iotoken, sizeof(iotoken));
    	
    	status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) tcp_cb, &iotoken.CompletionToken, &iotoken.CompletionToken.Event);
			
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event (3): %d\r\n", status);
		}
		
		//Print (L"e10\r\n");
		
		CHAR8 databuf[4096];
		
		iotoken.Packet.RxData = &rxdata;
	    rxdata.FragmentCount = 1;
    	rxdata.DataLength = sizeof(databuf);
    	frag = &rxdata.FragmentTable[0];
    	frag->FragmentBuffer = databuf;
    	frag->FragmentLength = sizeof(databuf);
    	
    	status = uefi_call_wrapper(tcp->Receive, 2, tcp, &iotoken);
    	
    	//Print (L"e11\r\n");
    	
    	if (status == EFI_CONNECTION_FIN) {
    		Print(L"\r\nFinal connection\r\n");
		} else if (EFI_ERROR(status)) {
			Print(L"\r\nError in receiving: %d\r\n", status);
		}
		
		while (cb_status == -1) {
			status = uefi_call_wrapper(tcp->Poll, 1, tcp);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
			
			//break;
		}
		
		cb_status = -1;
		
		CHAR16 szBuffer[4096];
		
		//Print (L"e12\r\n");
		
		for (int i = 0; i < frag->FragmentLength; ++i) {
			szBuffer[i] = ((CHAR8*) frag->FragmentBuffer)[i];
		}
		
		//Print (L"e13\r\n");
		
		szBuffer[frag->FragmentLength] = '\0';
		
		Print(L"%s\r\n", szBuffer);
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event (3): %d\r\n", status);
		}
		
		//Print (L"e14\r\n");

	}

	while (1) {
		CHAR16 szLine[MAX_PATH];
		
		Input(L"\r\n$>> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
		
		if (!StrCmp(szLine, L"ACM/Box")) {
			Box_Main();
		} else if (!StrCmp(szLine, L"ACM/Pyramids")) {
			Pyramids_Main();
		} else if (!StrCmp(szLine, L"AntTSP")) {
			AntTSP_Main();
		} else if (!StrCmp(szLine, L"ls")) {
			EFI_FILE *file, *newFile;
			
			status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);
			
			if (!EFI_ERROR(status)) {				
				status = uefi_call_wrapper(file->Open, 5, file, &newFile, szCurrentPath, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
				
				if (EFI_ERROR(status)) {
					Print(L"\r\nError in opening directory: %d\r\n", status);
				} else {
					CHAR16 Buffer[MAX_PATH];
					
					UINTN BufferSize = MAX_PATH >> 1;
					
					while (BufferSize) {
						BufferSize = sizeof (Buffer) >> 1;
						
						status = uefi_call_wrapper(newFile->Read, 3, newFile, &BufferSize, Buffer);
						
						if (EFI_ERROR(status)) {
							Print(L"\r\nError in reading directory: %d\r\n", status);
						} else if (BufferSize > 0) {
							Print(L"\r\n%s%s", ((EFI_FILE_INFO*) Buffer)->FileName,
								((EFI_FILE_INFO*) Buffer)->Attribute & EFI_FILE_DIRECTORY ? L" [d]" : L"");
						}
					}
					
					Print(L"\r\n");
				}				
			} else {
				Print(L"\r\nError in opening volume: %d\r\n", status);
			}
		} else if (!StrnCmp(szLine, L"cd", 2)) {
			StrCpy(szCurrentPath, szLine + 3);
		} else if (!StrCmp(szLine, L"cwd")) {
			Print(L"\r\n%s\r\n", szCurrentPath);
		} else if (!StrnCmp(szLine, L"cat", 3)) {
			CHAR16 szPath[MAX_PATH];
			
			CHAR16 *szCatPath = szLine + 4;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			EFI_FILE *file, *newFile;
			
			status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);
			
			if (!EFI_ERROR(status)) {
				status = uefi_call_wrapper(file->Open, 5, file, &newFile, szPath, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
				
				if (EFI_ERROR (status)) {
					Print(L"\r\nError in opening file: %d\r\n", status);
				} else {
					CHAR8 Buffer[MAX_PATH];
					
					UINTN BufferSize = MAX_PATH >> 1;
					
					Print(L"\r\n");
					
					while (BufferSize) {
						BufferSize = sizeof (Buffer) >> 1;
						
						status = uefi_call_wrapper(newFile->Read, 3, newFile, &BufferSize, Buffer);
						
						if (EFI_ERROR(status)) {
							Print(L"\r\nError in reading file: %d\r\n", status);
						} else if (BufferSize > 0) {
							CHAR16 szBuffer[MAX_PATH];
							
							Buffer[BufferSize] = '\0';
							
							for (UINTN i = 0; i <= BufferSize; ++i) {
								szBuffer[i] = Buffer[i];
							}
							
							Print(L"%s", szBuffer);
						}
					}
					
					Print(L"\r\n");
				}
			}
			
		}
	}
	
	return EFI_SUCCESS;
}
