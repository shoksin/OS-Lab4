#include "common.h"
#include <iostream>
#include <vector>
#include <string>

int main(void) {
    std::string fileName;
    int numSlots;
    int numSenders;
    
    std::cout << "Enter binary file name: ";
    std::cin >> fileName;
    std::cout << "Enter number of slots in the file: ";
    std::cin >> numSlots;
    if (numSlots < 1) {
        std::cout<<"Number of slots in the file should be 1 or more"<<std::endl;
        return 1;
    }
    
    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating file: " << GetLastError() << std::endl;
        return 1;
    }
    
    FileHeader header;
    header.readIndex = 0;
    header.writeIndex = 0;
    header.messageCount = 0;
    header.totalSlots = numSlots;
    WriteFileHeader(hFile, header);
    
    Message emptyMessage;
    ZeroMemory(&emptyMessage, sizeof(Message));
    emptyMessage.isOccupied = false;
    
    for (int i = 0; i < numSlots; i++) {
        WriteMessage(hFile, i, emptyMessage);
    }
    
    HANDLE hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    HANDLE hEmptySemaphore = CreateSemaphoreA(NULL, numSlots, numSlots, EMPTY_SEMAPHORE_NAME);
    HANDLE hFilledSemaphore = CreateSemaphoreA(NULL, 0, numSlots, FILLED_SEMAPHORE_NAME);
    
    if (!hMutex || !hEmptySemaphore || !hFilledSemaphore) {
        std::cerr << "Error creating synchronization objects: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }
    
    std::cout << "Enter number of Sender processes: ";
    std::cin >> numSenders;
    if (numSenders < 1){
        std::cout<<"Number of Sender processes should be 1 or more"<<std::endl;
        return 1;
    }
    
    std::vector<HANDLE> senderProcesses;
    std::vector<HANDLE> readyEvents;
    
    for (int i = 0; i < numSenders; i++) {
        // Создание события готовности для этого Sender
        std::string eventName = READY_EVENT_NAME_PREFIX + std::to_string(i);
        HANDLE hReadyEvent = CreateEventA(NULL, TRUE, FALSE, eventName.c_str());
        if (hReadyEvent == NULL) {
            std::cerr << "Error creating ready event: " << GetLastError() << std::endl;
            continue;
        }
        readyEvents.push_back(hReadyEvent);
        
        std::string cmdLine = "sender.exe " + fileName + " " + std::to_string(i);
        
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;
        ZeroMemory(&pi, sizeof(pi));
        
        if (!CreateProcessA(NULL, const_cast<LPSTR>(cmdLine.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cerr << "Error creating sender process: " << GetLastError() << std::endl;
            continue;
        }
        
        senderProcesses.push_back(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    if (senderProcesses.size() == 0) {
        std::cerr << "Failed to create any sender processes" << std::endl;
        CloseHandle(hFile);
        CloseHandle(hMutex);
        CloseHandle(hEmptySemaphore);
        CloseHandle(hFilledSemaphore);
        return 1;
    }
    
    std::cout << "Waiting for all senders to be ready..." << std::endl;
    
    WaitForMultipleObjects(static_cast<DWORD>(readyEvents.size()), readyEvents.data(), TRUE, INFINITE);
    
    std::cout << "All senders are ready!" << std::endl;
    
    bool running = true;
    while (running) {
        std::cout << "Enter command (r - read message, q - quit): ";
        char cmd;
        std::cin >> cmd;
        
        switch (cmd) {
            case 'r': {
                std::cout << "Waiting for a message..." << std::endl;
                WaitForSingleObject(hFilledSemaphore, INFINITE);
                WaitForSingleObject(hMutex, INFINITE);
                
                FileHeader header = ReadFileHeader(hFile);
                Message message = ReadMessage(hFile, header.readIndex);
                
                header.readIndex = (header.readIndex + 1) % header.totalSlots;
                header.messageCount--;
                WriteFileHeader(hFile, header);
                
                message.isOccupied = false;
                WriteMessage(hFile, (header.readIndex - 1 + header.totalSlots) % header.totalSlots, message);
                
                ReleaseMutex(hMutex);
                ReleaseSemaphore(hEmptySemaphore, 1, NULL);
                
                std::cout << "Message received: " << message.text << std::endl;
                break;
            }
            case 'q': {
                running = false;
                break;
            }
            default: {
                std::cout << "Unknown command." << std::endl;
                break;
            }
        }
    }
    
    CloseHandle(hFile);
    CloseHandle(hMutex);
    CloseHandle(hEmptySemaphore);
    CloseHandle(hFilledSemaphore);
    
    for (HANDLE h : readyEvents) {
        CloseHandle(h);
    }
    
    for (HANDLE h : senderProcesses) {
        TerminateProcess(h, 0);
        CloseHandle(h);
    }
    
    return 0;
}