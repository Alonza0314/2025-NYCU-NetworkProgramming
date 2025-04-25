#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

int main() {
    std::cout << "等待連接到 FIFO..." << std::endl;
    
    int fd = open("test_fifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    std::cout << "連接成功！等待數據..." << std::endl;

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
    if (bytesRead == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    std::cout << "讀取 " << bytesRead << " 字節: " << buffer << std::endl;

    close(fd);
    unlink("test_fifo");
    return 0;
}