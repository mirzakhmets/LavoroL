
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

extern EFI_GUID Tcp4Protocol;

extern EFI_GUID Tcp4ServiceBindingProtocol;

extern EFI_GUID gEfiSimpleNetworkProtocolGuid;

extern "C" EFI_HANDLE gImageHandle = NULL;

extern "C" int volatile TCPEventStatus = 0;

extern "C" void TCPCompletionTokenEventStart() {
	++TCPEventStatus;
}

extern "C" void TCPCompletionTokenEventFinish() {
	if (TCPEventStatus) {
		--TCPEventStatus;
	}
}

extern "C" bool TCPCompletionTokenEventRunning() {
	return TCPEventStatus;
}

extern "C" 
EFIAPI void TCPCompletionTokenEvent(EFI_EVENT event, void *context) {
	EFI_TCP4_COMPLETION_TOKEN *token = context;

	(void) event;

	if (token->Status == EFI_SUCCESS) {
		TCPCompletionTokenEventFinish();
	} else {
		TCPCompletionTokenEventFinish();
	}
}

extern "C" EFI_HANDLE SimpleNetworkProtocolHandle = NULL;
extern "C" EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetworkProtocolInterface = NULL;

extern "C"
void InitializeNetworkProtocol() {
	EFI_STATUS status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &gEfiSimpleNetworkProtocolGuid, NULL, (VOID**)&SimpleNetworkProtocolInterface);

	if (EFI_ERROR(status)) {
		Print(L"\r\nSimple protocol not located: %d\r\n", status);
	}
	
	if (!SimpleNetworkProtocolInterface) {
		EFI_STATUS status = uefi_call_wrapper(BS->InstallMultipleProtocolInterfaces, 4, &SimpleNetworkProtocolHandle,
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
	
	SimpleNetworkProtocolHandle = NULL;
	SimpleNetworkProtocolInterface = NULL;
}

extern "C" EFI_HANDLE ServiceBindingHandle = NULL;
extern "C" EFI_SERVICE_BINDING *ServiceBinding = NULL;
extern "C" bool IsServiceBindingInstalled = false;

extern "C" 
void InitializeBindingProtocol() {
	EFI_STATUS status = uefi_call_wrapper(
		ST->BootServices->LocateProtocol,
		3, &Tcp4ServiceBindingProtocol, NULL, (VOID**)&ServiceBinding);

	if (EFI_ERROR(status)) {
		Print(L"\r\nError in locating protocol: %d\r\n", status);
	}
	
	//if (SimpleNetworkProtocolHandle) {
	//	ServiceBindingHandle = SimpleNetworkProtocolHandle;
	//}
	
	//ServiceBindingHandle = NULL;
	//ServiceBindingHandle = SimpleNetworkProtocolHandle;
	
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
	
	/*
	status =  uefi_call_wrapper(BS->CloseProtocol, 4,
                ServiceBindingHandle,
                &Tcp4ServiceBindingProtocol,
                gImageHandle,
                NULL);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError closing binding protocol: %d\r\n", status);
	}
	*/
	
	if (IsServiceBindingInstalled) {
		status = uefi_call_wrapper(BS->UninstallMultipleProtocolInterfaces , 4, &ServiceBindingHandle,
			&Tcp4ServiceBindingProtocol, &ServiceBinding,
			NULL);

		if (EFI_ERROR(status)) {
			Print(L"\r\nError uninstalling binding protocol: %d\r\n", status);
		}
	}
	
	ServiceBinding = NULL;
	ServiceBindingHandle = NULL;
	IsServiceBindingInstalled = false;
}

EFI_TCP4_LISTEN_TOKEN TCPConnectionAcceptToken;

extern "C" void TCPAcceptConnection(EFI_TCP4 *, EFI_HANDLE);

extern "C" 
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
	
	TCPAcceptConnection (Child, acceptToken->NewChildHandle);
}

extern "C"
void TCPConnectionAcceptInitialize() {
	EFI_STATUS status = uefi_call_wrapper(BS->CreateEvent, 5,
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TCPConnectionAccepted,
                  &TCPConnectionAcceptToken,
                  &TCPConnectionAcceptToken.CompletionToken.Event
                );
    
    if (EFI_ERROR(status)) {
		Print(L"\r\nError creating accept event: %d\r\n", status);
	}
}

