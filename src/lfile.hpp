
#ifndef _L_FILE_

#define _L_FILE_

#include <lfs.hpp>

#include <lreader.hpp>
#include <lwriter.hpp>

#ifdef _TEST_
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;
#endif

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
	
	#ifdef _TEST_
	FILE *FileHandle = NULL;
	char FileName[MAX_PATH], FileMode[MAX_PATH];
	#endif
	
	LFile(const CHAR16 *Path, EFI_FILE *_Handle, UINT64 Mode, UINT64 Attributes) : Handle (_Handle), Reader (this), Writer (this) {
		#ifdef _TEST_
		int i;
		
		for (i = 0; Path[i]; ++i) {
			FileName[i] = Path[i];
		}
		
		FileName[i] = '\0';
		
		FileMode[0] = '\0';
		
		if (Mode & EFI_FILE_MODE_READ) {
			strcat (FileMode, "r");
		}

		if (Mode & EFI_FILE_MODE_WRITE) {
			strcat (FileMode, "w");
		}
		
		strcat (FileMode, "b");
		
		this->FileHandle = fopen (FileName, FileMode);
		#else
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
		#endif
	}
	
	virtual ~LFile() {
		#ifdef _TEST_
		if (FileHandle) {
			fclose (FileHandle);
		}
		#else
		if (Handle) {
			uefi_call_wrapper(Handle->Close, 1, Handle);
			
			Handle = NULL;
		}
		#endif
	}
	
	void Delete() {
		#ifdef _TEST_
		remove (FileName);
		#else
		if (Handle) {
			EFI_STATUS status = uefi_call_wrapper (Handle->Delete, 1, Handle);
			
			if (EFI_ERROR(status)) {
				Print (L"\r\nError deleting file: %d\r\n", status);
			} else {
				Handle = NULL;
			}
		}
		#endif
	}
	
	UINT64 Position() {
		#ifdef _TEST_
		return ftell (FileHandle);
		#else
		UINT64 result = InvalidPosition;
		
		if (Handle) {
			EFI_STATUS status = uefi_call_wrapper (Handle->GetPosition, 2, Handle, &result);
			
			if (EFI_ERROR(status)) {
				result = InvalidPosition;
				
				Print (L"\r\nError getting file position: %d\r\n", status);
			}
		}
		
		return result;
		#endif
	}
	
	void SetPosition (UINT64 _Position) {
		this->Reader.Reset();
		this->Writer.Flush();
		this->Writer.Reset();

		#ifdef _TEST_
		fsetpos (FileHandle, &_Position);
		#else
		if (Handle) {
			EFI_STATUS status = uefi_call_wrapper (Handle->SetPosition, 2, Handle, _Position);
			
			if (EFI_ERROR(status)) {
				Print (L"\r\nError getting file position: %d\r\n", status);
			}
		}
		#endif		
	}
};

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

#endif
