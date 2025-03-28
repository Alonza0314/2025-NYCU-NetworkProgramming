# project-1-Alonza0314: npshell

- [steps](#steps)
  - [1. 基本shell架構](#1-基本shell架構)
  - [2. 一般pipe](#2-一般pipe)
  - [3. 編號pipe](#3-編號pipe)
  - [4. 檔案重新定向](#4-檔案重新定向)
  - [5. 優化、測試](#5-優化測試)

## Steps

### 1. 基本shell架構

- 建立一個能夠讀取輸入、解析指令，並執行基本命令的 Shell。
- 處理 內建指令(built-in commands):
  - setenv
  - printenv
  - exit
- 處理 未知指令，輸出 Unknown command: [command] 到標準錯誤。

### 2. 一般pipe

- 支援 cmd1 | cmd2, 讓第一個指令的輸出成為第二個指令的輸入。

### 3. 編號pipe

- 實作 cmd |N, 讓輸出傳給 第 N 行之後的指令。
- 實作 cmd !N, 讓 STDOUT 和 STDERR 傳給 第 N 行之後的指令。

### 4. 檔案重新定向

- 支援 cmd > file.txt, 將輸出寫入檔案。

### 5. 優化、測試

- 處理殭屍進程: 使用 waitpid(-1, NULL, WNOHANG) 來確保子行程正確回收。
- 確保管線資料不遺失: 測試大量輸入 cat large_file.txt | number 是否正確輸出。
- 確保大指令數量時仍能正常執行: 測試 ls | cat | cat | ... | cat 是否超過 process limit。
- 確保在 NP 伺服器可運行: 測試 printenv PATH 是否正確顯示 bin/:./。
