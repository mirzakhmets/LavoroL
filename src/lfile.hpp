
#ifndef _L_FILE_

#define _L_FILE_

#include <lfs.hpp>

#include <lreader.hpp>
#include <lwriter.hpp>

const UINT64 InvalidPosition = ~0;

class LFile;

class LFileReader : public LReader {
protected:
	virtual bool ReadBuffer();
	
public:
	LFile *File = NULL;
	
	LFileReader(LFile *_File) : File (_File) {
	}
	
	virtual ~LFileReader() {
	}
};

class LFileWriter : public LWriter {
protected:
	virtual void WriteBuffer();
	
public:
	LFile *File = NULL;
	
	LFileWriter(LFile *_File) : File (_File) {
	}
	
	virtual ~LFileWriter() {
	}
};

class LFile {
public:
	LFileReader Reader;
	LFileWriter Writer;
	
	EFI_FILE* Handle = NULL;
	
	LFile(const CHAR16 *Path, EFI_FILE *_Handle, UINT64 Mode, UINT64 Attributes) : Handle (_Handle), Reader (this), Writer (this) {
		if (!Handle && StrLen(Path) > 0) {
			EFI_FILE *Volume = NULL;
			
			EFI_STATUS status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &Volume);
			
			if (EFI_ERROR (status)) {
				Print(L"\r\nError in opening file: %d\r\n", status);
			} else {
				status = uefi_call_wrapper(Volume->Open, 5, Volume, &this->Handle, Path, Mode, Attributes);
				
				if (EFI_ERROR(status)) {
					Print(L"\r\nError in opening file: %d\r\n", status);
				}
			}
			
			if (Volume) {
				uefi_call_wrapper(Volume->Close, 1, Volume);
				
				Volume = NULL;
			}
		}
	}
	
	virtual ~LFile() {
		if (Handle) {
			uefi_call_wrapper(Handle->Close, 1, Handle);
			
			Handle = NULL;
		}
	}
	
	void Delete() {
		if (Handle) {
			EFI_STATUS status = uefi_call_wrapper (Handle->Delete, 1, Handle);
			
			if (EFI_ERROR(status)) {
				Print (L"\r\nError deleting file: %d\r\n", status);
			} else {
				Handle = NULL;
			}
		}
	}
	
	UINT64 Position() {
		UINT64 result = InvalidPosition;
					
		if (Handle) {
			EFI_STATUS status = uefi_call_wrapper (Handle->GetPosition, 2, Handle, &result);
			
			if (EFI_ERROR(status)) {
				result = InvalidPosition;
				
				Print (L"\r\nError getting file position: %d\r\n", status);
			}
		}
		
		return result;
	}
	
	void SetPosition (UINT64 _Position) {
		if (Handle) {
			EFI_STATUS status = uefi_call_wrapper (Handle->SetPosition, 2, Handle, _Position);
			
			if (EFI_ERROR(status)) {
				Print (L"\r\nError getting file position: %d\r\n", status);
			}
		}
	}	
};

bool LFileReader::ReadBuffer() {
	if (File) {
		UINTN BufferSize = sizeof (this->buffer);
		
		EFI_STATUS status = uefi_call_wrapper(File->Handle->Read, 3, File->Handle, &BufferSize, this->buffer);
		
		this->current = 0;
		
		this->size = BufferSize;
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in reading file: %d\r\n", status);
			
			return false;
		}
		
		return this->current != this->size;
	}
	
	return false;
}

void LFileWriter::WriteBuffer() {
	if (File && this->current) {
		UINTN BufferSize = this->current;
		
		EFI_STATUS status = uefi_call_wrapper(File->Handle->Write, 3, File->Handle, &BufferSize, this->buffer);
		
		if (EFI_ERROR(status)) {
			Print(L"\r\nError in reading file: %d\r\n", status);
		}
	}
}

#endif
