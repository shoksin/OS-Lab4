#include "common.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: sender.exe <file_name> <sender_id>" << std::endl;
        return 1;
    }
    
    std::string fileName = argv[1];
    int senderId = std::atoi(argv[2]);
    
    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening file: " << GetLastError() << std::endl;
        return 1;
    }
    
    HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
    HANDLE hEmptySemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, EMPTY_SEMAPHORE_NAME);
    HANDLE hFilledSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, FILLED_SEMAPHORE_NAME);
    
    if (!hMutex || !hEmptySemaphore || !hFilledSemaphore) {
        std::cerr << "Error opening synchronization objects: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }
    
    std::string eventName = READY_EVENT_NAME_PREFIX + std::to_string(senderId);
    HANDLE hReadyEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, eventName.c_str());
    
    if (!hReadyEvent) {
        std::cerr << "Error opening ready event: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        CloseHandle(hMutex);
        CloseHandle(hEmptySemaphore);
        CloseHandle(hFilledSemaphore);
        return 1;
    }
    
    SetEvent(hReadyEvent);
    CloseHandle(hReadyEvent);
    
    std::cout << "Sender " << senderId << " is ready and waiting for commands." << std::endl;
    
    bool running = true;
    while (running) {
        std::cout << "Enter command (s - send message, q - quit): ";
        std::string cmd;
        std::cin >> cmd;
        if (cmd.length()>1){
            std::cout << "Unknown command." << std::endl;
            continue;
        }
        
        switch (cmd[0]) {
            case 's': {
                std::string messageText;
                std::cout << "Enter message (max " << MAX_MESSAGE_LENGTH << " characters): ";
                std::cin.ignore(); // Очистка символа новой строки от предыдущего ввода
                std::getline(std::cin, messageText);
                
                if (messageText.length() >= MAX_MESSAGE_LENGTH) {
                    messageText = messageText.substr(0, MAX_MESSAGE_LENGTH - 1);
                }
                
                std::cout << "Waiting for an empty slot..." << std::endl;
                WaitForSingleObject(hEmptySemaphore, INFINITE);
                WaitForSingleObject(hMutex, INFINITE);
                
                FileHeader header = ReadFileHeader(hFile);
                
                Message message;
                ZeroMemory(&message, sizeof(Message));
                strncpy_s(message.text, messageText.c_str(), messageText.length());
                message.isOccupied = true;
                
                WriteMessage(hFile, header.writeIndex, message);
                
                header.writeIndex = (header.writeIndex + 1) % header.totalSlots;
                header.messageCount++;
                WriteFileHeader(hFile, header);
                
                ReleaseMutex(hMutex);
                ReleaseSemaphore(hFilledSemaphore, 1, NULL);
                
                std::cout << "Message sent: " << messageText << std::endl;
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
    
    return 0;
}