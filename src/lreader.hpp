
#ifndef _L_READER_

#define _L_READER_

const int ReaderEof = -1;

const unsigned long ReaderBufferSize = 66000;

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
};

#endif
