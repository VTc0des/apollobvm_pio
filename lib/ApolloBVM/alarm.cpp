#include "alarm.h"

void AlarmManager::addAlarm(Alarm a) { alarms.push_back(a); }

void AlarmManager::addAlarm(AlarmFunc condition, bool blink_led,
                            Color led_color, bool stop_running,
                            bool sound_buzzer, int display_time,
                            bool cancelable, String top_text,
                            String bottom_text) {
  alarms.emplace_back(condition, blink_led, led_color, stop_running,
                      sound_buzzer, display_time, cancelable, top_text,
                      bottom_text);
}

bool AlarmManager::evaluate() {

  int delta_time = millis() - prev_time;
  prev_time = millis();

  for (Alarm &a : alarms) {
    // Check if each alarm has been triggered.
    if (a.condition(vio_ptr)) {
      // Add the delta time to the total time this alarm has been triggered.
      a.time_triggered += delta_time;

      if (a.time_triggered > time_until_display) {

        // TODO: Change the Alarm Panel to display this alarm.

        return true;
      }
    } else {
      // TODO: Add time that alarm hasn't been triggered for to know when to
      // reset the time_triggered counter.
    }
  }

  // No alarm should be displayed.
  return false;
}
