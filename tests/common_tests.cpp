#include <gtest/gtest.h>
#include "../src/common.h"
#include <windows.h>
#include <string>

// Добавляем main для запуска тестов
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class FileOperationsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создание временного тестового файла
        hFile = CreateFileA("test_file.bin", GENERIC_READ | GENERIC_WRITE, 
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    void TearDown() override {
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
        }
        DeleteFileA("test_file.bin");
    }

    HANDLE hFile;
};

TEST_F(FileOperationsTest, HeaderReadWrite) {
    // Инициализация тестового заголовка
    FileHeader testHeader;
    testHeader.readIndex = 1;
    testHeader.writeIndex = 2;
    testHeader.messageCount = 3;
    testHeader.totalSlots = 4;
    
    // Запись заголовка
    WriteFileHeader(hFile, testHeader);
    
    // Чтение заголовка
    FileHeader readHeader = ReadFileHeader(hFile);
    
    // Сравнение
    EXPECT_EQ(readHeader.readIndex, testHeader.readIndex);
    EXPECT_EQ(readHeader.writeIndex, testHeader.writeIndex);
    EXPECT_EQ(readHeader.messageCount, testHeader.messageCount);
    EXPECT_EQ(readHeader.totalSlots, testHeader.totalSlots);
}

TEST_F(FileOperationsTest, MessageReadWrite) {
    // Создание заголовка файла для инициализации
    FileHeader header;
    header.readIndex = 0;
    header.writeIndex = 0;
    header.messageCount = 0;
    header.totalSlots = 5;
    WriteFileHeader(hFile, header);
    
    // Инициализация тестового сообщения
    Message testMessage;
    ZeroMemory(&testMessage, sizeof(Message));
    strcpy_s(testMessage.text, "Test Message");
    testMessage.isOccupied = true;
    
    // Запись сообщения
    WriteMessage(hFile, 0, testMessage);
    
    // Чтение сообщения
    Message readMessage = ReadMessage(hFile, 0);
    
    // Сравнение
    EXPECT_STREQ(readMessage.text, testMessage.text);
    EXPECT_EQ(readMessage.isOccupied, testMessage.isOccupied);
}

TEST_F(FileOperationsTest, CircularBufferOperation) {
    // Создание заголовка файла
    FileHeader header;
    header.readIndex = 0;
    header.writeIndex = 0;
    header.messageCount = 0;
    header.totalSlots = 3;
    WriteFileHeader(hFile, header);
    
    // Инициализация тестовых сообщений
    Message message1, message2, message3;
    ZeroMemory(&message1, sizeof(Message));
    ZeroMemory(&message2, sizeof(Message));
    ZeroMemory(&message3, sizeof(Message));
    
    strcpy_s(message1.text, "Message 1");
    strcpy_s(message2.text, "Message 2");
    strcpy_s(message3.text, "Message 3");
    
    message1.isOccupied = true;
    message2.isOccupied = true;
    message3.isOccupied = true;
    
    // Запись сообщений (заполнение буфера)
    // Запись первого сообщения
    WriteMessage(hFile, header.writeIndex, message1);
    header.writeIndex = (header.writeIndex + 1) % header.totalSlots;
    header.messageCount++;
    WriteFileHeader(hFile, header);
    
    // Запись второго сообщения
    WriteMessage(hFile, header.writeIndex, message2);
    header.writeIndex = (header.writeIndex + 1) % header.totalSlots;
    header.messageCount++;
    WriteFileHeader(hFile, header);
    
    // Запись третьего сообщения
    WriteMessage(hFile, header.writeIndex, message3);
    header.writeIndex = (header.writeIndex + 1) % header.totalSlots;
    header.messageCount++;
    WriteFileHeader(hFile, header);
    
    // Проверка чтения сообщений в порядке FIFO
    // Чтение первого сообщения
    Message readMessage1 = ReadMessage(hFile, header.readIndex);
    header.readIndex = (header.readIndex + 1) % header.totalSlots;
    header.messageCount--;
    WriteFileHeader(hFile, header);
    
    // Чтение второго сообщения
    Message readMessage2 = ReadMessage(hFile, header.readIndex);
    header.readIndex = (header.readIndex + 1) % header.totalSlots;
    header.messageCount--;
    WriteFileHeader(hFile, header);
    
    // Чтение третьего сообщения
    Message readMessage3 = ReadMessage(hFile, header.readIndex);
    header.readIndex = (header.readIndex + 1) % header.totalSlots;
    header.messageCount--;
    WriteFileHeader(hFile, header);
    
    // Проверка правильности чтения
    EXPECT_STREQ(readMessage1.text, message1.text);
    EXPECT_STREQ(readMessage2.text, message2.text);
    EXPECT_STREQ(readMessage3.text, message3.text);
    
    // Проверка, что буфер пуст
    FileHeader finalHeader = ReadFileHeader(hFile);
    EXPECT_EQ(finalHeader.messageCount, 0);
}