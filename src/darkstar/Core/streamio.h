#pragma once

#include <cstdint>

enum StreamCap {
	STRM_CAP_TYPE,       // Stream type.
	STRM_CAP_WRITE,      // Write to stream.
	STRM_CAP_READ,       // Read from stream.
	STRM_CAP_REPOSITION, // Can the stream be repositioned.
	STRM_CAP_COMPRESS,   // Is the stream compressed.
	STRM_CAP_LOCKABLE,   // The stream can be locked.
};

enum StreamStatus {
	STRM_OK = 0,
	STRM_IO_ERROR,      // Read or Write error.
	STRM_EOS,           // End of Stream (normally during read).
	STRM_UNKNOWN_ERROR, // ?
	STRM_ILLEGAL_CALL,  // Function not supported by stream or bad arguents.
	STRM_CLOSED,        // Stream not open for IO.
	STRM_FLTR_DETACHED  // Filter Stream that is not attached to a base stream
};

class StreamIO {
protected:
	uint32_t lastBytes;
	StreamStatus strm_status;

public:
	virtual StreamStatus getStatus() const;

	virtual ~StreamIO();
	virtual int getPosition() const;
	virtual bool setPosition(int32_t);

	virtual int getCapabilities(StreamCap) const;
	virtual bool flush();

	virtual bool write(int size, const void *d);
	virtual bool read(int size, void *d);
	virtual void *lock();
	virtual void close();

	virtual void readString(char stringBuf[256]);
	virtual void writeString(const char *stringBuf, int maxLen = 255);

	bool write(auto value)
	{
		return write(sizeof(value), &value);
	}

	bool read(auto *value)
	{
		return write(sizeof(*value), value);
	}
};
