# Tomasulo Algorithm - Readme file  

## 資料結構  
  vector, queue  

  RF( Register Field ) = { 0 , 2, 4, 6, 8 }  
  InitialQueue 將input的所有instruction依照順序放入  


## 演算法分為三個部分

一. Issue

從initialQueue抓出最前面的insturction排進Reservation Station.  ( RS預設提供三個給Add or Sub, 兩個給Mul or Div ), 共五個  

如果RS有空間則分成兩種Type  
- Type為Add or Sub放在前三個RS  
- Type為Mul or Div放在後兩個RS  

如果RS沒有空間, 則不issue  

二. Execute

查看RS裡面是否已經有instruction可以dispatch  
如果可以被dispatch則要查看目前運算單元是否被占用  
運算單元沒有被占用 - 可以則佔用運算單元, 其他同類型instruction必須等待  
運算單元被占用         - 繼續在RS等待, 直到運算單元被釋出  

三. WriteBack

當被dispatch的instruction開始運算後, 直到預設的運算cycle數(加法減法 - 2 cycles, 乘法 - 10 cycles, 除法 - 40 cycles) 到了後, 進行writeback  
writeback的值會被廣播到RS, RAT  
提供RS及RAT抓取(Capture)所需的值, 如果這個值是個舊值, 也就是已經不需要的值, 則不進行寫入  
如果是新值, 則將值寫入  

writeback完後代表instruction已經被執行完成, 所以可以將RS釋出給後續instruction使用  
