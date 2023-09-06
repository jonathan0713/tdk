# 設定輸出的圖片格式和名稱
set terminal pngcairo enhanced font "arial,10" fontscale 1.0 size 600, 400
set output 'output.png'

# 設定圖表標題和軸標籤
set title "XY Data Plot"
set xlabel "X"
set ylabel "Y"

# 指定使用的數據檔案和欄位
plot "data.txt" using 1:2 with linespoints title "Data"
