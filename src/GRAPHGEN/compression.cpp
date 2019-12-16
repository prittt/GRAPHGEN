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
	/* error */
	perror(filename);
	exit(ERROR_fopen);
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

void ZstdDecompression::allocateResources(size_t maxFileSize)
{
	//resources_.fBufferSize = maxFileSize;
	//resources_.cBufferSize = ZSTD_decompressBound;
	
	//resources_.fBuffer = malloc_(resources_.fBufferSize);
	//resources_.cBuffer = malloc_(resources_.cBufferSize);
	resources_.dctx = ZSTD_createDCtx();
	CHECK(resources_.dctx != NULL, "ZSTD_createCCtx() failed!");
}

void ZstdDecompression::freeResources()
{
	/*free(resources_.fBuffer);
	free(resources_.cBuffer);*/
	ZSTD_freeDCtx(resources_.dctx);   /* never fails */
}

void ZstdDecompression::decompressFile(std::string input_file_name, std::string output_file_name)
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

	void* const rBuff = malloc_((size_t)rSize);

	size_t const dSize = ZSTD_decompressDCtx(resources_.dctx, rBuff, rSize, cBuff, cSize);
	CHECK_ZSTD(dSize);
	/* When zstd knows the content size, it will error if it doesn't match. */
	CHECK(dSize == rSize, "Impossible because zstd will check this condition!");

	saveFile(oname, rBuff, rSize);

	/* success */
	printf("%25s : %6u -> %7u \n", fname, (unsigned)cSize, (unsigned)rSize);

	free(rBuff);
	free(cBuff);
}
