#include <BLEDevice.h>
#include <BLEScan.h>
#include <WiFi.h>
// Local
#include "main.h"
#include "config.h"

/*
/ 変数宣言
*/

BLEScan *pBLEScan;

boolean is_state_changed = true;

void drawNumberNormal(uint8_t x_start, uint8_t y_start, uint8_t number, uint8_t bar_width, uint8_t bar_length, uint8_t bar_gap, uint8_t corner_radius, uint16_t color_value)
{
  if (number > 10)
  {
    number = 10;
  }
  // top
  if (digits_normal[number] & 0b0000000000000001)
    M5.Display.fillRoundRect(x_start, y_start, bar_length, bar_width, corner_radius, color_value);
  // upper-left
  if (digits_normal[number] & 0b0000000000000010)
    M5.Display.fillRoundRect((x_start - bar_gap * 2), (y_start + bar_gap), bar_width, bar_length, corner_radius, color_value);
  // under-left
  if (digits_normal[number] & 0b0000000000000100)
    M5.Display.fillRoundRect((x_start - bar_gap * 2), (y_start + bar_gap + bar_length * 1), bar_width, bar_length, corner_radius, color_value);
  // bottom
  if (digits_normal[number] & 0b0000000000001000)
    M5.Display.fillRoundRect(x_start, (y_start + bar_length * 2), bar_length, bar_width, corner_radius, color_value);
  // under-right
  if (digits_normal[number] & 0b0000000000010000)
    M5.Display.fillRoundRect((x_start + bar_length), (y_start + bar_gap + bar_length * 1), bar_width, bar_length, corner_radius, color_value);
  // upper-right
  if (digits_normal[number] & 0b0000000000100000)
    M5.Display.fillRoundRect((x_start + bar_length), (y_start + bar_gap), bar_width, bar_length, corner_radius, color_value);
  // center
  if (digits_normal[number] & 0b0000000001000000)
    M5.Display.fillRoundRect(x_start, (y_start + bar_length * 1), bar_length, bar_width, corner_radius, color_value);
}

// Blink
void BlinkCount::incrementCount(void)
{
  ++count;
}

void BlinkCount::resetCount(void)
{
  count = 0;
}

boolean BlinkCount::isHideDisplay(void)
{
  if ((count / LCD_DISP_BLINK_LOOP_CNT % 2) == 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

void SystemClock::backupCurrentTime(void)
{
  prev_year = year;
  prev_month = month;
  prev_day = day;
  prev_hour = hour;
  prev_minute = minute;
  prev_week_day = week_day;
  prev_second = second;
}

void SystemClock::updateBySoftTimer(uint32_t elasped_second)
{
  struct tm *local_time;
  time_t timer_add = timer + elasped_second;
  local_time = localtime(&timer_add);
  year = local_time->tm_year + 1900;
  month = local_time->tm_mon + 1;
  day = local_time->tm_mday;
  hour = local_time->tm_hour;
  minute = local_time->tm_min;
  week_day = local_time->tm_wday;
  second = local_time->tm_sec;
}

void SystemClock::updateByNtp(void)
{
  Serial.println("---NTP ACCESS---");
  if (!getLocalTime(&time_info))
  {
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    week_day = 0;
    second = 0;
    timer = 0;
  }
  else
  {
    year = time_info.tm_year + 1900;
    month = time_info.tm_mon + 1;
    day = time_info.tm_mday;
    hour = time_info.tm_hour;
    minute = time_info.tm_min;
    week_day = time_info.tm_wday;
    second = time_info.tm_sec;
    timer = mktime(&time_info);
  }
}

int32_t calc_color(int32_t lo_temp, int32_t hi_temp, int32_t lo_temp_color, int32_t hi_temp_color, int32_t now_temp)
{
  if (now_temp > hi_temp)
  {
    now_temp = hi_temp;
  }
  else if (now_temp < lo_temp)
  {
    now_temp = lo_temp;
  }

  int32_t color = 0;
  if (lo_temp != hi_temp)
  {
    double y = (double)(now_temp - lo_temp) * double(hi_temp_color - lo_temp_color) / double(hi_temp - lo_temp) + lo_temp_color;
    color = round(y);
  }
  else
  {
    color = lo_temp_color;
  }

  return color;
}

void setup()
{
  auto cfg = M5.config();

  // 設定ファイル詳細：https://docs.m5stack.com/ja/api/m5unified/m5unified_appendix#m5config
  cfg.serial_baudrate = 115200;
  cfg.output_power = true;

  M5.begin(cfg); // 設定した値でデバイスを開始する。

  M5.Display.setBrightness(100);

  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(100);
  }

  M5.Display.fillScreen(TFT_WHITE);
  cl_blink_count.resetCount();
  delay(1000);

  // init time setting
  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server_1st, ntp_server_2nd);
  struct tm time_info;
  if (!getLocalTime(&time_info))
  {
    M5.Display.fillScreen(TFT_RED);
    delay(3000);
  }
  WiFi.disconnect(true);

  Serial.println("Hello world");
  Serial.println("Starting BLE work!");

  BLEDevice::init("hoge");

  pBLEScan = BLEDevice::getScan(); // create new scan
  pBLEScan->setInterval(1000);
  pBLEScan->setWindow(1000);
  pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster

  delay(1000);
}

void loop()
{
  static boolean is_state_changed = true;
  static boolean ntp_access_flag = true;
  static uint32_t base_milli_time;
  uint32_t elasped_second = 0;
  uint32_t diff_milli_time = 0;

  static int32_t integer_part_temperature = 0;
  static int32_t decimal_part_temperature = 0;
  static double temperature = 0.0;
  static uint32_t humidity = 0;
  static uint32_t battery_level = 0;
  delay(1000);
  M5.update();

  Serial.println("Scan start!");
  BLEScanResults foundDevices = pBLEScan->start(2, false);
  Serial.println("Scan end!");
  uint32_t dev_count = foundDevices.getCount(); // 受信したデバイス数を取得
  Serial.print("Devices found: ");
  Serial.println(dev_count);

  for (int iDevNo = 0; iDevNo < dev_count; iDevNo++)
  { // 受信したデータに対して
    BLEAdvertisedDevice device = foundDevices.getDevice(iDevNo);
    BLEAddress address = device.getAddress();
    // Serial.println(device.toString().c_str());
    // Serial.println(address.toString().c_str());

    // アドレス一致検索
    if (TargetAddressStr.compareTo(address.toString().c_str()) != 0)
    {
      // Serial.println(iDevNo);
      // Serial.println(":coninue");
      continue;
    }
    else
    {
      Serial.println("Device Found");
      if (device.haveManufacturerData())
      {
        char *pHexService = BLEUtils::buildHexData(nullptr, (uint8_t *)device.getManufacturerData().data(), device.getManufacturerData().length());
        std::string manufacture_data = pHexService;
        // Serial.print("manufacture_data:");
        // Serial.println((manufacture_data.c_str()));

        // 8bitごとに数値に変換する
        String full_str = manufacture_data.c_str();
        String split_str[19] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
        uint32_t split_num[19] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        char *endptr;
        for (int i = 0; i < 19; ++i)
        {
          split_str[i] = full_str.substring(i * 2, (i + 1) * 2);
          // Serial.println(split_str[i]);
          split_num[i] = strtoul(split_str[i].c_str(), &endptr, 16);
        }
        free(pHexService);
        // 温度算出
        const uint32_t mask_temp_sign = 0x80;    // 128
        const uint32_t mask_temp_integer = 0x7F; // 127
        const uint32_t mask_temp_decimal = 0x0F; // 15
        integer_part_temperature = split_num[16] & mask_temp_integer;
        decimal_part_temperature = split_num[15] & mask_temp_decimal;
        temperature = (double)integer_part_temperature + (double)decimal_part_temperature / 10;
        if ((split_num[16] & mask_temp_sign) == 0)
        {
          temperature = temperature * -1;
        }
        Serial.print("Temp.:");
        // Serial.print(integer_part_temperature);
        // Serial.print(".");
        // Serial.print(decimal_part_temperature);
        // Serial.print("[`C] / ");
        Serial.print(temperature);
        Serial.println("[`C] / ");

        // 湿度算出
        const uint32_t mask_humi = 0x7F; // 127
        humidity = split_num[17] & mask_humi;
        Serial.print("Humi.:");
        Serial.print(humidity);
        Serial.println("[%RH]");
        Serial.flush();
      }
      else
      {
        Serial.println("device NOT have ServiceData");
      }
    }
  }
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

  // 時刻取得
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(100);
  }
  if (ntp_access_flag == true)
  {
    base_milli_time = millis();
    Serial.print("base_milli_time:");
    Serial.println(base_milli_time);
    cl_system_clock.updateByNtp();
    ntp_access_flag = false;
  }
  else
  {
    diff_milli_time = millis() - base_milli_time;
    if (diff_milli_time > NTP_ACCESS_MS_INTERVAL)
    {
      ntp_access_flag = true;
    }
    elasped_second = diff_milli_time / 1000;
    cl_system_clock.updateBySoftTimer(elasped_second);
  }
  WiFi.disconnect(true);

  if (is_state_changed == true)
  {
    M5.Display.fillScreen(bkground_color);
    is_state_changed = false;
  }

  // 温度表示
  int32_t temp_red_value = calc_color(0, 30, 100, 250, (int)temperature);
  // Serial.print("RED[T]:");
  // Serial.println(temp_red_value);
  int32_t temp_blue_value = calc_color(0, 30, 250, 50, (int)temperature);
  // Serial.print("BLUE[T]:");
  // Serial.println(temp_blue_value);
  int32_t temp_green_value = 50;

  uint16_t temperature_font_color = M5.Display.color565(temp_red_value, temp_green_value, temp_blue_value);

  // 符号
  M5.Display.setTextColor(bkground_color);
  M5.Display.setTextSize(5);
  M5.Display.drawString("-", 10, 35);
  if (temperature < 0)
  {
    M5.Display.setTextColor(temperature_font_color);
    M5.Display.drawString("-", 10, 35);
  }
  // 整数部
  drawNumberNormal(55, 20, LCD_DIGITS_CLEAR_ELM_NO, LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, bkground_color);
  drawNumberNormal(55 + 65, 20, LCD_DIGITS_CLEAR_ELM_NO, LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, bkground_color);
  if ((integer_part_temperature / 10) != 0)
  {
    drawNumberNormal(55, 20, (integer_part_temperature / 10), LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, temperature_font_color);
  }
  drawNumberNormal(55 + 65, 20, (integer_part_temperature % 10), LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, temperature_font_color);
  M5.Display.setTextColor(temperature_font_color);
  M5.Display.setTextSize(7);
  M5.Display.drawString(".", 155, 40);
  // 小数部
  drawNumberNormal(55 + 140, 20, LCD_DIGITS_CLEAR_ELM_NO, LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, bkground_color);
  drawNumberNormal(55 + 140, 20, decimal_part_temperature, LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, temperature_font_color);
  M5.Display.setTextSize(6);
  M5.Display.drawString("C", 260, 50);
  M5.Display.fillEllipse(250, 55, 5, 5, temperature_font_color);

  M5.Display.fillRoundRect(10, 116, 300, 8, 4, TFT_WHITE);

  // 湿度表示
  int32_t humi_red_value = calc_color(30, 100, 160, 100, humidity);
  // Serial.print("RED[H]:");
  // Serial.println(humi_red_value);
  int32_t humi_blue_value = calc_color(30, 100, 60, 240, humidity);
  // Serial.print("BLUE[H]:");
  // Serial.println(humi_blue_value);
  int32_t humi_green_value = 120;

  uint16_t humidity_font_color = M5.Display.color565(humi_red_value, humi_green_value, humi_blue_value);

  diff_milli_time = millis() - base_milli_time;
  elasped_second = diff_milli_time / 1000;
  drawNumberNormal(140, 150, LCD_DIGITS_CLEAR_ELM_NO, LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, bkground_color);
  drawNumberNormal(140 + 65, 150, LCD_DIGITS_CLEAR_ELM_NO, LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, bkground_color);
  if ((humidity / 10) != 0)
  {
    drawNumberNormal(140, 150, (humidity / 10), LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, humidity_font_color);
  }
  drawNumberNormal(140 + 65, 150, (humidity % 10), LCD_LARGE_BAR_WIDTH, LCD_LARGE_BAR_LENGTH, LCD_LARGE_BAR_GAP, LCD_LARGE_BAR_CORNER_RADIUS, humidity_font_color);
  M5.Display.setTextColor(humidity_font_color);
  M5.Display.setTextSize(6);
  M5.Display.drawString("%", 260, 180);

  // 時刻表示
  // Month
  if (cl_system_clock.month != cl_system_clock.prev_month)
  {
    drawNumberNormal(10, LCD_CLOCK_MD_STR_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
    drawNumberNormal(35, LCD_CLOCK_MD_STR_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
  }
  drawNumberNormal(10, LCD_CLOCK_MD_STR_DISP_Y_POS, (cl_system_clock.month / 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  drawNumberNormal(35, LCD_CLOCK_MD_STR_DISP_Y_POS, (cl_system_clock.month % 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  M5.Display.drawLine(55, LCD_SMALL_BAR_LENGTH * 2 + LCD_CLOCK_MD_STR_DISP_Y_POS, 65, LCD_CLOCK_MD_STR_DISP_Y_POS, TFT_OLIVE);
  // Day
  if (cl_system_clock.day != cl_system_clock.prev_day)
  {
    drawNumberNormal(75, LCD_CLOCK_MD_STR_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
    drawNumberNormal(100, LCD_CLOCK_MD_STR_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
  }
  drawNumberNormal(75, LCD_CLOCK_MD_STR_DISP_Y_POS, (cl_system_clock.day / 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  drawNumberNormal(100, LCD_CLOCK_MD_STR_DISP_Y_POS, (cl_system_clock.day % 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  // Hour
  if (cl_system_clock.hour != cl_system_clock.prev_hour)
  {
    drawNumberNormal(10, LCD_CLOCK_HM_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
    drawNumberNormal(35, LCD_CLOCK_HM_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
  }
  drawNumberNormal(10, LCD_CLOCK_HM_DISP_Y_POS, (cl_system_clock.hour / 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  drawNumberNormal(35, LCD_CLOCK_HM_DISP_Y_POS, (cl_system_clock.hour % 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  // Sec
  if (cl_system_clock.minute != cl_system_clock.prev_minute)
  {
    drawNumberNormal(75, LCD_CLOCK_HM_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
    drawNumberNormal(100, LCD_CLOCK_HM_DISP_Y_POS, LCD_DIGITS_CLEAR_ELM_NO, LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, bkground_color);
  }
  drawNumberNormal(75, LCD_CLOCK_HM_DISP_Y_POS, (cl_system_clock.minute / 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  drawNumberNormal(100, LCD_CLOCK_HM_DISP_Y_POS, (cl_system_clock.minute % 10), LCD_SMALL_BAR_WIDTH, LCD_SMALL_BAR_LENGTH, LCD_SMALL_BAR_GAP, LCD_SMALL_BAR_CORNER_RADIUS, TFT_DARKGREY);
  if (cl_blink_count.isHideDisplay() == false)
  {
    M5.Display.fillEllipse(60, LCD_CLOCK_HM_DISP_Y_POS + 8, 2, 2, TFT_DARKGREY);
    M5.Display.fillEllipse(60, LCD_CLOCK_HM_DISP_Y_POS + 20, 2, 2, TFT_DARKGREY);
  }
  else
  {
    M5.Display.fillEllipse(60, LCD_CLOCK_HM_DISP_Y_POS + 8, 2, 2, bkground_color);
    M5.Display.fillEllipse(60, LCD_CLOCK_HM_DISP_Y_POS + 20, 2, 2, bkground_color);
  }

  cl_blink_count.incrementCount();
}
