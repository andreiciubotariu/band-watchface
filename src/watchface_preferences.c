#include "appkeys.h"
#include "watchface_preferences.h"

#include <pebble.h>

#define WATCHFACE_PREFS_KEY (0)
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

static const WatchfacePrefs s_default_prefs =  {
    .background_color = {GColorBlackARGB8},
    .band_color = {GColorRedARGB8},
    .time_text_color = {GColorWhiteARGB8},
    .date_format = DateFormat_MonthDayYearDashes,
    .date_text_color = {GColorWhiteARGB8},
    .disconnect_indicator_color = {GColorRedARGB8},
    .vibe_on_disconnect = true,
  };

bool watchface_preferences_get_prefs(WatchfacePrefs *data_out) {
  if (!data_out) {
    return false;
  }

  const size_t buffer_size = sizeof(SerializedPrefs);
  uint8_t buffer[buffer_size];

  status_t status = persist_read_data(WATCHFACE_PREFS_KEY, buffer, buffer_size);

  if (status == E_DOES_NOT_EXIST) {
    *data_out = s_default_prefs;
    return true;
  }

  size_t bytes_written = status;
  if (bytes_written < buffer_size) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Did not completely write buffer. Missing %u bytes",
            (buffer_size - bytes_written));
    return false;
  }

  SerializedPrefs *serialized_prefs = (SerializedPrefs *)buffer;
  APP_LOG(APP_LOG_LEVEL_ERROR, "Dateformat: %u", serialized_prefs->date_format);
  *data_out = (WatchfacePrefs) {
    .background_color = {serialized_prefs->background_color},
    .band_color = {serialized_prefs->band_color},
    .time_text_color = {serialized_prefs->time_text_color},
    .date_format = serialized_prefs->date_format,
    .date_text_color = {serialized_prefs->date_text_color},
    .disconnect_indicator_color = {serialized_prefs->date_text_color},
    .vibe_on_disconnect = serialized_prefs->vibe_on_disconnect,
  };

  return true;
}

bool watchface_preferences_set_prefs(const WatchfacePrefs *prefs) {
  SerializedPrefs serialized_prefs = (SerializedPrefs) {
    .background_color = prefs->background_color.argb,
    .band_color = prefs->band_color.argb,
    .time_text_color = prefs->time_text_color.argb,
    .date_format = prefs->date_format,
    .date_text_color = prefs->date_text_color.argb,
    .disconnect_indicator_color = prefs->disconnect_indicator_color.argb,
    .vibe_on_disconnect = prefs->vibe_on_disconnect,
  };

  const size_t data_size = sizeof(SerializedPrefs);
  status_t status = persist_write_data(WATCHFACE_PREFS_KEY, &serialized_prefs, data_size);
  return (status == S_SUCCESS);
}

static uint8_t prv_get_uint8_from_dict(DictionaryIterator *iterator, AppKey app_key,
                                       uint8_t default_value) {
  Tuple *tuple = dict_find(iterator, app_key);
  if (!tuple) {
    return default_value;
  }

  return tuple->value->uint8;
}

static GColor prv_get_color_from_dict(DictionaryIterator *iterator, AppKey app_key,
                                      GColor default_color) {
  Tuple *tuple = dict_find(iterator, app_key);
  if (!tuple) {
    return default_color;
  }

  return GColorFromHEX(tuple->value->int32);
}

static int32_t prv_get_int32_from_dict(DictionaryIterator *iterator, AppKey app_key,
                                        int32_t default_value) {
  Tuple *tuple = dict_find(iterator, app_key);
  if (!tuple) {
    return default_value;
  }

  return tuple->value->int32;
}

static bool prv_get_bool_from_dict(DictionaryIterator *iterator, AppKey app_key,
                                   bool default_value) {
  Tuple *tuple = dict_find(iterator, app_key);
  if (!tuple) {
    return default_value;
  }

  return (bool) tuple->value->int16;
}

bool watchface_preferences_create_from_dict(WatchfacePrefs *prefs_out,
                                            DictionaryIterator *iterator) {
  if (!prefs_out) {
    return false;
  }

  *prefs_out = (WatchfacePrefs) {
    .background_color = prv_get_color_from_dict(iterator, AppKey_BackgroundColor,
                                                s_default_prefs.background_color),
    .band_color = prv_get_color_from_dict(iterator, AppKey_BandColor, s_default_prefs.band_color),
    .time_text_color = prv_get_color_from_dict(iterator, AppKey_TimeTextColor,
                                               s_default_prefs.time_text_color),
    .date_format = prv_get_int32_from_dict(iterator, AppKey_DateFormat,
                                           s_default_prefs.date_format),
    .date_text_color = prv_get_color_from_dict(iterator, AppKey_DateTextColor,
                                               s_default_prefs.date_text_color),
    .disconnect_indicator_color = prv_get_color_from_dict(iterator, AppKey_DisconnectIndicatorColor,
        s_default_prefs.disconnect_indicator_color),
    .vibe_on_disconnect = (bool) prv_get_uint8_from_dict(iterator, AppKey_VibeOnDisconnect,
                                                         s_default_prefs.vibe_on_disconnect),
  };

  return true;
}

const char *watchface_preferences_get_date_format(DateFormat format) {
  return s_date_formats[format];
}
