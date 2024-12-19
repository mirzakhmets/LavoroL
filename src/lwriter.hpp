
#ifndef _L_WRITER_

#define _L_WRITER_

const int WriterBufferSize = 128;

class LWriter {
protected:
	unsigned long current = 0;
	unsigned char buffer[WriterBufferSize + 1];
	
	virtual void WriteBuffer() {
	}
public:
	LWriter() {
	}
	
	~LWriter() {
		this->Flush();
	}
	
	void Flush() {
		this->WriteBuffer();
		
		this->current = 0;
	}
	
	void Write(const unsigned char ch) {
		if (current == sizeof(buffer)) {
			this->Flush();
		}
		
		buffer[current++] = ch;
	}
	
	void Write(const unsigned char *pbuffer, const unsigned size) {
		for (unsigned i = 0; i != size; ++i, pbuffer++) {
			Write (*pbuffer);
		}
	}	
};

#endif
