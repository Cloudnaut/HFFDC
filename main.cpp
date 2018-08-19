/* Copyright (c) 2018 Cloudnaut */

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include "md5.h"
using namespace std;

void PrintHelp();
bool IsString(char a[], const char b[]);
long long GetFileSize(int &fileHandle);
void ExpandFile(int *fileHandle, long long size);
void ShrinkFile(int *fileHandle, long long size);
void DeltaCopy(int *fromFileHandle, int *toFileHandle, long long fileSize, unsigned int blockSize);
void DeltaCopyChecksum(int *fromFileHandle, ifstream &fromChecksumFile, int *toFileHandle, ifstream &toChecksumFile, long long fileSize);
void CreateChecksumFile(ofstream &checksumFile, int *fromFileHandle, long long fileSize, unsigned int blockSize);
inline void ReadFileContentToBuffer(int *fileHandle, long long position, unsigned int length, char *buffer);
void PrintProgress(long long *currentBlock, long long blockCount);

int main(int argc, char *argv[])
{
	if (argc < 2) //No parameters
	{
		PrintHelp();
		return EXIT_SUCCESS;
	}

	if(IsString(argv[1], "-gc") || IsString(argv[1], "--generate-checksum")) //Generate Checksum
	{
		char *fromFilePath = argv[2];
		unsigned int blockSize = atoi(argv[3]);

		int fromFileHandle = 0;
		_sopen_s(&fromFileHandle, fromFilePath, _O_RDONLY | _O_BINARY, _SH_DENYNO, 0);
		long long fromFileSize = GetFileSize(fromFileHandle);

		char checksumFilePath[_MAX_PATH];
		strcpy_s(checksumFilePath, fromFilePath);
		strcat_s(checksumFilePath, ".hffdc");

		ofstream checksumFile(checksumFilePath);
		CreateChecksumFile(checksumFile, &fromFileHandle, fromFileSize, blockSize);

		checksumFile.close();
		_close(fromFileHandle);
	}
	else if(IsString(argv[1], "-compcpy") || IsString(argv[1], "--compare-and-copy")) //Compare and copy
	{
		char *fromFilePath = argv[2];
		char *toFilePath = argv[3];
		unsigned int blockSize = atoi(argv[4]);

		int fromFileHandle = 0;
		int toFileHandle = 0;

		_sopen_s(&fromFileHandle, fromFilePath, _O_RDWR | _O_BINARY, _SH_DENYNO, 0);
		_sopen_s(&toFileHandle, toFilePath, _O_RDWR | _O_BINARY, _SH_DENYNO, 0);

		long long fromFileSize = GetFileSize(fromFileHandle);
		long long toFileSize = GetFileSize(toFileHandle);

		if (fromFileSize < 0 || toFileSize < 0)
			exit(0xDEAD);

		if (fromFileSize > toFileSize)
			ExpandFile(&toFileHandle, fromFileSize);
		else if (fromFileSize < toFileSize)
			ShrinkFile(&toFileHandle, fromFileSize);

		DeltaCopy(&fromFileHandle, &toFileHandle, fromFileSize, blockSize);

		_close(fromFileHandle);
		_close(toFileHandle);
	}
	else if(IsString(argv[1], "-chksmcpy") || IsString(argv[1], "--checksum-copy")) //Copy with checksum files
	{
		char *fromFilePath = argv[2];
		char *toFilePath = argv[3];

		char fromChecksumFilePath[_MAX_PATH];
		strcpy_s(fromChecksumFilePath, fromFilePath);
		strcat_s(fromChecksumFilePath, ".hffdc");

		char toChecksumFilePath[_MAX_PATH];
		strcpy_s(toChecksumFilePath, toFilePath);
		strcat_s(toChecksumFilePath, ".hffdc");

		ifstream fromChecksumFile(fromChecksumFilePath);
		ifstream toChecksumFile(toChecksumFilePath);

		int fromFileHandle = 0;
		int toFileHandle = 0;

		_sopen_s(&fromFileHandle, fromFilePath, _O_RDWR | _O_BINARY, _SH_DENYNO, 0);
		_sopen_s(&toFileHandle, toFilePath, _O_RDWR | _O_BINARY, _SH_DENYNO, 0);

		long long fromFileSize = GetFileSize(fromFileHandle);
		long long toFileSize = GetFileSize(toFileHandle);

		if (fromFileSize < 0 || toFileSize < 0)
			exit(0xDEAD);

		if (fromFileSize > toFileSize)
			ExpandFile(&toFileHandle, fromFileSize);
		else if (fromFileSize < toFileSize)
			ShrinkFile(&toFileHandle, fromFileSize);

		DeltaCopyChecksum(&fromFileHandle, fromChecksumFile, &toFileHandle, toChecksumFile, fromFileSize);

		fromChecksumFile.close();
		toChecksumFile.close();

		_close(fromFileHandle);
		_close(toFileHandle);
	}
	else PrintHelp();

	return EXIT_SUCCESS;
}

void PrintHelp()
{
	cout << "Copyright (c) 2018 Cloudnaut" << endl;
	cout << "HFFDC is a program designed to copy huge files efficiently" << endl << endl;
	cout << "Usage: " << endl;
	cout << "HFFDC.exe OPTION [PARAMETERS]" << endl << endl;
	
	cout << "Options:" << endl << endl;
	
	cout << "-gc, --generate-checksum" << endl;
	cout << '\t' << "[PARAMETERS: filePath blockSize]" << endl;
	cout << '\t' << "Generates a checksum file for the specified file" << endl << endl;
	
	cout << "-compcpy, --compare-and-copy" << endl;
	cout << '\t' << "[PARAMETERS: sourceFilePath targetFilePath blockSize]" << endl;
	cout << '\t' << "Reads each block of the source and the targetfile, only differing blocks will be copied to destination" << endl << endl;

	cout << "-chksmcpy, --checksum-copy" << endl;
	cout << '\t' << "[PARAMETERS: sourceFilePath targetFilePath]" << endl;
	cout << '\t' << "Reads the checksum files for each block, only blocks with differing checksums will be copied to destination" << endl << endl;
}

bool IsString(char a[], const char b[])
{
	return memcmp(a, b, strlen(a)) == 0;
}

long long GetFileSize(int &fileHandle)
{
	long long fileSize = _lseeki64(fileHandle, 0L, SEEK_SET);
	return _lseeki64(fileHandle, 0L, SEEK_END) - fileSize;
}

void ExpandFile(int *fileHandle, long long size)
{
	exit(0xBEEF);
}

void ShrinkFile(int *fileHandle, long long size)
{
	exit(0xBEEF);
}

void DeltaCopy(int *fromFileHandle, int *toFileHandle, long long fileSize, unsigned int blockSize)
{
	long long blockCount = fileSize / blockSize;
	unsigned int lastBlockSize = fileSize % blockSize;
	long long currentBlock = 0;

	char *fromBuffer = new char[blockSize];
	char *toBuffer = new char[blockSize];

	thread progressThread(PrintProgress, &currentBlock, blockCount);
	chrono::steady_clock::time_point startTime = chrono::high_resolution_clock::now();
	for (; currentBlock < blockCount; currentBlock++)
	{
		thread fromThread(ReadFileContentToBuffer, fromFileHandle, currentBlock * blockSize, blockSize, fromBuffer);
		thread toThread(ReadFileContentToBuffer, toFileHandle, currentBlock * blockSize, blockSize, toBuffer);

		fromThread.join();
		toThread.join();

		if (memcmp(fromBuffer, toBuffer, blockSize) != 0)
		{
			_lseeki64(*toFileHandle, currentBlock * blockSize, SEEK_SET);
			_write(*toFileHandle, fromBuffer, blockSize);
		}
	}

	thread fromThread(ReadFileContentToBuffer, fromFileHandle, blockCount * blockSize, lastBlockSize, fromBuffer);
	thread toThread(ReadFileContentToBuffer, toFileHandle, blockCount * blockSize, lastBlockSize, toBuffer);

	fromThread.join();
	toThread.join();

	if (memcmp(fromBuffer, toBuffer, lastBlockSize) != 0)
	{
		_lseeki64(*toFileHandle, blockCount * blockSize, SEEK_SET);
		_write(*toFileHandle, fromBuffer, lastBlockSize);
	}

	chrono::steady_clock::time_point stopTime = chrono::high_resolution_clock::now();
	chrono::duration<double, milli> elapsedTime = stopTime - startTime;
	progressThread.join();

	cout << "File copy took " << elapsedTime.count() << " ms" << endl;

	delete[] fromBuffer;
	delete[] toBuffer;
}

void DeltaCopyChecksum(int *fromFileHandle, ifstream &fromChecksumFile, int *toFileHandle, ifstream &toChecksumFile, long long fileSize)
{
	unsigned int fromBlockSize, toBlockSize;
	fromChecksumFile >> fromBlockSize;
	toChecksumFile >> toBlockSize;

	if(fromBlockSize != toBlockSize)
	{
		cout << "Checksum blocksize mismatch (Source: " << fromBlockSize << " | Target: " << toBlockSize << ")";
		exit(0xDEAD);
	}

	unsigned int blockSize = fromBlockSize;

	long long blockCount = fileSize / blockSize;
	unsigned int lastBlockSize = fileSize % blockSize;
	long long currentBlock = 0;

	char *fromBuffer = new char[blockSize];
	char *toBuffer = new char[blockSize];
	char fromBlockChecksum[33];
	char toBlockChecksum[33];

	thread progressThread(PrintProgress, &currentBlock, blockCount);
	chrono::steady_clock::time_point startTime = chrono::high_resolution_clock::now();
	for (; currentBlock < blockCount; currentBlock++)
	{
		if(fromChecksumFile >> fromBlockChecksum && toChecksumFile >> toBlockChecksum) //Try to copy only by checking checksums
		{
			if(memcmp(&fromBlockChecksum, &toBlockChecksum, 33) != 0)
			{
				ReadFileContentToBuffer(fromFileHandle, currentBlock * blockSize, blockSize, fromBuffer);
				_lseeki64(*toFileHandle, currentBlock * blockSize, SEEK_SET);
				_write(*toFileHandle, fromBuffer, blockSize);
			}

			continue;
		}

		//Could not use checksums to compare, do it manually
		thread fromThread(ReadFileContentToBuffer, fromFileHandle, currentBlock * blockSize, blockSize, fromBuffer);
		thread toThread(ReadFileContentToBuffer, toFileHandle, currentBlock * blockSize, blockSize, toBuffer);

		fromThread.join();
		toThread.join();

		if (memcmp(fromBuffer, toBuffer, blockSize) != 0)
		{
			_lseeki64(*toFileHandle, currentBlock * blockSize, SEEK_SET);
			_write(*toFileHandle, fromBuffer, blockSize);
		}
	}

	if (fromChecksumFile >> fromBlockChecksum && toChecksumFile >> toBlockChecksum) //Try to copy only by checking checksums
	{
		if (memcmp(&fromBlockChecksum, &toBlockChecksum, 33) != 0)
		{
			ReadFileContentToBuffer(fromFileHandle, currentBlock * blockSize, blockSize, fromBuffer);
			_lseeki64(*toFileHandle, currentBlock * blockSize, SEEK_SET);
			_write(*toFileHandle, fromBuffer, blockSize);
		}
	}
	else //Could not use checksums to compare, do it manually
	{
		thread fromThread(ReadFileContentToBuffer, fromFileHandle, blockCount * blockSize, lastBlockSize, fromBuffer);
		thread toThread(ReadFileContentToBuffer, toFileHandle, blockCount * blockSize, lastBlockSize, toBuffer);

		fromThread.join();
		toThread.join();

		if (memcmp(fromBuffer, toBuffer, lastBlockSize) != 0)
		{
			_lseeki64(*toFileHandle, blockCount * blockSize, SEEK_SET);
			_write(*toFileHandle, fromBuffer, lastBlockSize);
		}
	}

	chrono::steady_clock::time_point stopTime = chrono::high_resolution_clock::now();
	chrono::duration<double, milli> elapsedTime = stopTime - startTime;
	progressThread.join();

	cout << "File copy took " << elapsedTime.count() << " ms" << endl;

	delete[] fromBuffer;
	delete[] toBuffer;
}

void CreateChecksumFile(ofstream &checksumFile, int *fromFileHandle, long long fileSize, unsigned int blockSize)
{
	MD5_CTX ctx;
	unsigned char digest[16];
	char resultBuffer[33];
	long long blockCount = fileSize / blockSize;
	unsigned int lastBlockSize = fileSize % blockSize;
	long long currentBlock = 0;

	checksumFile << blockSize << endl;

	char *fromBuffer = new char[blockSize];

	thread progressThread(PrintProgress, &currentBlock, blockCount);
	chrono::steady_clock::time_point startTime = chrono::high_resolution_clock::now();
	for (; currentBlock < blockCount; currentBlock++)
	{
		MD5_Init(&ctx);
		ReadFileContentToBuffer(fromFileHandle, currentBlock * blockSize, blockSize, fromBuffer);
		MD5_Update(&ctx, fromBuffer, blockSize);
		MD5_Final(digest, &ctx);
		memset(&resultBuffer, '\0', 33);
		for (int i = 0; i < 16; ++i)
			sprintf_s(&resultBuffer[i * 2], 33, "%02x", (unsigned int)digest[i]);
		checksumFile << resultBuffer << endl;
	}

	MD5_Init(&ctx);
	ReadFileContentToBuffer(fromFileHandle, blockCount * blockSize, lastBlockSize, fromBuffer);
	MD5_Update(&ctx, fromBuffer, lastBlockSize);
	MD5_Final(digest, &ctx);
	memset(&resultBuffer, '\0', 33);
	for (int i = 0; i < 16; ++i)
		sprintf_s(&resultBuffer[i * 2], 33, "%02x", (unsigned int)digest[i]);
	checksumFile << resultBuffer << endl;

	chrono::steady_clock::time_point stopTime = chrono::high_resolution_clock::now();
	chrono::duration<double, milli> elapsedTime = stopTime - startTime;
	progressThread.join();

	cout << "Checksum creation took " << elapsedTime.count() << " ms" << endl;

	delete[] fromBuffer;
}

inline void ReadFileContentToBuffer(int *fileHandle, long long position, unsigned int length, char *buffer)
{
	_lseeki64(*fileHandle, position, SEEK_SET);
	_read(*fileHandle, buffer, length);
}

void PrintProgress(long long *currentBlock, long long blockCount)
{
	while (*currentBlock != blockCount)
	{
		cout << "Progress " << *currentBlock + 1 << "/" << blockCount + 1 << ": " << ((*currentBlock + 1.0) / (blockCount + 1.0)) * 100 << " %" << endl;
		this_thread::sleep_for(1s);
	}

	cout << "Progress " << *currentBlock + 1 << "/" << blockCount + 1 << ": " << ((*currentBlock + 1.0) / (blockCount + 1.0)) * 100 << " %" << endl;
}