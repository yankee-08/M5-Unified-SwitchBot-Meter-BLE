# M5-Unified-SwitchBot-Meter-BLE

SwitchBot温湿度計とM5StackシリーズをBLE連携させたアプリです。  
元々、過去にArduino IDEで作成していたものを`PlatformIO`＋`M5Unified`の組み合わせで作り直しています。  
以下のQiita記事に、Arduino IDEで作成した際の手順をまとめています。  

[SwitchBot 温湿度計のBLEデータをM5Stackで読み取って画面に表示する](https://qiita.com/yankee/items/f1e1fd47a1a3e83501e4)

<img src="https://github.com/yankee-08/M5-Unified-SwitchBot-Meter-BLE/assets/130916826/b8a9a3e4-51fb-4666-85db-158ff05e19ee" width="600" />

## Description

- 以前、作成した物を`M5Unified.h`用に一部修正した物をGitHubに載せています
- SwitchBot温湿度計から送信されているBLEのアドバタイジングデータをM5Stackで受信
- 受信したデータから、温湿度に関するデータを抜き出し、ディスプレイ上に表示

## Requirement
### Hardware

ディスプレイ付きM5Stackシリーズ  
Unifiedライブラリを使用しているため、M5Stackシリーズに書き込みはできますが、画面サイズが320 x 240のモデルを基準に作成しているため、他のモデルだとうまく動作しないと思います。

動作確認済み機種

- M5Stack Core2
- M5Stack Fire

### Software

- Visual Studio Code
- PlatformIO

## Usage

- [config.h](https://github.com/yankee-08/M5-Unified-SwitchBot-Meter-BLE/blob/main/include/config.h) 内の`{_SSID_}`,`{_key_}`,`{_BLUETOOTH_ADDR_}`を自宅の環境に合わせて設定してください
- 背景色を変えたい場合は、`bkground_color`の変数を適宜変更してください
- 予め定義されている色については、以下のページを参照（// New color definitions use for all my librariesの部分）
  https://github.com/m5stack/M5Stack/blob/master/src/utility/In_eSPI.h

## License

[MIT](https://github.com/yankee-08/M5-Unified-SwitchBot-Meter-BLE/blob/main/LICENSE)  

## Author

[yankee](https://github.com/yankee-08)  
