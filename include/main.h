#ifndef __MAIN_H__
#define __MAIN_H__

#include <M5Unified.h> // M5Unifiedライブラリをプログラムで使用可能にします。

// LCD
#define LCD_LARGE_BAR_WIDTH (10)
#define LCD_LARGE_BAR_LENGTH (30)
#define LCD_LARGE_BAR_CORNER_RADIUS (6)
#define LCD_LARGE_BAR_GAP (LCD_LARGE_BAR_WIDTH >> 1)

#define LCD_SMALL_BAR_WIDTH (4)
#define LCD_SMALL_BAR_LENGTH (12)
#define LCD_SMALL_BAR_CORNER_RADIUS (3)
#define LCD_SMALL_BAR_GAP (LCD_SMALL_BAR_WIDTH >> 1)

#define LCD_DIGITS_CLEAR_ELM_NO (8)

#define LCD_DISP_BLINK_LOOP_CNT (1)

#define NTP_ACCESS_MS_INTERVAL (300000)

#define LCD_CLOCK_YMD_DISP_Y_POS (10)
// #define LCD_CLOCK_MD_STR_DISP_Y_POS (45)
// #define LCD_CLOCK_HM_DISP_Y_POS (100)
#define LCD_CLOCK_PM_STR_DISP_Y_POS (175)
#define LCD_CLOCK_ICON_DISP_Y_POS (200)
#define LCD_CLOCK_WEEK_STR_DISP_Y_POS (220)

#define LCD_TEMPERATURE_DISP_X_POS (35)
#define LCD_TEMPERATURE_DISP_Y_POS (20)
#define LCD_CLOCK_MD_STR_DISP_Y_POS (160)
#define LCD_CLOCK_HM_DISP_Y_POS (200)

// スクリーンの解像度は 横320 x 高さ240 で、左上が原点(0, 0)です
const uint8_t digits_normal[] =
    {
        0b00111111, // 0
        0b00110000, // 1
        0b01101101, // 2
        0b01111001, // 3
        0b01110010, // 4
        0b01011011, // 5
        0b01011111, // 6
        0b00110011, // 7
        0b01111111, // 8
        0b01111011, // 9
        0b00000000, // off
};

class BlinkCount
{
private:
  uint32_t count;

public:
  void incrementCount(void);
  void resetCount(void);
  boolean isHideDisplay(void);
};

// Date-time Screen
class SystemClock
{
private:
  struct tm time_info;
  time_t timer;

public:
  uint32_t year = 0;
  uint32_t month = 0;
  uint32_t day = 0;
  uint32_t hour = 0;
  uint32_t minute = 0;
  uint32_t week_day = 0;
  uint32_t second = 0;
  uint32_t prev_year = 0;
  uint32_t prev_month = 0;
  uint32_t prev_day = 0;
  uint32_t prev_hour = 0;
  uint32_t prev_minute = 0;
  uint32_t prev_week_day = 0;
  uint32_t prev_second = 0;
  void backupCurrentTime(void);
  void updateByNtp(void);
  void updateBySoftTimer(uint32_t elasped_second);
};

BlinkCount cl_blink_count;
SystemClock cl_system_clock;

#endif // __MAIN_H__