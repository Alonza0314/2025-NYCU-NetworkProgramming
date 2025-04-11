# project-2-Alonza0314: socket

- [Part 1](#part-1)
- [Part 2](#part-2)

## Part 1

- step 1：基本TCP server
  - server 監聽7001
  - 允許client連線並顯示‘%’
  - 處理client exit斷線
- step 2：整合npshell
  - 讓 Client 端可以輸入指令，並讓 Server 執行
  - 讓多個 Client 互不干涉（獨立變數、Pipe）
- step 3
  - 用 telnet 測試基本指令執行

## Part 2

- step 1：更改架構為select
- step 2：server指令（這個順序比較好做）
  - welcom message
  - login
  - logout
  - who
  - name
  - yell
  - tell
- step 3：user pipe指令
  - 當一個瘋狂接水管的無情水電工
  - 接水管接得很開心，結果忘記接水管的順序
  - 接錯水管，導致水管爆炸
  - 接錯水管，導致水管爆炸
  - 接錯水管，導致水管爆炸
  - 接錯水管，導致水管爆炸
  - 接錯水管，導致水管爆炸
