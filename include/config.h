#ifndef __CONFIG_H__
#define __CONFIG_H__

// Wi-Fi
const char *ssid = "{_SSID_}";
const char *password = "{_key_}";

// NTP
const char *ntp_server_1st = "ntp.nict.jp";
const char *ntp_server_2nd = "time.google.com";
const long gmt_offset_sec = 9 * 3600; // 時差（秒換算）
const int daylight_offset_sec = 0;    // 夏時間

// LCD
const uint16_t bkground_color = TFT_SILVER;

// BLE
const String TargetAddressStr = "{_BLUETOOTH_ADDR_}";

#endif // __CONFIG_H__