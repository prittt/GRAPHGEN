/*
 * Copyright (c) 2016-present, Yann Collet, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

 // Adapted from the examples given in the zstd repository


/*
*  #####################################
*  #          USAGE EXAMPLES           #
*  #####################################
* // Compress loaded memory into a file in a simple way
* ZstdCompression compr;
* compr.allocateResources(128 * 1024 * 1024);
* compr.compressFile("my_uncompressed_file.bin", "my_compressed_file.zst");
* compr.freeResources();
* 
* // Compress loaded memory into a file with a stream
* ZstdStreamingCompression streamcompr;
* streamcompr.beginStreaming("my_stream_output.zst");
* for (int i = 0; i < 100000000; i++) {
* 	streamcompr.compressDataChunk(reinterpret_cast<const void*>(&i), sizeof(i), i == 99999999);
* }
* streamcompr.endStreaming();
* 
* // Decompress a file (using streaming internally)
* ZstdDecompression decompr;
* decompr.allocateResources();
* decompr.decompressFileToFile("my_compressed_file.zst", "my_uncompressed_file.bin");
* decompr.freeResources();
*/

#include <zstd.h>

#include "utilities.h"

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
	size_t buffOutSize;
	void*  buffOut;

	int compression_level = ZSTD_CLEVEL_DEFAULT;

	ZSTD_CCtx* cctx;

public:
	ZstdStreamingCompression() { };
	ZstdStreamingCompression(int compression_level) : compression_level(compression_level) { };

	void allocateResources();
	bool beginStreaming(std::string output_file_name);
	void compressDataChunk(const void* data, size_t read_size, bool last_chunk);
	void endStreaming();
	void freeResources();
};

class ZstdDecompression {
	ZSTD_DCtx* dctx;

public:
	void allocateResources();

	/* Decompresses the given input file and write the uncompressed data into the output file.
	 * This version uses the "streaming decompress" function of Zstd internally. 
	 * It will work in all scenarios but may be slower or less efficient.
	 */
	void decompressFileToFile(std::string input_file_name, std::string output_file_name);

	/* Decompresses the given input file and write the uncompressed data into memory.
	 * This version uses the "streaming decompress" function of Zstd internally.
	 * It will work in all scenarios but may be slower or less efficient.
	 */	
	void decompressFileToMemory(std::string input_file_name, std::vector<action_bitset>& data);

	/* Decompresses the given input file and write the uncompressed data into the output file.
	 * This version uses the simple decompress function of Zstd internally. However, that function
	 * requires that the encoder wrote the frame size into the header. This is not the case for 
	 * a streaming compressor. Therefore, it is usually better to use the decompressFileToFile() function. 
	 */
	void decompressFileToFileAlt(std::string input_file_name, std::string output_file_name);

	void freeResources();
};


#endif // ZSTD_COMPRESSION_H_