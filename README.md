# 智慧門鎖應用

## 說明

這是一個使用 Arduino Uno 搭配 ESP-01 的智慧門鎖應用，具有密碼驗證、警報和通知功能。使用 4x3 薄膜鍵盤、LCD 顯示器、光敏電阻和蜂鳴器等元件。

## 主要功能

1. 密碼鎖定和解鎖
   - 自定義密碼
   - 密碼重置功能

2. 安全機制
   - 最多允許 3 次密碼嘗試
   - 達到最大嘗試次數會觸發蜂鳴器警報聲
   - 解鎖門鎖時會發送通知至 LINE (注意: 需先設定IFTTT webhook)

3. 環境感知
   - 使用光敏電阻檢測環境光線
   - LED 根據光線變化

## 硬體需求

- Arduino Uno
- ESP-01
- 4x3 薄膜鍵盤
- 1602 LCD 顯示器
- 蜂鳴器
- LED
- 光敏電阻
- 繼電器
- 電子鎖機構(電磁閥)

## Arduino 程式庫

- LiquidCrystal_I2C
- SoftwareSerial
- Wire
- Keypad

## 使用說明

0. 修改配置：
    - 'WIFI_SSID' 'WIFI_PASSWORD' Wi-Fi 名稱和密碼
    - 'IFTTT_URI' IFTTT Webhook 密鑰

1. 開機後選擇模式：
   - 模式 1：重置密碼
   - 模式 2：解鎖門鎖
   - 按 '#' 鍵確定/鎖定
   - 按 '*' 鍵清除

2. 密碼重置：
   - 輸入當前密碼 (預設為 8888)
   - 輸入新密碼 (最大長度為 10)
   - 密碼更新成功
   
3. 門鎖解鎖：
   - LINE 收到訊息通知
   - 輸入正確密碼解鎖
   - `#` 鎖門
   - 3 次錯誤嘗試觸發警報