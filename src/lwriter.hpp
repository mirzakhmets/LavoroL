
#ifndef _L_WRITER_

#define _L_WRITER_

const int WriterBufferSize = 128;

class LWriter {
protected:
	int current = 0;
	unsigned char buffer[WriterBufferSize];
	
	virtual void WriteBuffer() = 0;
public:
	LWriter() {
	}
	
	void Write(const unsigned char ch) {
		if (current == sizeof(buffer)) {
			WriteBuffer();
			
			current = 0;
		}
		
		buffer[current++] = ch;
	}
	
	void Write(const unsigned char *buffer, const unsigned size) {
		for (unsigned i = 0; i != size; ++i, buffer++) {
			Write (*buffer);
		}
	}
	
	virtual void Destroy() {
		this->WriteBuffer();
	}
};

#endif
