#include "common.h"

const char* MUTEX_NAME = "MessageFileMutex";
const char* EMPTY_SEMAPHORE_NAME = "EmptySlotsSemaphore";
const char* FILLED_SEMAPHORE_NAME = "FilledSlotsSemaphore";
const char* READY_EVENT_NAME_PREFIX = "SenderReadyEvent_";

FileHeader ReadFileHeader(HANDLE hFile) {
    FileHeader header;
    DWORD bytesRead;
    
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    ReadFile(hFile, &header, sizeof(FileHeader), &bytesRead, NULL);
    
    return header;
}

void WriteFileHeader(HANDLE hFile, const FileHeader& header) {
    DWORD bytesWritten;
    
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    WriteFile(hFile, &header, sizeof(FileHeader), &bytesWritten, NULL);
}

Message ReadMessage(HANDLE hFile, int index) {
    Message message;
    DWORD bytesRead;
    
    DWORD offset = sizeof(FileHeader) + index * sizeof(Message);
    SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
    ReadFile(hFile, &message, sizeof(Message), &bytesRead, NULL);
    
    return message;
}

void WriteMessage(HANDLE hFile, int index, const Message& message) {
    DWORD bytesWritten;
    
    DWORD offset = sizeof(FileHeader) + index * sizeof(Message);
    SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
    WriteFile(hFile, &message, sizeof(Message), &bytesWritten, NULL);
}