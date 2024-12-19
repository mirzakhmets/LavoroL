
#ifndef _L_READER_

#define _L_READER_

const int ReaderEof = -1;

const int BufferSize = 128;

class LReader {
protected:
	unsigned long size = 0;
	unsigned long current = 0;
	
	unsigned char buffer[BufferSize + 1];
	
	virtual bool ReadBuffer() {
		return false;
	}
public:
	LReader() {
	}
	
	~LReader() {
	}
	
	bool AtEnd() {
		if (size == ~0U) {
			return true;
		}
		
		if (current == size) {
			if (!this->ReadBuffer()) {
				size = ~0U;
			} else if (size) {
				current = 0;
			} else {
				size = ~0U;
			}
		}
		
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
		}
		
		return this->Current();
	}
};

#endif
