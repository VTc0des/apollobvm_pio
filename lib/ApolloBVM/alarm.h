/* #include "etl/delegate.h" */
/* #include "etl/vector.h" */
#include "panel.h"
#include "ventproperties.h"
/* #include <ArduinoSTL.h> */

/* #ifdef SAMD */
/* #include <vector> */
/* #else */
/* #include <ArduinoSTL.h> */
/* #include <estd/functional.h> */
/* #include <estd/vector.h> */
/* using AlarmFunc = estd::function<bool(VentIO *)>; */
/* #endif */

/* using AlarmFunc = etl::delegate<bool(VentIO *)>; */
/* using AlarmFunc = std::function<bool(VentIO *)>; */
/* using AlarmFunc = std::function<VentIO *, bool>; */
using AlarmFunc = bool (*)(const VentIO &vio);

enum Color { red, orange, green };

class Alarm {

public:
  Alarm(AlarmFunc condition, bool blink_led = true, Color led_color = red,
        bool stop_running = true, bool sound_buzzer = true,
        int display_time = -1, bool cancelable = false, String top_text = "top",
        String bottom_text = "bottom")
      : condition(condition), blink_led(blink_led), led_color(led_color),
        stop_running(stop_running), sound_buzzer(sound_buzzer),
        display_time(display_time), cancelable(cancelable), top_text(top_text),
        bottom_text(bottom_text) {}

  // Lambda function evaluating to true when alarm is triggered.
  AlarmFunc condition;

  // Alarm actions to take.
  bool blink_led;
  Color led_color;
  bool stop_running;
  bool sound_buzzer;

  // Alarm display parameters.
  int display_time;
  bool cancelable;
  String top_text;
  String bottom_text;

  // Alarm state variables.
  /* bool prev_triggered; */
  int time_triggered;
  /* int cycles_triggered; */
};

class AlarmManager {

public:
  AlarmManager(const VentIO &vio) : _vio(vio), _prev_time(0) {}

  void addAlarm(int idx, const Alarm &a);
  void addAlarm(int idx, AlarmFunc condition, bool blink_led, Color led_color,
                bool stop_running, bool sound_buzzer, int display_time,
                bool cancelable, String top_text, String bottom_text);
  bool evaluate();

  AlarmPanel *getAlarmPanel();

private:
  // Ventilator IO reference.
  const VentIO &_vio;

  // Array of alarms to evaluate.
  Alarm *_alarms;
  int _alarms_len;

  // Alarm Panel instance.
  AlarmPanel _alarm_panel;

  // Number of mashine cycles alarm condition has to be valid to be
  // considered triggered.
  /* const int time_until_triggered = 100; */

  // Number of machine cycles which alarm has to be triggered before we
  // display alarm.
  /* const int cycles_until_display = 3; */

  // Number of miliseconds an alarm must be triggered before we display.
  const int _time_until_display = 500;

  // Previous time alarm manager was run.
  int _prev_time;
};
