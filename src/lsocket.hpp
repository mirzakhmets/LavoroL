
#ifndef _L_SOCKET_

#define _L_SOCKET_

extern "C" EFI_GUID Tcp4Protocol;

extern "C" EFI_GUID Tcp4ServiceBindingProtocol;

extern "C" EFI_GUID gEfiSimpleNetworkProtocolGuid;

extern "C" EFI_HANDLE gImageHandle;

extern "C" int TCPEventStatus;

extern "C" EFI_HANDLE SimpleNetworkProtocolHandle;
extern "C" EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetworkProtocolInterface;

extern "C" EFI_HANDLE ServiceBindingHandle;
extern "C" EFI_SERVICE_BINDING *ServiceBinding;
extern "C" bool IsServiceBindingInstalled;

extern "C" void TCPCompletionTokenEventStart();

extern "C" void TCPCompletionTokenEventFinish();

extern "C" bool TCPCompletionTokenEventRunning();

extern "C" void TCPCompletionTokenEvent(EFI_EVENT event, void *context);

extern "C" void TCPConnectionAcceptInitialize();

extern "C" void TCPConnectionAccepted (EFI_EVENT Event, VOID *Context);

extern "C" void InitializeNetworkProtocol();

extern "C" void FreeNetworkProtocol();

extern "C" void InitializeBindingProtocol();

extern "C" void FreeBindingProtocol();

extern "C" EFI_TCP4_LISTEN_TOKEN TCPConnectionAcceptToken;

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
		
		/*
		EFI_STATUS status = uefi_call_wrapper(this->Child->GetModeData,
			6, this->Child, &Tcp4State, &Tcp4ConfigData, &Ip4ModeData, &MnpConfigData, &SnpModeData);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError getting mode data: %d\r\n", status);
		}
		*/
		
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
		
		EFI_STATUS status = uefi_call_wrapper(this->Child->Configure, 2, this->Child, &ConfigData);
		
		while (status == EFI_NO_MAPPING) {
			status = uefi_call_wrapper(this->Child->Configure, 2, this->Child, &ConfigData);
			
			int r = 0;
			for (int i = 0; i < 1000000; ++i) {
				r += 10 + i;
			}
			
			// Do events
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
		
		TCPCompletionTokenEventStart();
		
		status = uefi_call_wrapper(this->Child->Connect, 2, this->Child, &token);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in connecting: %d\r\n", status);
		}
		
		while (TCPCompletionTokenEventRunning()) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
			
			// Do events
		}
		
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
		
		TCPCompletionTokenEventStart();
		
		status = uefi_call_wrapper(this->Child->Transmit, 2, this->Child, &iotoken);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError transmitting data: %d\r\n", status);
		}
		
		while (TCPCompletionTokenEventRunning()) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
			
			// Do events
		}
		
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

		TCPCompletionTokenEventStart();
    	
    	status = uefi_call_wrapper(this->Child->Receive, 2, this->Child, &iotoken);
    	
    	if (status == EFI_CONNECTION_FIN) {
    		this->AllReceived = true;
		} else if (EFI_ERROR(status)) {
			Print(L"\r\nError in receiving: %d\r\n", status);
		}
		
		while (TCPCompletionTokenEventRunning()) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
			}
			
			// Do events
		}
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event (3): %d\r\n", status);
		}
		
		*databufLength = frag->FragmentLength;
		
		return true;
	}
	
	bool Accept() {
		if (Child == NULL) {
			return false;
		}
		
		EFI_STATUS status =  uefi_call_wrapper(this->Child->Accept, 2, this->Child, &TCPConnectionAcceptToken);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in accepting event: %d\r\n", status);
			
			return false;
		}
		
		return true;
	}
	
	void Destroy() {
		EFI_STATUS status =  uefi_call_wrapper(BS->CloseProtocol, 4,
	                this->Handle,
	                &Tcp4Protocol,
	                gImageHandle,
	                NULL);	
	}
};

#endif
