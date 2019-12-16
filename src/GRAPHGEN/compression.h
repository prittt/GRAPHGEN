
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

typedef struct {
	void* fBuffer;
	void* cBuffer;
	size_t fBufferSize;
	size_t cBufferSize;
	ZSTD_DCtx* dctx;
} decompression_resources;

class ZstdCompression {
	compression_resources resources_;

public:
	void allocateResources(size_t maxFileSize);
	void compressFile(std::string input_file_name, std::string output_file_name);
	void freeResources();
};

class ZstdDecompression {
	decompression_resources resources_;

public:
	void allocateResources(size_t maxFileSize);
	void decompressFile(std::string input_file_name, std::string output_file_name);
	void freeResources();
};


#endif // ZSTD_COMPRESSION_H_