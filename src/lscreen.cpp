
#include <lfont.hpp>
#include <lscreen.hpp>

LScreen *MainScreen = NULL;

EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP = NULL;

extern "C" void InitializeGraphics() {
	EFI_STATUS status = LibLocateProtocol(&GraphicsOutputProtocol, (void **)&GOP);
	
	if (EFI_ERROR(status)) {
		Print(L"Could not locate GOP: %r\n", status);
		
		return;
	}
	
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo;
	
	status = uefi_call_wrapper(GOP->QueryMode, 4, GOP, GOP->Mode->Mode, &SizeOfInfo, &info);
	
	if (EFI_ERROR(status)) {
		Print(L"Could not query GOP: %r\n", status);
		
		return;
	}
	
	LoadFonts();
	
	MainScreen = new LScreen (0, 0, info->HorizontalResolution, info->VerticalResolution);
}
