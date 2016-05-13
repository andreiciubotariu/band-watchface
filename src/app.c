#include "watchface_preferences.h"

#include <pebble.h>

#define MASK_CONTAINS_FLAG(mask, flag) (((mask) & (flag)) == (flag))
#define APP_TICK_UNITS (MINUTE_UNIT | DAY_UNIT)
#define APP_TIME_BUFFER_MAX_SIZE (8)
#define APP_DATE_BUFFER_MAX_SIZE (20)

typedef struct tm Time;

typedef struct AppData {
  WatchfacePrefs prefs;
  char time_buffer[APP_TIME_BUFFER_MAX_SIZE];
  char date_buffer[APP_DATE_BUFFER_MAX_SIZE];
  GFont number_font;
  GFont date_font;
  Window *window;
  Layer *root_layer;
} AppData;

static AppData s_data;

static void prv_window_layer_update_proc(Layer *layer, GContext *context) {
  const GRect layer_bounds = layer_get_bounds(layer);

  // background
  graphics_context_set_fill_color(context, s_data.prefs.background_color);
  graphics_fill_rect(context, layer_bounds, 0, GCornerNone);

  const int band_height = 50;
  const int band_top_bottom_padding = (layer_bounds.size.h - band_height) / 2;
  const int band_left_right_padding = -2;
  const GRect band_rect = grect_inset(layer_bounds, GEdgeInsets2(band_top_bottom_padding,
                                      band_left_right_padding));
  graphics_context_set_fill_color(context, s_data.prefs.band_color);
  graphics_fill_rect(context, band_rect, 0, GCornerNone);

  // time
  GRect text_bounds = band_rect;
  const int time_text_padding_reduction = 3;
  text_bounds.origin.y -= time_text_padding_reduction;
  graphics_context_set_text_color(context, s_data.prefs.time_text_color);
  graphics_draw_text(context, s_data.time_buffer, s_data.number_font, text_bounds,
                     GTextOverflowModeFill , GTextAlignmentCenter, NULL);

  // date
  text_bounds.origin.y += band_height;
  graphics_context_set_text_color(context, s_data.prefs.date_text_color);
  graphics_draw_text(context, s_data.date_buffer, s_data.date_font, text_bounds,
                     GTextOverflowModeFill , GTextAlignmentCenter, NULL);
}

static void prv_tick_handler(Time *tick_time, TimeUnits units_changed) {

  if (MASK_CONTAINS_FLAG(units_changed, MINUTE_UNIT)) {
    strftime(s_data.time_buffer, APP_TIME_BUFFER_MAX_SIZE, clock_is_24h_style() ? "%H:%M" : "%I:%M",
             tick_time);
  }
  if (MASK_CONTAINS_FLAG(units_changed, DAY_UNIT)) {
    const char *date_format = watchface_preferences_get_date_format(s_data.prefs.date_format);
    strftime(s_data.date_buffer, APP_DATE_BUFFER_MAX_SIZE, date_format, tick_time);
  }
  layer_mark_dirty(s_data.root_layer);
}

static void prv_forced_time_format_update(void) {
  const time_t temp = time(NULL);
  Time *tick_time = localtime(&temp);
  prv_tick_handler(tick_time, APP_TICK_UNITS);
}

static void prv_inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Received data from PebbleKit");

  WatchfacePrefs prefs;
  if (watchface_preferences_create_from_dict(&prefs, iterator)) {
    s_data.prefs = prefs;
    watchface_preferences_set_prefs(&prefs);
    prv_forced_time_format_update();
  }
}

static void prv_init(void) {
  s_data.number_font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_data.date_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
	s_data.window = window_create();

  Window *window = s_data.window;
  Layer *window_layer = window_get_root_layer(window);
  s_data.root_layer = window_layer;
  layer_set_update_proc(window_layer, prv_window_layer_update_proc);

  if (!watchface_preferences_get_prefs(&s_data.prefs)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not get watch prefs!");
  }

  prv_forced_time_format_update();
  tick_timer_service_subscribe(APP_TICK_UNITS, prv_tick_handler);

  app_message_register_inbox_received(prv_inbox_received_callback);
  const int inbox_size = 128;
  const int outbox_size = 0;
  app_message_open(inbox_size, outbox_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void prv_deinit(void) {
	window_destroy(s_data.window);
}

int main(void) {
	prv_init();
  app_event_loop();
	prv_deinit();
}
