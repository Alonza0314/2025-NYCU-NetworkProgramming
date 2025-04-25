#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

int main() {
    // 刪除已存在的 FIFO
    unlink("test_fifo");
    
    // 創建 FIFO
    if (mkfifo("test_fifo", 0666) == -1) {
        perror("mkfifo");
        return 1;
    }
    sleep(5); 
    std::cout << "等待讀取端打開..." << std::endl;

    // 以寫入模式打開 FIFO
    int writeFd = open("test_fifo", O_WRONLY);
    if (writeFd == -1) {
        perror("open write");
        return 1;
    }
    
    std::cout << "連接成功！" << std::endl;
    
    const char* message = "Hello from writer process!";
    ssize_t bytesWritten = write(writeFd, message, strlen(message));
    if (bytesWritten == -1) {
        perror("write");
        close(writeFd);
        return 1;
    }
    std::cout << "寫入 " << bytesWritten << " 字節到 FIFO" << std::endl;

    close(writeFd);
    return 0;
}