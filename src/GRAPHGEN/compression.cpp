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

#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>    
#include <string>    
#include <zstd.h>
#include <sys/stat.h> 
#include <vector>

#include "compression.h"

#pragma region Common Functions


/*
	* Define the returned error code from utility functions.
	*/
typedef enum {
	ERROR_fsize = 1,
	ERROR_fopen = 2,
	ERROR_fclose = 3,
	ERROR_fread = 4,
	ERROR_fwrite = 5,
	ERROR_loadFile = 6,
	ERROR_saveFile = 7,
	ERROR_malloc = 8,
	ERROR_largeFile = 9,
} ZstdErrorCode;

/*! CHECK
	* Check that the condition holds. If it doesn't print a message and die.
	*/
#define CHECK(cond, ...)                        \
do {                                        \
    if (!(cond)) {                          \
        fprintf(stderr,                     \
                "%s:%d CHECK(%s) failed: ", \
                __FILE__,                   \
                __LINE__,                   \
                #cond);                     \
        fprintf(stderr, "" __VA_ARGS__);    \
        fprintf(stderr, "\n");              \
        exit(1);                            \
    }                                       \
} while (0)

	/*! CHECK_ZSTD
	* Check the zstd error code and die if an error occurred after printing a
	* message.
	*/
#define CHECK_ZSTD(fn, ...)                                      \
do {                                                         \
    size_t const err = (fn);                                 \
    CHECK(!ZSTD_isError(err), "%s", ZSTD_getErrorName(err)); \
} while (0)

	/*! fileSize() :
	* Get the size of a given file path.
	*
	* @return The size of a given file path.
	*/
size_t fileSize(const char *filename)
{
	struct stat st;
	if (stat(filename, &st) != 0) {
		/* error */
		perror(filename);
		exit(ERROR_fsize);
	}

	off_t const fileSize = st.st_size;
	size_t const size = (size_t)fileSize;
	/* 1. fileSize should be non-negative,
		* 2. if off_t -> size_t type conversion results in discrepancy,
		*    the file size is too large for type size_t.
		*/
	if ((fileSize < 0) || (fileSize != (off_t)size)) {
		fprintf(stderr, "%s : filesize too large \n", filename);
		exit(ERROR_largeFile);
	}
	return size;
}

/*! openFile() :
	* Open a file using given file path and open option.
	*
	* @return If successful this function will return a FILE pointer to an
	* opened file otherwise it sends an error to stderr and exits.
	*/
FILE* openFile(const char *filename, const char *instruction)
{
	FILE* const inFile = fopen(filename, instruction);
	if (inFile) return inFile;
	return nullptr;
}

/*! closeFile() :
	* Close an opened file using given FILE pointer.
	*/
void closeFile(FILE* file)
{
	if (!fclose(file)) { return; };
	/* error */
	perror("fclose");
	exit(ERROR_fclose);
}

/*! readFromFile() :
	*
	* Read sizeToRead bytes from a given file, storing them at the
	* location given by buffer.
	*
	* @return The number of bytes read.
	*/
size_t readFromFile(void* buffer, size_t sizeToRead, FILE* file)
{
	size_t const readSize = fread(buffer, 1, sizeToRead, file);
	if (readSize == sizeToRead) return readSize;   /* good */
	if (feof(file)) return readSize;   /* good, reached end of file */
	/* error */
	perror("fread");
	exit(ERROR_fread);
}

/*! writeToFile() :
	*
	* Write sizeToWrite bytes to a file pointed to by file, obtaining
	* them from a location given by buffer.
	*
	* Note: This function will send an error to stderr and exit if it
	* cannot write data to the given file pointer.
	*
	* @return The number of bytes written.
	*/
size_t writeToFile(const void* buffer, size_t sizeToWrite, FILE* file)
{
	size_t const writtenSize = fwrite(buffer, 1, sizeToWrite, file);
	if (writtenSize == sizeToWrite) return sizeToWrite;   /* good */
	/* error */
	perror("fwrite");
	exit(ERROR_fwrite);
}

/*! malloc() :
	* Allocate memory.
	*
	* @return If successful this function returns a pointer to allo-
	* cated memory.  If there is an error, this function will send that
	* error to stderr and exit.
	*/
void* malloc_(size_t size)
{
	void* const buff = malloc(size);
	if (buff) return buff;
	/* error */
	perror("malloc");
	exit(ERROR_malloc);
}

/*! loadFile() :
	* load file into buffer (memory).
	*
	* Note: This function will send an error to stderr and exit if it
	* cannot read data from the given file path.
	*
	* @return If successful this function will load file into buffer and
	* return file size, otherwise it will printout an error to stderr and exit.
	*/
size_t loadFile(const char* fileName, void* buffer, size_t bufferSize)
{
	size_t const fsize = fileSize(fileName);
	CHECK(fsize <= bufferSize, "File too large!");

	FILE* const inFile = openFile(fileName, "rb");
	size_t const readSize = fread(buffer, 1, fsize, inFile);
	if (readSize != (size_t)fsize) {
		fprintf(stderr, "fread: %s : %s \n", fileName, strerror(errno));
		exit(ERROR_fread);
	}
	fclose(inFile);  /* can't fail, read only */
	return fsize;
}

/*! mallocAndLoadFile() :
	* allocate memory buffer and then load file into it.
	*
	* Note: This function will send an error to stderr and exit if memory allocation
	* fails or it cannot read data from the given file path.
	*
	* @return If successful this function will return buffer and bufferSize(=fileSize),
	* otherwise it will printout an error to stderr and exit.
	*/
void* mallocAndLoadFile(const char* fileName, size_t* bufferSize) {
	size_t const fsize = fileSize(fileName);
	*bufferSize = fsize;
	void* const buffer = malloc_(*bufferSize);
	loadFile(fileName, buffer, *bufferSize);
	return buffer;
}

/*! saveFile() :
	*
	* Save buffSize bytes to a given file path, obtaining them from a location pointed
	* to by buff.
	*
	* Note: This function will send an error to stderr and exit if it
	* cannot write to a given file.
	*/
void saveFile(const char* fileName, const void* buff, size_t buffSize)
{
	FILE* const oFile = openFile(fileName, "wb");
	size_t const wSize = fwrite(buff, 1, buffSize, oFile);
	if (wSize != (size_t)buffSize) {
		fprintf(stderr, "fwrite: %s : %s \n", fileName, strerror(errno));
		exit(ERROR_fwrite);
	}
	if (fclose(oFile)) {
		perror(fileName);
		exit(ERROR_fclose);
	}
}

#pragma endregion


#pragma region Compression

void ZstdCompression::allocateResources(size_t maxFileSize)
{
	resources_.fBufferSize = maxFileSize;
	resources_.cBufferSize = ZSTD_compressBound(maxFileSize);

	resources_.fBuffer = malloc_(resources_.fBufferSize);
	resources_.cBuffer = malloc_(resources_.cBufferSize);
	resources_.cctx = ZSTD_createCCtx();
	CHECK(resources_.cctx != NULL, "ZSTD_createCCtx() failed!");
}

void ZstdCompression::freeResources()
{
	free(resources_.fBuffer);
	free(resources_.cBuffer);
	ZSTD_freeCCtx(resources_.cctx);   /* never fails */
}

void ZstdCompression::compressFile(std::string input_file_name, std::string output_file_name)
{
	const char* fname = input_file_name.c_str();
	const char* oname = output_file_name.c_str();

	size_t fSize = loadFile(fname, resources_.fBuffer, resources_.fBufferSize);

	size_t const cSize = ZSTD_compressCCtx(resources_.cctx, resources_.cBuffer, resources_.cBufferSize, resources_.fBuffer, fSize, 1);
	CHECK_ZSTD(cSize);

	saveFile(oname, resources_.cBuffer, cSize);

	/* success */
	printf("%25s : %6u -> %7u - %s \n", fname, (unsigned)fSize, (unsigned)cSize, oname);
}

#pragma endregion


#pragma region Decompression

void ZstdDecompression::allocateResources()
{
	dctx = ZSTD_createDCtx();
	CHECK(dctx != NULL, "ZSTD_createCCtx() failed!");
}

void ZstdDecompression::freeResources()
{
	ZSTD_freeDCtx(dctx);   /* never fails */
}

void ZstdDecompression::decompressFileToFile(std::string input_file_name, std::string output_file_name)
{
	FILE* const fin = openFile(input_file_name.c_str(), "rb");
	size_t const buffInSize = ZSTD_DStreamInSize();
	void*  const buffIn = malloc_(buffInSize);
	FILE* const fout = openFile(output_file_name.c_str(), "wb");
	size_t const buffOutSize = ZSTD_DStreamOutSize();  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
	void*  const buffOut = malloc_(buffOutSize);

	/* This loop assumes that the input file is one or more concatenated zstd
	 * streams. This example won't work if there is trailing non-zstd data at
	 * the end, but streaming decompression in general handles this case.
	 * ZSTD_decompressStream() returns 0 exactly when the frame is completed,
	 * and doesn't consume input after the frame.
	 */
	size_t const toRead = buffInSize;
	size_t read;
	size_t lastRet = 0;
	int isEmpty = 1;
	while ((read = readFromFile(buffIn, toRead, fin))) {
		isEmpty = 0;
		ZSTD_inBuffer input = { buffIn, read, 0 };
		/* Given a valid frame, zstd won't consume the last byte of the frame
		 * until it has flushed all of the decompressed data of the frame.
		 * Therefore, instead of checking if the return code is 0, we can
		 * decompress just check if input.pos < input.size.
		 */
		while (input.pos < input.size) {
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			/* The return code is zero if the frame is complete, but there may
			 * be multiple frames concatenated together. Zstd will automatically
			 * reset the context when a frame is complete. Still, calling
			 * ZSTD_DCtx_reset() can be useful to reset the context to a clean
			 * state, for instance if the last decompression call returned an
			 * error.
			 */
			size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
			CHECK_ZSTD(ret);
			writeToFile(buffOut, output.pos, fout);
			lastRet = ret;
		}
	}

	if (isEmpty) {
		fprintf(stderr, "input is empty\n");
		exit(1);
	}

	if (lastRet != 0) {
		/* The last return value from ZSTD_decompressStream did not end on a
		 * frame, but we reached the end of the file! We assume this is an
		 * error, and the input was truncated.
		 */
		fprintf(stderr, "EOF before end of stream: %zu\n", lastRet);
		exit(1);
	}

	closeFile(fin);
	closeFile(fout);
	free(buffIn);
	free(buffOut);
}

void ZstdDecompression::decompressFileToMemory(std::string input_file_name, std::vector<action_bitset>& data) {
	FILE* const fin = openFile(input_file_name.c_str(), "rb");
	size_t const buffInSize = ZSTD_DStreamInSize();
	void*  const buffIn = malloc_(buffInSize);
	size_t const buffOutSize = ZSTD_DStreamOutSize();  /* Guarantee to successfully flush at least one complete compressed block in all circumstances. */
	void*  const buffOut = malloc_(buffOutSize);

	/* This loop assumes that the input file is one or more concatenated zstd
	 * streams. This example won't work if there is trailing non-zstd data at
	 * the end, but streaming decompression in general handles this case.
	 * ZSTD_decompressStream() returns 0 exactly when the frame is completed,
	 * and doesn't consume input after the frame.
	 */
	size_t const toRead = buffInSize;
	action_bitset* dst = &data[0];
	size_t read;
	size_t lastRet = 0;
	int isEmpty = 1;
	while ((read = readFromFile(buffIn, toRead, fin))) {
		isEmpty = 0;
		ZSTD_inBuffer input = { buffIn, read, 0 };
		/* Given a valid frame, zstd won't consume the last byte of the frame
		 * until it has flushed all of the decompressed data of the frame.
		 * Therefore, instead of checking if the return code is 0, we can
		 * decompress just check if input.pos < input.size.
		 */
		while (input.pos < input.size) {
			ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
			/* The return code is zero if the frame is complete, but there may
			 * be multiple frames concatenated together. Zstd will automatically
			 * reset the context when a frame is complete. Still, calling
			 * ZSTD_DCtx_reset() can be useful to reset the context to a clean
			 * state, for instance if the last decompression call returned an
			 * error.
			 */
			size_t const ret = ZSTD_decompressStream(dctx, &output, &input);
			CHECK_ZSTD(ret);

			memcpy(dst, (action_bitset*)output.dst, output.pos);
			dst += output.pos;
			lastRet = ret;
		}
	}

	if (isEmpty) {
		fprintf(stderr, "input is empty\n");
		exit(1);
	}

	if (lastRet != 0) {
		/* The last return value from ZSTD_decompressStream did not end on a
		 * frame, but we reached the end of the file! We assume this is an
		 * error, and the input was truncated.
		 */
		fprintf(stderr, "EOF before end of stream: %zu\n", lastRet);
		exit(1);
	}

	closeFile(fin);
	free(buffIn);
	free(buffOut);
}

void ZstdDecompression::decompressFileToFileAlt(std::string input_file_name, std::string output_file_name)
{
	size_t cSize;
	const char* fname = input_file_name.c_str();
	const char* oname = output_file_name.c_str();
	void* const cBuff = mallocAndLoadFile(fname, &cSize);
	/* Read the content size from the frame header. For simplicity we require
	 * that it is always present. By default, zstd will write the content size
	 * in the header when it is known. If you can't guarantee that the frame
	 * content size is always written into the header, either use streaming
	 * decompression, or ZSTD_decompressBound().
	 */
	unsigned long long const rSize = ZSTD_getFrameContentSize(cBuff, cSize);
	CHECK(rSize != ZSTD_CONTENTSIZE_ERROR, "%s: not compressed by zstd!", fname);
	CHECK(rSize != ZSTD_CONTENTSIZE_UNKNOWN, "%s: original size unknown!", fname);

	CHECK(rSize <= sizeof(size_t), "Read Frame size too large for size_t!");

	void* const rBuff = malloc_(static_cast<size_t>(rSize));

	size_t const dSize = ZSTD_decompressDCtx(dctx, rBuff, static_cast<size_t>(rSize), cBuff, cSize);
	CHECK_ZSTD(dSize);
	/* When zstd knows the content size, it will error if it doesn't match. */
	CHECK(dSize == rSize, "Impossible because zstd will check this condition!");

	saveFile(oname, rBuff, rSize);

	/* success */
	printf("%25s : %6u -> %7u \n", fname, (unsigned)cSize, (unsigned)rSize);

	free(rBuff);
	free(cBuff);
}

#pragma endregion


#pragma region Streaming Compression

void ZstdStreamingCompression::allocateResources() {
	/* Create the context. */
	cctx = ZSTD_createCCtx();
	CHECK(cctx != NULL, "ZSTD_createCCtx() failed!");

	/* Set any parameters you want.
	 * Here we set the compression level, and enable the checksum.
	 */
	CHECK_ZSTD(ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, compression_level));
	CHECK_ZSTD(ZSTD_CCtx_setParameter(cctx, ZSTD_c_checksumFlag, 1));
}

bool ZstdStreamingCompression::beginStreaming(std::string output_file_name) {
	fout = openFile(output_file_name.c_str(), "wb");
	if (fout == nullptr) {
		return false;
	}
	/* Create the input and output buffers.
	 * They may be any size, but we recommend using these functions to size them.
	 * Performance will only suffer significantly for very tiny buffers.
	 */
	buffOutSize = ZSTD_CStreamOutSize();
	buffOut = malloc_(buffOutSize);
	return true;
}

void ZstdStreamingCompression::compressDataChunk(const void* data, size_t read_size, bool last_chunk) {
	/* Select the flush mode.
		* If the read may not be finished (read == toRead) we use
		* ZSTD_e_continue. If this is the last chunk, we use ZSTD_e_end.
		* Zstd optimizes the case where the first flush mode is ZSTD_e_end,
		* since it knows it is compressing the entire source in one pass.
		*/
	ZSTD_EndDirective const mode = last_chunk ? ZSTD_e_end : ZSTD_e_continue;
	/* Set the input buffer to what we just read.
		* We compress until the input buffer is empty, each time flushing the
		* output.
		*/
	ZSTD_inBuffer input = { data, read_size, 0 };
	int finished;
	do {
		/* Compress into the output buffer and write all of the output to
			* the file so we can reuse the buffer next iteration.
			*/
		ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
		size_t const remaining = ZSTD_compressStream2(cctx, &output, &input, mode);
		CHECK_ZSTD(remaining);
		writeToFile(buffOut, output.pos, fout);
		/* If we're on the last chunk we're finished when zstd returns 0,
			* which means its consumed all the input AND finished the frame.
			* Otherwise, we're finished when we've consumed all the input.
			*/
		finished = last_chunk ? (remaining == 0) : (input.pos == input.size);
	} while (!finished);
	CHECK(input.pos == input.size,
		"Impossible: zstd only returns 0 when the input is completely consumed!");
}

void ZstdStreamingCompression::endStreaming() {
	closeFile(fout);
	free(buffOut);
}

void ZstdStreamingCompression::freeResources()
{
	ZSTD_freeCCtx(cctx);
}

#pragma endregion
