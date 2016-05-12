#include "watchface_preferences.h"

#include <pebble.h>

#define SERIALIZED_PREFS_CURRENT_VERSION (1)

typedef struct  __attribute__((packed)) SerializedPrefs {
  uint16_t version;
  uint8_t background_color;
  uint8_t band_color;
  uint8_t time_text_color;
  uint8_t date_format;
  uint8_t date_text_color;
  uint8_t disconnect_indicator_color;
  bool vibe_on_disconnect;
} SerializedPrefs;

static const char *s_date_formats[DateFormatCount] = {
  [DateFormat_DayMonthYearDashes] = "%d-%m-%Y",
  [DateFormat_DayMonthYearSlashes] = "%d/%m/%Y",
  [DateFormat_MonthDayYearDashes] = "%m-%d-%Y",
  [DateFormat_MonthDayYearDaySlashes] = "%m/%d/%Y",
  [DateFormat_YearMonthDayDashes] = "%Y-%m-%d",
  [DateFormat_YearMonthDaySlashes] = "%Y/%m/%d",
};

bool watchface_preferences_get_prefs(WatchfacePrefs *data_out) {
  const WatchfacePrefs default_prefs = (WatchfacePrefs) {
    .background_color = GColorBlack,
    .band_color = GColorRed,
    .time_text_color = GColorWhite,
    .date_format = DateFormat_MonthDayYearDashes,
    .date_text_color = GColorWhite,
    .disconnect_indicator_color = GColorRed,
    .vibe_on_disconnect = true,
  };
  
  if (!data_out) {
    return false;
  }
  
  *data_out = default_prefs;
  return true;
}

const char *watchface_preferences_get_date_format(DateFormat format) {
 return s_date_formats[format];
}