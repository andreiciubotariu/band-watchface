#include "watchface_preferences.h"

#include <pebble.h>

#define MASK_CONTAINS_FLAG(mask, flag) (((mask) & (flag)) == (flag))
#define TIME_BUFFER_MAX_SIZE (8)
#define DATE_BUFFER_MAX_SIZE (20)

typedef struct tm Time;
  
static struct {
  WatchfacePrefs prefs;
  char time_buffer[TIME_BUFFER_MAX_SIZE];
  char date_buffer[DATE_BUFFER_MAX_SIZE];
  GFont number_font;
  GFont date_font;
  Layer *root_layer;
} s_data;

static Window *s_window;

static void prv_window_layer_update_proc(Layer *layer, GContext *context) {
  const GRect layer_bounds = layer_get_bounds(layer);
  
  // background
  graphics_context_set_fill_color(context, s_data.prefs.background_color);
  graphics_fill_rect(context, layer_bounds, 0, GCornerNone);
  
  const int band_height = 50;
  const int band_top_bottom_padding = (layer_bounds.size.h - band_height) / 2;
  const int band_left_right_padding = -2;
  const GRect band_rect = grect_inset(layer_bounds, GEdgeInsets2(band_top_bottom_padding, band_left_right_padding));
  graphics_context_set_fill_color(context, s_data.prefs.band_color);
  graphics_fill_rect(context, band_rect, 0, GCornerNone);   
  
  // time
  GRect text_bounds = band_rect;
  const int time_text_padding_reduction = 3;
  text_bounds.origin.y -= time_text_padding_reduction;
  graphics_context_set_text_color(context, s_data.prefs.time_text_color);
  graphics_draw_text(context, s_data.time_buffer, s_data.number_font, text_bounds, GTextOverflowModeFill , GTextAlignmentCenter, NULL);
  
  // date
  text_bounds.origin.y += band_height;
  graphics_context_set_text_color(context, s_data.prefs.date_text_color);
  graphics_draw_text(context, s_data.date_buffer, s_data.date_font, text_bounds, GTextOverflowModeFill , GTextAlignmentCenter, NULL);
}

static void prv_tick_handler(Time *tick_time, TimeUnits units_changed) {
  
  if (MASK_CONTAINS_FLAG(units_changed, MINUTE_UNIT)) {
    strftime(s_data.time_buffer, TIME_BUFFER_MAX_SIZE, clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  }
  if (MASK_CONTAINS_FLAG(units_changed, DAY_UNIT)) {
    const char *date_format = watchface_preferences_get_date_format(s_data.prefs.date_format);
    strftime(s_data.date_buffer, DATE_BUFFER_MAX_SIZE, date_format, tick_time);
  }
  layer_mark_dirty(s_data.root_layer);
}

static void prv_init(void) {
  s_data.number_font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_data.date_font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
	s_window = window_create();
  
  Layer *window_layer = window_get_root_layer(s_window);
  s_data.root_layer = window_layer;
  layer_set_update_proc(window_layer, prv_window_layer_update_proc);
  
  if (!watchface_preferences_get_prefs(&s_data.prefs)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not get watch prefs!");
  }
	
  const TimeUnits tick_units = (MINUTE_UNIT | DAY_UNIT);
  const time_t temp = time(NULL);
  Time *tick_time = localtime(&temp);
  prv_tick_handler(tick_time, tick_units);
  tick_timer_service_subscribe(tick_units, prv_tick_handler);
	
	const bool animated = true;
	window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
	window_destroy(s_window);
}

int main(void) {
	prv_init();
	app_event_loop();
	prv_deinit();
}
