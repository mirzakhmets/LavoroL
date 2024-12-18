
#ifndef _L_READER_

#define _L_READER_

const int ReaderEof = -1;

const int BufferSize = 128;

class LReader {
protected:
	unsigned size = 0;
	unsigned current = 0;
	
	unsigned char buffer[BufferSize];
	
	virtual bool ReadBuffer() {
		return false;
	}
public:
	LReader() {
	}
	
	~LReader() {
	}
	
	bool AtEnd() {
		if (size == ~0) {
			return true;
		}
		
		if (current == size) {
			if (!this->ReadBuffer()) {
				size = ~0U;
			} else {
				current = 0;
			}
		}
		
		return current == size;
	}
	
	int Current() {
		if (this->AtEnd()) {
			return ReaderEof;
		}
		
		return this->buffer[current];
	}
	
	int Next() {
		if (this->current != this->size) {
			this->current++;
		}
		
		return this->Current();
	}
};

#endif
