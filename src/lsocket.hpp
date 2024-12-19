
#ifndef _L_SOCKET_

#define _L_SOCKET_

#include <ltask.hpp>

#include <lreader.hpp>
#include <lwriter.hpp>

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

#define IP4_ADDR_TO_STRING(IpAddr, IpAddrString) UnicodeSPrint (       \
                                                   IpAddrString,       \
                                                   16 * 2,             \
                                                   L"%d.%d.%d.%d",     \
                                                   IpAddr.Addr[0],     \
                                                   IpAddr.Addr[1],     \
                                                   IpAddr.Addr[2],     \
                                                   IpAddr.Addr[3]      \
                                                   );

class LSocket;

class LSocketReader : public LReader {
protected:
	virtual bool ReadBuffer();
	
public:
	LSocket *Socket = NULL;
	
	LSocketReader(LSocket *_Socket) : Socket (_Socket) {
	}
	
	virtual ~LSocketReader() {
	}
};

class LSocketWriter : public LWriter {
protected:
	virtual void WriteBuffer();
	
public:
	LSocket *Socket = NULL;
	
	LSocketWriter(LSocket *_Socket) : Socket (_Socket) {
	}
	
	virtual ~LSocketWriter() {
	}
};

class LSocket {
public:
	UINT16 Port;
	EFI_TCP4 *Child = NULL;
	EFI_HANDLE Handle = NULL;
	EFI_IPv4_ADDRESS Address;
	bool AllReceived = false;
	
	LSocketReader Reader;
	LSocketWriter Writer;
	
	LSocket (EFI_TCP4 *_Child, EFI_HANDLE _Handle) : Child(_Child), Handle(_Handle), Reader(this), Writer(this) {
	}
	
	LSocket(EFI_IPv4_ADDRESS *_Address, UINT16 _Port) : Port (_Port), Reader(this), Writer(this) {
		CopyMem(&this->Address, _Address, sizeof (this->Address));
		
		this->Initialize();
	}
	
	~LSocket() {
		/*
		EFI_STATUS status =  uefi_call_wrapper(BS->CloseProtocol, 4,
	                this->Handle,
	                &Tcp4Protocol,
	                gImageHandle,
	                NULL);
	    */
	    
	    this->DestroyChild();
	}
	
	void Initialize() {
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
		
		EFI_TCP4_CONFIG_DATA TcpConfigData = {
		    0x00,                                           // IPv4 Type of Service
		    255,                                            // IPv4 Time to Live
		    {                                               // AccessPoint:
		      TRUE,                                         // Use default address
		      { {Address.Addr[0], Address.Addr[1], Address.Addr[2], Address.Addr[3]} },
		      //{ {0, 0, 0, 0} },                             // IP Address  (ignored - use default)
		      //{ {0, 0, 0, 0} },                             // Subnet mask (ignored - use default)
		      { {255, 255, 255, 0} },                             // Subnet mask (ignored - use default)
		      80,                             				// Station port
		      { {0, 0, 0, 0} },                             // Remote address: accept any
		      0,                                            // Remote Port: accept any
		      FALSE                                         // ActiveFlag: be a "server"
		    },
		    NULL                                            // Default advanced TCP options
		  };
		
		EFI_IP4_MODE_DATA               Ip4ModeData;
		
		status = uefi_call_wrapper(Child->Configure, 2, Child, &TcpConfigData);
		
		if (status == EFI_NO_MAPPING) {
			do {
		      status = uefi_call_wrapper(Child->GetModeData, 6,
			  	Child,
		        NULL, NULL,
		        &Ip4ModeData,
		        NULL, NULL
		    	);
		    } while (!Ip4ModeData.IsConfigured);
			
		    status = uefi_call_wrapper(Child->Configure, 2, Child, &TcpConfigData);
		} else if (EFI_ERROR (status)) {
		    Print (L"\r\nTCP Configure: %d\r\n", status);
		}
		
		CHAR16                          IpAddrString[16];
		
		IP4_ADDR_TO_STRING (Ip4ModeData.ConfigData.StationAddress, IpAddrString);
		
		Print( L"TCP transport configured.\r\n");
		Print (L"IP address: ");
		Print (L"%s\r\n", IpAddrString);
		Print (L"\r\n");
	}
	
	LSocket* CreateChild() {
		EFI_TCP4 *ChildTCP4 = NULL;
		
		EFI_HANDLE ChildHandle = NULL;		
		
		EFI_STATUS status = uefi_call_wrapper(ServiceBinding->CreateChild, 2, ServiceBinding, &ChildHandle);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating child: %d\r\n", status);
		}
		
		status = uefi_call_wrapper(BS->OpenProtocol, 6,
	                   ChildHandle,
	                   &Tcp4Protocol,
	                   (VOID **)&ChildTCP4,
	                   gImageHandle,
	                   NULL,
	                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
	                   );
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in opening protocol: %d\r\n", status);
		}
		
		LSocket *Socket = new LSocket(ChildTCP4, ChildHandle);
		
		Socket->Port = this->Port;
		
		CopyMem (&Socket->Address, &this->Address, sizeof (this->Address));
		
		return Socket;
	}
	
	void DestroyChild() {
		if (!ServiceBinding || !Child || !Handle) {
			return;
		}
		
		EFI_STATUS status = uefi_call_wrapper(ServiceBinding->DestroyChild, 2, ServiceBinding, Handle);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating child: %d\r\n", status);
		}
		
		Child = NULL;
		
		Handle = NULL;
	}
	
	bool Connect(EFI_IPv4_ADDRESS *gRemoteAddress, EFI_IPv4_ADDRESS *gSubnetMask, UINT16 gRemotePort) {
		if (Child == NULL) {
			return false;
		}
		
		EFI_TCP4_CONFIG_DATA TcpConfigData = {
		    0x00,                                           // IPv4 Type of Service
		    255,                                            // IPv4 Time to Live
		    {                                               // AccessPoint:
		      TRUE,                                         // Use default address
		      { {Address.Addr[0], Address.Addr[1], Address.Addr[2], Address.Addr[3]} },                             // IP Address  (ignored - use default)
		      { {gSubnetMask->Addr[0], gSubnetMask->Addr[1], gSubnetMask->Addr[2], gSubnetMask->Addr[3]} },                             // Subnet mask (ignored - use default)
		      this->Port,                             				// Station port
		      { {gRemoteAddress->Addr[0], gRemoteAddress->Addr[1], gRemoteAddress->Addr[2], gRemoteAddress->Addr[3]} },                             // Remote address: accept any
		      gRemotePort,                                            // Remote Port: accept any
		      TRUE                                         // ActiveFlag: be a "server"
		    },
		    NULL                                            // Default advanced TCP options
		  };
		
		EFI_STATUS status = 0;
		
		EFI_IP4_MODE_DATA               Ip4ModeData;

		status = uefi_call_wrapper(Child->Configure, 2, Child, &TcpConfigData);

	    do {		    	
	      status = uefi_call_wrapper(Child->GetModeData, 6,
		  	Child,
	        NULL, NULL,
	        &Ip4ModeData,
	        NULL, NULL
	    	);
	    	
	    	DoEvents();
	    } while (!Ip4ModeData.IsConfigured);
		
		if (EFI_ERROR (status)) {
			Print (L"\r\nTCP Configure (2): %d\r\n", status);
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
			
			DoEvents();
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
			
			DoEvents();
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
			
			DoEvents();
		}
		
		if (EFI_ERROR(iotoken.CompletionToken.Status)) {
			*databufLength = 0;
		} else {
			*databufLength = frag->FragmentLength;
		}
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event (3): %d\r\n", status);
		}
		
		return *databufLength;
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
};

bool LSocketReader::ReadBuffer() {
	if (Socket) {
		UINTN BufferSize = sizeof (this->buffer);
		
		if (Socket->AllReceived) {
			this->size = ~0U;
		} else if (!Socket->Receive(this->buffer, &BufferSize)) {
			this->size = ~0U;
		} else {					
			this->current = 0;
			
			this->size = BufferSize;
		}
	}
	
	return this->size != ~0U;
}

void LSocketWriter::WriteBuffer() {
	if (Socket && this->current) {
		Socket->Transmit(this->buffer, this->current);
	}
}

#endif
