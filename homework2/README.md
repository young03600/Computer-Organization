# 簡易之Branch Predictor

一開始宣告存放historyBits的vector, size為2, 代表two bits history, 並將兩個bits都設為false  
enum type的Counter, 模擬counter的行為, 初始為SN, 代表 strong not taken  
在更改counter狀態前立即根據目前history狀態做推測動作, 再依據實際taken or not taken狀態更新counter的狀態  

如果某個counter狀態為SN, 實際為not taken, 則繼續讓SN狀態為SN  
如果某個counter狀態為ST, 實際為taken, 則繼續讓ST狀態為ST  
其他counter狀態則依照實際taken or not taken,  
如果taken, 則狀態往前, ex: 00->01, 01->10, 10->11  
如果not taken, 則狀態回復, ex: 01->00, 10->01, 11->10  
