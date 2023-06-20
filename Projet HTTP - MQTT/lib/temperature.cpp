#include <Arduino.h>

#include <soc/rtc.h>

#define M_CALPOINT1_CELSIUS 23.0f
#define M_CALPOINT1_RAW 163600.0f
#define M_CALPOINT2_CELSIUS -20.0f
#define M_CALPOINT2_RAW 183660.0f

float readTemp(bool printRaw = false)
{
  uint64_t value = rtc_time_get();
  delay(100);
  value = (rtc_time_get() - value);

  if (printRaw)
  {
    printf("%s: raw value is: %llu\r\n", __FUNCTION__, value);
  }

  return ((float)value * 10.0 - M_CALPOINT1_RAW) * (M_CALPOINT2_CELSIUS - M_CALPOINT1_CELSIUS) / (M_CALPOINT2_RAW - M_CALPOINT1_RAW) + M_CALPOINT1_CELSIUS;
}
