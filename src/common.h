#pragma once

#include <windows.h>
#include <string>

const int MAX_MESSAGE_LENGTH = 20;

struct Message {
    char text[MAX_MESSAGE_LENGTH];
    bool isOccupied;
};

struct FileHeader {
    int readIndex;     // Индекс для чтения
    int writeIndex;    // Индекс для записи
    int messageCount;  // Текущее количество сообщений
    int totalSlots;    // Общее количество слотов
};

extern const char* MUTEX_NAME;
extern const char* EMPTY_SEMAPHORE_NAME;
extern const char* FILLED_SEMAPHORE_NAME;
extern const char* READY_EVENT_NAME_PREFIX;

FileHeader ReadFileHeader(HANDLE hFile);
void WriteFileHeader(HANDLE hFile, const FileHeader& header);
Message ReadMessage(HANDLE hFile, int index);
void WriteMessage(HANDLE hFile, int index, const Message& message);