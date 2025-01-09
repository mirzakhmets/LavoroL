
#ifndef _L_SOCKET_

#define _L_SOCKET_

#include <ltask.hpp>

#include <lreader.hpp>
#include <lwriter.hpp>

extern "C" EFI_GUID Tcp4Protocol;

extern "C" EFI_GUID Tcp4ServiceBindingProtocol;

extern "C" EFI_GUID gEfiSimpleNetworkProtocolGuid;

extern "C" EFI_HANDLE gImageHandle;

extern "C" EFI_HANDLE SimpleNetworkProtocolHandle;
extern "C" EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetworkProtocolInterface;

extern "C" EFI_HANDLE ServiceBindingHandle;
extern "C" EFI_SERVICE_BINDING *ServiceBinding;
extern "C" bool IsServiceBindingInstalled;

extern "C" void TCPCompletionTokenEventStart();

extern "C" void TCPCompletionTokenEventFinish();

extern "C" bool TCPCompletionTokenEventRunning();

extern VOID EFIAPI TCPCompletionTokenEvent(EFI_EVENT event, VOID *context);

extern "C" void TCPConnectionAcceptInitialize();

extern VOID EFIAPI TCPConnectionAccepted (EFI_EVENT Event, VOID *Context);

extern "C" int TCPAcceptStatus;

extern "C" void InitializeNetworkProtocol();

extern "C" void FreeNetworkProtocol();

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

extern "C" int TCPEventStatus;

extern "C" EFI_HANDLE gImageHandle;

extern "C" EFI_GUID Tcp4Protocol;

extern "C" EFI_GUID Tcp4ServiceBindingProtocol;

class LSocket {
protected:
	EFI_HANDLE ServiceBindingHandle = NULL;
	EFI_SERVICE_BINDING *ServiceBinding = NULL;
	
	bool Closed = false;
	
	EFI_TCP4_IO_TOKEN iotoken;
	EFI_TCP4_RECEIVE_DATA rxdata;
	EFI_TCP4_TRANSMIT_DATA txdata;
	EFI_TCP4_CONNECTION_TOKEN token;
	EFI_TCP4_FRAGMENT_DATA *frag = NULL;
public:
	UINT16 Port;
	EFI_IPv4_ADDRESS Address;

	EFI_TCP4 *Child = NULL;
	EFI_HANDLE Handle = NULL;
	
	LSocketReader Reader;
	LSocketWriter Writer;
	
	LSocket (EFI_IPv4_ADDRESS *_Address, UINT16 _Port) : Reader(this), Writer(this), Port (_Port) {
		CopyMem(&this->Address, _Address, sizeof (this->Address));
		
		EFI_HANDLE protocol, child, *handles = NULL;
		UINTN i, nr_handles = 0;
		EFI_STATUS status = LibLocateHandle(ByProtocol, &Tcp4ServiceBindingProtocol, NULL, &nr_handles, &handles);
		
		for (i = 0; i < nr_handles; i++) {
			status = uefi_call_wrapper(BS->OpenProtocol, 6, handles[i],
						   &Tcp4ServiceBindingProtocol, (void **)&ServiceBinding,
						   gImageHandle, handles[i],
						   EFI_OPEN_PROTOCOL_GET_PROTOCOL);
			
			if (status == EFI_SUCCESS) {
				ServiceBindingHandle = handles[i];
				break;
			}
			
			uefi_call_wrapper(BS->CloseProtocol, 4, handles[i], &Tcp4ServiceBindingProtocol,
				  gImageHandle, handles[i]);
	    }
		
		if (!ServiceBinding) {
			status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 4, &ServiceBindingHandle,
				&Tcp4ServiceBindingProtocol, &ServiceBinding,
				NULL);
	
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in installing binding protocol: %d\r\n", status);
			}
		}
	
		status = uefi_call_wrapper(ServiceBinding->CreateChild, 2, ServiceBinding, (EFI_HANDLE*)&Handle);
	
		if (status != EFI_SUCCESS) {
			uefi_call_wrapper(BS->CloseProtocol, 4, handles[i], &Tcp4Protocol,
			      gImageHandle, handles[i]);
			
			goto END_MARK;
		}
	
		status = uefi_call_wrapper(BS->OpenProtocol, 6, Handle,
				      &Tcp4Protocol, (void **)&Child,
				      gImageHandle, ServiceBinding,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);

	    if (status != EFI_SUCCESS) {
	    	uefi_call_wrapper(ServiceBinding->DestroyChild, 2, ServiceBinding, Handle);
	    	
	    	goto END_MARK;
		}
		
		END_MARK:
			;
	}
	
	LSocket (EFI_TCP4 *_Child, EFI_HANDLE _Handle) : Child (_Child), Handle (_Handle),
		Reader(this), Writer(this) {
	}
	
	virtual ~LSocket() {
		if (Handle) {
			uefi_call_wrapper(BS->CloseProtocol, 4, Handle, &Tcp4Protocol,
				gImageHandle, Handle);
			
			uefi_call_wrapper(ServiceBinding->DestroyChild, 2, ServiceBinding, Handle);
		}
		
		if (ServiceBindingHandle) {
			uefi_call_wrapper(BS->CloseProtocol, 4, ServiceBindingHandle, &Tcp4ServiceBindingProtocol,
				gImageHandle, ServiceBindingHandle);
		}
		
		if (Child) {
			Child->Close(Child, NULL);
		}
	}
	
	bool Connect(EFI_IPv4_ADDRESS *gRemoteAddress, UINT16 gRemotePort) {
		if (Child == NULL) {
			return false;
		}
		
		ZeroMem(&token, sizeof(token));
		
		EFI_TCP4_CONFIG_DATA TcpConfigData = {
		    0x00,                                           // IPv4 Type of Service
		    255,                                            // IPv4 Time to Live
		    {                                               // AccessPoint:
		      TRUE,                                         // Use default address
		      {
		      	{
		      		//this->Address.Addr[0], this->Address.Addr[1], this->Address.Addr[2], this->Address.Addr[3]
		      		0, 0, 0, 0
				  }
			  },
			  {
		      	{
		      		//255, 255, 255, 255
		      		0, 0, 0, 0
				  }
			  },
		      this->Port,                             				// Station port
		      { {gRemoteAddress->Addr[0], gRemoteAddress->Addr[1], gRemoteAddress->Addr[2], gRemoteAddress->Addr[3]} },                             // Remote address: accept any
		      gRemotePort,                                            // Remote Port: accept any
		      TRUE                                         // ActiveFlag: be a "server"
		    },
		    NULL                                            // Default advanced TCP options
		  };
		
		EFI_STATUS status = 0;
		
		EFI_IP4_MODE_DATA               Ip4ModeData;
		
		status = Child->Configure (Child, &TcpConfigData);
		  
		  if (status == EFI_NO_MAPPING) {
		    // Wait until the IP configuration process (probably DHCP) has finished
		    do {
		      status = Child->GetModeData (Child,
		                               NULL, NULL,
		                               &Ip4ModeData,
		                               NULL, NULL
		                               );
		        
		    	if (status == EFI_ACCESS_DENIED) {
					Print (L"Access denied\r\n");
					
					return false;
				} else if (EFI_ERROR (status)) {
					break;
				}
		    } while (!Ip4ModeData.IsConfigured);
		    
			status = Child->Configure (Child, &TcpConfigData);
		  } else if (EFI_ERROR (status)) {
		    Print (L"TCP configure: %r\r\n", status);
		  }

		//status = Child->Configure (Child, &TcpConfigData);
		
		if (status == EFI_ACCESS_DENIED) {
			Print (L"Access denied\r\n");
			
			return false;
		}

		status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) TCPCompletionTokenEvent, &token.CompletionToken, &token.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
		
			return false;
		}
		
		TCPCompletionTokenEventStart();
		
		status = uefi_call_wrapper(this->Child->Connect, 2, this->Child, &token);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in connecting: %d\r\n", status);
			
			uefi_call_wrapper(BS->CloseEvent, 1, token.CompletionToken.Event);
			
			return false;
		}
		
		int Counter = 0;
		
		while (TCPCompletionTokenEventRunning()) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
				
				uefi_call_wrapper(BS->CloseEvent, 1, token.CompletionToken.Event);
				
				return false;
			}
			
			DoEvents();
			
			if (EFI_ERROR(token.CompletionToken.Status)) {
				TCPCompletionTokenEventFinish();
			} else if (++Counter > 1000000) {
				TCPCompletionTokenEventFinish();
			}
		}
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, token.CompletionToken.Event);
	
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event: %d\r\n", status);
			
			return false;
		}
		
		return true;
	}
	
	bool Transmit (CHAR8 *transmitData, UINTN transmitDataLength) {
		if (Child == NULL) {
			return false;
		}
    	
    	ZeroMem(&txdata, sizeof(txdata));
    	ZeroMem(&iotoken, sizeof(iotoken));    	
    	
    	txdata.DataLength = transmitDataLength;
	    txdata.FragmentCount = 1;
	    txdata.Push = TRUE;
	    txdata.Urgent = FALSE;
		frag = &txdata.FragmentTable[0];
	    frag->FragmentLength = transmitDataLength;
	    frag->FragmentBuffer = (void*) transmitData;
	    
	    iotoken.Packet.TxData = &txdata;

		EFI_STATUS status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) TCPCompletionTokenEvent, &iotoken.CompletionToken, &iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
			
			return false;
		}
		
		TCPCompletionTokenEventStart();
		
		status = uefi_call_wrapper(this->Child->Transmit, 2, this->Child, &iotoken);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError transmitting data: %d\r\n", status);
			
			uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
			
			return false;
		}
		
		int Counter = 0;
		
		while (TCPCompletionTokenEventRunning()) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
				
				uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
				
				return false;
			}
			
			if (EFI_ERROR(iotoken.CompletionToken.Status)) {
				TCPCompletionTokenEventFinish();
			} else if (++Counter > 1000000) {
				TCPCompletionTokenEventFinish();
			}
			
			DoEvents();
		}
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event: %d\r\n", status);
			
			return false;
		}
		
		return true;
	}
	
	bool Receive(CHAR8* databuf, UINTN *databufLength) {
		if (Child == NULL || this->Closed) {
			return false;
		}
		
		ZeroMem(&rxdata, sizeof(rxdata));
    	ZeroMem(&iotoken, sizeof(iotoken));

		TCPCompletionTokenEventStart();
    	
    	EFI_STATUS status = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK, (EFI_EVENT_NOTIFY) TCPCompletionTokenEvent, &TCPEventStatus, &iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in creating event: %d\r\n", status);
			
			return false;
		}
		
		iotoken.Packet.RxData = &rxdata;
		rxdata.UrgentFlag = FALSE;
	    rxdata.FragmentCount = 1;
    	rxdata.DataLength = *databufLength;
    	frag = &rxdata.FragmentTable[0];
    	frag->FragmentBuffer = databuf;
    	frag->FragmentLength = *databufLength;
    	
    	status = uefi_call_wrapper(this->Child->Receive, 2, this->Child, &iotoken);
    	
    	if (status == EFI_CONNECTION_FIN) {
    		uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
    		
    		*databufLength = frag->FragmentLength;
    		
    		this->Closed = true;
    		
    		return *databufLength;
		} else if (EFI_ERROR(status)) {
			Print(L"\r\nError in receiving: %d\r\n", status);
			
			uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
			
			return false;
		}
		
		int Counter = 0;
		
		while (TCPCompletionTokenEventRunning()) {
			status = uefi_call_wrapper(this->Child->Poll, 1, this->Child);
			
			if (EFI_ERROR(status)) {
				Print(L"\r\nError in polling: %d\r\n", status);
				
				uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
				
				return false;
			}
			
			DoEvents();
			
			if (EFI_ERROR(iotoken.CompletionToken.Status)) {
				frag->FragmentLength = 0;
				
				TCPCompletionTokenEventFinish();
			} else if (++Counter > 1000000) {
				frag->FragmentLength = 0;
				
				TCPCompletionTokenEventFinish();
			}
		}

		if (EFI_ERROR(iotoken.CompletionToken.Status)) {
			*databufLength = 0;
			
			uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
			
			return false;
		} else {
			*databufLength = frag->FragmentLength;
		}
		
		status = uefi_call_wrapper(BS->CloseEvent, 1, iotoken.CompletionToken.Event);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in closing event (3): %d\r\n", status);
			
			return false;
		}
		
		return *databufLength;
	}
	
	bool Accept() {
		if (Child == NULL) {
			return false;
		}
		
		ZeroMem(&TCPConnectionAcceptToken, sizeof(TCPConnectionAcceptToken));
		
		EFI_TCP4_CONFIG_DATA TcpConfigData = {
		    0x00,                                           // IPv4 Type of Service
		    255,                                            // IPv4 Time to Live
		    {                                               // AccessPoint:
		      TRUE,                                         // Use default address
		      {
		      	{
		      		//this->Address.Addr[0], this->Address.Addr[1], this->Address.Addr[2], this->Address.Addr[3]
		      		0, 0, 0, 0
				  }
			  },
			  {
		      	{
		      		//255, 255, 255, 255
		      		0, 0, 0, 0
				  }
			  },
		      this->Port,                             				// Station port
		      { {0, 0, 0, 0} },                             // Remote address: accept any
		      0,                                            // Remote Port: accept any
		      FALSE                                         // ActiveFlag: be a "server"
		    },
		    NULL                                            // Default advanced TCP options
		  };
		
		EFI_STATUS status = 0;
		
		EFI_IP4_MODE_DATA               Ip4ModeData;
		
		  status = Child->Configure (Child, &TcpConfigData);
		  
		  if (status == EFI_NO_MAPPING) {
		    // Wait until the IP configuration process (probably DHCP) has finished
		    do {
		      status = Child->GetModeData (Child,
		                               NULL, NULL,
		                               &Ip4ModeData,
		                               NULL, NULL
		                               );
		        
		        //if (status == EFI_ACCESS_DENIED) {
				//	Print (L"Access denied\r\n");
					
				//	return false;
				//} else if (EFI_ERROR (status)) {
				//	break;
				//}
		    } while (!Ip4ModeData.IsConfigured);
		    
			status = Child->Configure (Child, &TcpConfigData);
		  } else if (EFI_ERROR (status)) {
		    Print (L"TCP configure: %r\r\n", status);
		  }

		if (status == EFI_ACCESS_DENIED) {
			Print (L"Access denied\r\n");
			
			return false;
		}
		
		status = Child->GetModeData (Child,
		                               NULL, NULL,
		                               &Ip4ModeData,
		                               NULL, NULL
		                               );
		                               
		  CHAR16                          IpAddrString[16];

		IP4_ADDR_TO_STRING (Ip4ModeData.ConfigData.StationAddress, IpAddrString);

		Print (L"\r\nTCP transport configured.\r\n");
  		Print (L"IP address: %s\r\n", IpAddrString);
		
		TCPConnectionAcceptInitialize();
		
		status = uefi_call_wrapper(this->Child->Accept, 2, this->Child, &TCPConnectionAcceptToken);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in accepting event: %d\r\n", status);
			
			return false;
		}
		
		return true;
	}
};

bool LSocketReader::ReadBuffer() {
	if (Socket && !this->AtEnd()) {
		UINTN BufferSize = ReaderBufferSize - 16;
		
		if (!Socket->Receive(this->buffer, &BufferSize)) {
			this->size = ~0U;
			
			return false;
		} else {
			this->current = 0;
			
			this->size = BufferSize;
		}
		
		return this->current != this->size;
	}
	
	return false;
}

void LSocketWriter::WriteBuffer() {
	if (Socket && this->current) {
		Socket->Transmit(this->buffer, this->current);
	}
}

#endif
