
#include <lfile.hpp>

bool LFileReader::ReadBuffer() {
	#ifdef _TEST_
	if (File->FileHandle && !this->AtEnd()) {
		this->current = 0;
		
		this->size = fread (this->buffer, 1, sizeof (this->buffer) - 1, File->FileHandle);
		
		return this->current != this->size;
	}
	#else
	if (File && !this->AtEnd()) {
		UINTN BufferSize = sizeof (this->buffer) - 1;
		
		EFI_STATUS status = uefi_call_wrapper(File->Handle->Read, 3, File->Handle, &BufferSize, this->buffer);
		
		this->current = 0;
		
		this->size = BufferSize;
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in reading file: %d\r\n", status);
			
			return false;
		}
		
		return this->current != this->size;
	}
	#endif
	
	return false;
}

void LFileWriter::WriteBuffer() {
	#ifdef _TEST_
	if (this->current) {
		fwrite (this->buffer, 1, this->current, this->File->FileHandle);
	}
	#else
	if (File && this->current) {
		UINTN BufferSize = this->current;
		
		EFI_STATUS status = uefi_call_wrapper(File->Handle->Write, 3, File->Handle, &BufferSize, this->buffer);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in reading file: %d\r\n", status);
		}
	}
	#endif
}
