
#ifndef _L_READER_

#define _L_READER_

const int ReaderEof = -1;

const int ReaderBufferSize = 256;

class LReader {
protected:
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
	
	bool AtEnd() {
		return size == ~0U;
	}
	
	int Current() {
		if (this->AtEnd()) {
			return ReaderEof;
		}
		
		return this->buffer[current];
	}
	
	int Next() {
		if (this->AtEnd()) {
			return ReaderEof;
		}
		
		if (this->current != this->size) {
			this->current++;
		} else if (!this->ReadBuffer()) {
			this->size = ~0U;
			
			return ReaderEof;
		} else {
			this->current = 0;
		}
		
		return this->Current();
	}
};

#endif
