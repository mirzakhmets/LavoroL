
#ifndef _L_SOCKET_

#define _L_SOCKET_

extern EFI_GUID Tcp4Protocol;

extern EFI_GUID Tcp4ServiceBindingProtocol;

extern EFI_GUID gEfiSimpleNetworkProtocolGuid;

static EFI_HANDLE gImageHandle = NULL;

static int volatile TCPEventStatus = -1;

EFIAPI void TCPCompletionTokenEvent(EFI_EVENT event, void *context) {
	EFI_TCP4_COMPLETION_TOKEN *token = context;

	(void) event;

	if (token->Status == EFI_SUCCESS) {
		TCPEventStatus = 0;
	} else {
		TCPEventStatus = 1;
	}
}

EFIAPI void TCPConnectionAccepted (EFI_EVENT Event, VOID *Context)
{
	EFI_STATUS             status;
	EFI_TCP4_LISTEN_TOKEN  *acceptToken;
	
	acceptToken = (EFI_TCP4_LISTEN_TOKEN *) Context;
	status = acceptToken->CompletionToken.Status;
	
	if (EFI_ERROR (status)) {
		Print (L"Connection Error: %d\n", status);
		return;
	}
	
	EFI_TCP4 *Child = NULL;
	
	status = uefi_call_wrapper(BS->OpenProtocol, 6,
	              acceptToken->NewChildHandle,
	              &Tcp4Protocol,
	              (VOID **) &Child,
	              gImageHandle,
	              NULL,
	              EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	
	if (EFI_ERROR (status)) {
		Print (L"Open TCP Connection: %d\n", status);
		return;
	}	
}

static EFI_HANDLE SimpleNetworkProtocolHandle = NULL;
static EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetworkProtocolInterface = NULL;

extern "C"
void InitializeNetworkProtocol() {
	EFI_STATUS status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &gEfiSimpleNetworkProtocolGuid, NULL, (VOID**)&SimpleNetworkProtocolInterface);

	if (EFI_ERROR(status)) {
		Print(L"\r\nSimple protocol not located: %d\r\n", status);
	}
	
	if (!SimpleNetworkProtocolInterface) {
		status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 4, &SimpleNetworkProtocolHandle,
			&gEfiSimpleNetworkProtocolGuid, &SimpleNetworkProtocolInterface,
			NULL);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in installing simple protocol: %d\r\n", status);
		}
		
		status = uefi_call_wrapper(
			SimpleNetworkProtocolInterface->Start, 1, SimpleNetworkProtocolInterface);
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in starting simple protocol: %d\r\n", status);
		}
	}
}

extern "C"
void FreeNetworkProtocol() {
	if (SimpleNetworkProtocolHandle) {
		EFI_STATUS status = uefi_call_wrapper(BS->UninstallMultipleProtocolInterfaces, 4, &SimpleNetworkProtocolHandle,
			&gEfiSimpleNetworkProtocolGuid, &SimpleNetworkProtocolInterface,
			NULL);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError uninstalling simple protocol: %d\r\n", status);
		}
	}
}

static EFI_HANDLE ServiceBindingHandle = NULL;
static EFI_SERVICE_BINDING *ServiceBinding = NULL;
static bool IsServiceBindingInstalled = false;

extern "C"
void InitializeBindingProtocol() {
	EFI_STATUS status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &Tcp4ServiceBindingProtocol, NULL, (VOID**)&ServiceBinding);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating protocol: %d\r\n", status);
	}
	
	if (SimpleNetworkProtocolHandle) {
		ServiceBindingHandle = SimpleNetworkProtocolHandle;
	}
	
	if (!ServiceBinding) {
		status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 4, &ServiceBindingHandle,
			&Tcp4ServiceBindingProtocol, &ServiceBinding,
			NULL);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in installing binding protocol: %d\r\n", status);
		}
		
		IsServiceBindingInstalled = true;
	}
	
	EFI_HANDLE	*HandleBuffer;
	UINTN       NumHandles;
	
	status = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
		ByProtocol,
        &Tcp4ServiceBindingProtocol,
        NULL,
        &NumHandles,
        &HandleBuffer);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating binding buffer: %d\r\n", status);
	}
	
	status =  uefi_call_wrapper(BS->OpenProtocol, 6,
		ServiceBindingHandle = HandleBuffer[0],
        &Tcp4ServiceBindingProtocol,
        (VOID **) &ServiceBinding,
        gImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in opening binding protocol: %d\r\n", status);
	}
}

extern "C"
void FreeBindingProtocol() {
	EFI_STATUS status = 0;
	
	if (IsServiceBindingInstalled) {
		status = uefi_call_wrapper(BS->UninstallMultipleProtocolInterfaces , 4, &ServiceBindingHandle,
			&Tcp4ServiceBindingProtocol, &ServiceBinding,
			NULL);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError uninstalling binding protocol: %d\r\n", status);
		}
	}
	
	status =  uefi_call_wrapper(BS->CloseProtocol, 4,
                ServiceBindingHandle,
                &Tcp4ServiceBindingProtocol,
                gImageHandle,
                NULL);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError closing binding protocol: %d\r\n", status);
	}
}

class LSocket {
public:
	UINT16 Port;
	EFI_TCP4 *Child = NULL;
	EFI_HANDLE Handle = NULL;
	EFI_IPv4_ADDRESS Address;
	bool AllReceived = false;

	LSocket (EFI_TCP4 *_Child, EFI_HANDLE _Handle) : Child(_Child), Handle(_Handle) {
	}
	
	LSocket(EFI_IPv4_ADDRESS *_Address, UINT16 _Port) : Port (_Port) {
		CopyMem(&this->Address, _Address, sizeof (this->Address));
	}
	
	bool CreateChild() {
		if (!ServiceBinding) {
			return false;
		}
		
		EFI_STATUS status = uefi_call_wrapper(ServiceBinding->CreateChild, 2, ServiceBinding, &Handle);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating child: %d\r\n", status);
		}
		
		status = uefi_call_wrapper(BS->OpenProtocol, 6,
	                   Handle,
	                   &Tcp4Protocol,
	                   (VOID **)&Child,
	                   gImageHandle,
	                   NULL,
	                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
	                   );
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in opening protocol: %d\r\n", status);
		}
		
		return true;
	}
	
	bool Connect(EFI_IPv4_ADDRESS *gRemoteAddress, EFI_IPv4_ADDRESS *gSubnetMask, UINT16 gRemotePort) {
		if (Child == NULL) {
			return false;
		}
		
		EFI_TCP4_CONNECTION_STATE       Tcp4State;
    	EFI_TCP4_CONFIG_DATA            Tcp4ConfigData;
	    EFI_IP4_MODE_DATA               Ip4ModeData;
    	EFI_MANAGED_NETWORK_CONFIG_DATA MnpConfigData;
    	EFI_SIMPLE_NETWORK_MODE         SnpModeData;
		
		EFI_STATUS status = uefi_call_wrapper(this->Child->GetModeData,
			6, this->Child, &Tcp4State, &Tcp4ConfigData, &Ip4ModeData, &MnpConfigData, &SnpModeData);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError getting mode data: %d\r\n", status);
		}
		
		EFI_TCP4_CONFIG_DATA ConfigData;
		EFI_TCP4_ACCESS_POINT *ap = &ConfigData.AccessPoint;

		ZeroMem(&ConfigData, sizeof(ConfigData));
				
		ConfigData.TypeOfService = 0;
	    ap->UseDefaultAddress = TRUE;
		
		CopyMem(&ap->RemoteAddress, gRemoteAddress, sizeof(*gRemoteAddress));
		CopyMem(&ap->SubnetMask, gSubnetMask, sizeof (*gSubnetMask));
		
		ap->StationPort = this->Port;
		ap->RemotePort = gRemotePort;
		ap->ActiveFlag = TRUE;
		
		CopyMem(&ap->StationAddress, &this->Address, sizeof (this->Address));
		ConfigData.ControlOption = Tcp4ConfigData.ControlOption;
		ConfigData.TimeToLive = 800;
		
		status = uefi_call_wrapper(this->Child->Configure, 2, this->Child, &ConfigData);
		
		while (status == EFI_NO_MAPPING) {
			status = uefi_call_wrapper(this->Child->Configure, 2, this->Child, &ConfigData);
			
			int r = 0;
			for (int i = 0; i < 1000000; ++i) {
				r += 10 + i;
			}
		}
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in configuring TCP protocol: %d\r\n", status);
		}
		
		EFI_TCP4_CONNECTION_TOKEN token;
										
		status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) TCPCompletionTokenEvent, &token.CompletionToken, &token.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
		}
		
		status = uefi_call_wrapper(this->Child->Connect, 2, this->Child, &token);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in connecting: %d\r\n", status);
		}
		
		while (TCPEventStatus == -1) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
		}
		
		TCPEventStatus = -1;
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, token.CompletionToken.Event);
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event: %d\r\n", status);
		}
		
		return true;
	}
	
	bool Transmit (CHAR8 *transmitData, UINTN transmitDataLength) {
		if (Child == NULL) {
			return false;
		}
		
		EFI_TCP4_IO_TOKEN iotoken;
    	EFI_TCP4_TRANSMIT_DATA txdata;
		EFI_TCP4_FRAGMENT_DATA *frag;
    	
    	ZeroMem(&txdata, sizeof(txdata));
    	ZeroMem(&iotoken, sizeof(iotoken));    	
    	
    	txdata.DataLength = transmitDataLength;
	    txdata.FragmentCount = 1;
	    frag = &txdata.FragmentTable[0];
	    frag->FragmentLength = transmitDataLength;
	    frag->FragmentBuffer = (void*) transmitData;
	    
	    iotoken.Packet.TxData = &txdata;

		EFI_STATUS status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) TCPCompletionTokenEvent, &iotoken.CompletionToken, &iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
		}
		
		status = uefi_call_wrapper(this->Child->Transmit, 2, this->Child, &iotoken);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError transmitting data: %d\r\n", status);
		}
		
		while (TCPEventStatus == -1) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
		}
		
		TCPEventStatus = -1;
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event: %d\r\n", status);
		}
		
		return true;
	}
	
	bool Receive(CHAR8* databuf, UINTN *databufLength) {
		if (Child == NULL || this->AllReceived) {
			return false;
		}
		
		EFI_TCP4_IO_TOKEN iotoken;
		EFI_TCP4_RECEIVE_DATA rxdata;
		EFI_TCP4_FRAGMENT_DATA *frag;
		
		ZeroMem(&rxdata, sizeof(rxdata));
    	ZeroMem(&iotoken, sizeof(iotoken));
    	
    	EFI_STATUS status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) TCPCompletionTokenEvent, &iotoken.CompletionToken, &iotoken.CompletionToken.Event);
			
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
		}
		
		iotoken.Packet.RxData = &rxdata;
	    rxdata.FragmentCount = 1;
    	rxdata.DataLength = *databufLength;
    	frag = &rxdata.FragmentTable[0];
    	frag->FragmentBuffer = databuf;
    	frag->FragmentLength = *databufLength;
    	
    	status = uefi_call_wrapper(this->Child->Receive, 2, this->Child, &iotoken);
    	
    	if (status == EFI_CONNECTION_FIN) {
    		this->AllReceived = true;
		} else if (EFI_ERROR(status)) {
			Print(L"\r\nError in receiving: %d\r\n", status);
		}
		
		while (TCPEventStatus == -1) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
		}
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event (3): %d\r\n", status);
		}
		
		*databufLength = frag->FragmentLength;
		
		return true;
	}
	
	void Destroy() {
		EFI_STATUS status =  uefi_call_wrapper(BS->CloseProtocol, 4,
	                this->Handle,
	                &Tcp4Protocol,
	                gImageHandle,
	                NULL);	
	}
	
	LSocket* Accept() {
		
	}
};

#endif
