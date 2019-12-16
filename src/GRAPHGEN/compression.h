
#include <zstd.h>

#ifndef ZSTD_COMPRESSION_H_
#define ZSTD_COMPRESSION_H_

typedef struct {
	void* fBuffer;
	void* cBuffer;
	size_t fBufferSize;
	size_t cBufferSize;
	ZSTD_CCtx* cctx;
} compression_resources;

class ZstdCompression {
	compression_resources resources_;

public:
	void allocateResources(size_t maxFileSize);
	void compressFile(std::string input_file_name, std::string output_file_name);
	void freeResources();
};

class ZstdStreamingCompression {
	FILE*  fout;
	//size_t buffInSize;
	//void*  buffIn;
	size_t buffOutSize;
	void*  buffOut;

	int compression_level = ZSTD_CLEVEL_DEFAULT;

	ZSTD_CCtx* cctx;

public:
	ZstdStreamingCompression() { };
	ZstdStreamingCompression(int compression_level) : compression_level(compression_level) { };

	void beginStreaming(std::string output_file_name);
	void compressDataChunk(const void* data, size_t read_size, bool last_chunk);
	void endStreaming();
};

class ZstdDecompression {
	ZSTD_DCtx* dctx;

public:
	void allocateResources();
	//void decompressFileSimple(std::string input_file_name, std::string output_file_name);
	void decompressFile(std::string input_file_name, std::string output_file_name);
	void freeResources();
};


#endif // ZSTD_COMPRESSION_H_