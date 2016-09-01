#pragma once

#include <pebble.h>

typedef enum DateFormat {
  DateFormat_DayMonthYearDashes,
  DateFormat_DayMonthYearSlashes,
  DateFormat_MonthDayYearDashes,
  DateFormat_MonthDayYearDaySlashes,
  DateFormat_YearMonthDayDashes,
  DateFormat_YearMonthDaySlashes,
  DateFormatCount,
} DateFormat;

typedef struct WatchfacePrefs {
  GColor background_color;
  GColor band_color;
  GColor time_text_color;
  GColor date_text_color;
  DateFormat date_format;
  GColor disconnect_indicator_color;
  bool vibe_on_disconnect;
} WatchfacePrefs;

bool watchface_preferences_get_prefs(WatchfacePrefs *data_out);

bool watchface_preferences_set_prefs(const WatchfacePrefs *prefs);

bool watchface_preferences_create_from_dict(WatchfacePrefs *out,
                                            const DictionaryIterator *iterator);

const char *watchface_preferences_get_date_format(const DateFormat format);
