# project-3-Alonza0314: multi-proc

- [Steps](#Steps)
  - [1. select改成fork處理client](#1-select改成fork處理client)
  - [2. client資料用shm存放](#2-client資料用shm存放)
  - [3. shm](#3-shm)
  - [4. user pipe改為FIFO的方式](#4-user-pipe改為fifo的方式)

## Steps

### 1. select改成fork處理client

- 基於project 2的single-proc改成multi-proc

### 2. client資料用shm存放

- terminal查看shm:

    ```bash
    ipcs -m
    ```

- terminal刪除shm:

    ```bash
    ipcrm -m <shmid>
    ```

### 3. shm

- 使用shm儲存user結構
- 透過shm+signal做broadcast
- 透過shm+signal做tell功能

### 4. user pipe改為FIFO的方式
