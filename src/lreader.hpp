
#ifndef _L_READER_

#define _L_READER_

#include <lfs.hpp>

const int ReaderEof = -1;

const unsigned long ReaderBufferSize = 128;

class LReader {
protected:
	bool IsText = false;
	unsigned long size = 0;
	unsigned long current = 0;
	
	unsigned char buffer[ReaderBufferSize];
	
	virtual bool ReadBuffer() {
		return true;
	}
public:
	LReader() {
	}
	
	~LReader() {
	}
	
	void Reset() {
		if (!this->AtEnd()) {
			this->current = this->size = 0;
		}
	}
	
	void SetText() {
		this->IsText = true;
	}
	
	bool AtEnd() {
		return size == ~0U;
	}
	
	int Current() {
		if (this->AtEnd()) {
			return ReaderEof;
		}
		
		if (this->IsText && !this->buffer[current]) {
			this->size = ~0U;
			
			return ReaderEof;
		}
		
		if (this->current == this->size) {
			this->Next();
		}
		
		return this->buffer[current];
	}
	
	int Next() {
		if (this->AtEnd()) {
			return ReaderEof;
		}
		
		if (this->current != this->size) {
			this->current++;
		} else {
			if (!this->ReadBuffer()) {
				this->size = ~0U;
				
				return ReaderEof;
			} else {
				this->current = 0;
				
				if (this->current == this->size) {
					this->size = ~0U;
					
					return ReaderEof;
				}
			}
		}
		
		return this->Current();
	}
	
	unsigned Read (void *Buffer, unsigned BufferSize) {
		unsigned result = 0;
		char *CharBuffer = (char *) Buffer;
		
		for (; !this->AtEnd() && BufferSize; --BufferSize, ++CharBuffer, ++result) {
			*CharBuffer = (char) this->Current();
			
			this->Next();
		}
		
		return result;
	}
	
	unsigned ReadLine (CHAR16 *Buffer) {
		CHAR16 *CharBuffer = (CHAR16 *) Buffer;
		
		for (; !this->AtEnd(); ++CharBuffer) {
			if (this->Current() == L'\n') {
				this->Next();
				
				break;
			}
			
			*CharBuffer = (CHAR16) this->Current();
			
			this->Next();
		}
		
		if (*(CharBuffer - 1) == L'\r') {
			--CharBuffer;
		}
		
		*CharBuffer = L'\0';
		
		return CharBuffer - Buffer;
	}
};

#endif
