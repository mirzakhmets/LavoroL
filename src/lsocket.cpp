
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

extern "C" EFI_GUID Tcp4Protocol;

extern "C" EFI_GUID Tcp4ServiceBindingProtocol;

extern "C" EFI_GUID gEfiSimpleNetworkProtocolGuid;

extern "C" int TCPEventStatus = 0;

EFI_HANDLE gImageHandle = NULL;

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

VOID EFIAPI TCPCompletionTokenEvent(EFI_EVENT event, VOID *context) {
	(void) event;

	TCPCompletionTokenEventFinish();
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

VOID EFIAPI TCPConnectionAccepted (EFI_EVENT Event, VOID *Context)
{
	EFI_STATUS             status;
	EFI_TCP4_LISTEN_TOKEN  *acceptToken;
	
	(void) Event;
	
	acceptToken = (EFI_TCP4_LISTEN_TOKEN *) Context;
	status = acceptToken->CompletionToken.Status;
	
	if (EFI_ERROR (status)) {
		Print (L"Connection error: %d\n", status);
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
		Print (L"Open TCP connection: %d\n", status);
		return;
	}
	
	TCPAcceptConnection (Child, acceptToken->NewChildHandle);
	
	BS->CloseEvent (Event);
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
