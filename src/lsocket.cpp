
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

extern "C" EFI_GUID Tcp4Protocol;

extern "C" EFI_GUID Tcp4ServiceBindingProtocol;

extern "C" EFI_GUID gEfiSimpleNetworkProtocolGuid;

EFI_HANDLE gImageHandle = NULL;

int volatile TCPEventStatus = 0;

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

EFI_HANDLE SimpleNetworkProtocolHandle = NULL;
EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetworkProtocolInterface = NULL;

extern "C"
void InitializeNetworkProtocol() {
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
	
	#ifndef _TEST_
	TCPAcceptConnection (Child, acceptToken->NewChildHandle);
	#endif
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

