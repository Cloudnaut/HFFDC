}
#include <iostream>
#include <thread>
#include <chrono>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

long long GetFileSize(int &fileHandle);
void ExpandFile(int *fileHandle, long long size);
void ShrinkFile(int *fileHandle, long long size);
void DeltaCopy(int *fromFileHandle, int *toFileHandle, long long fileSize, unsigned int blockSize);
inline void ReadFileContentToBuffer(int *fileHandle, long long position, unsigned int length, char *buffer);
void PrintProgress(long long *currentBlock, long long blockCount);

int main(int argc, char *argv[])
{
 if (argc != 4)
  exit(0xDEAD);

 char *fromFilePath = argv[1];
 char *toFilePath = argv[2];
 unsigned int blockSize = atoi(argv[3]);

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

 return EXIT_SUCCESS;
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
