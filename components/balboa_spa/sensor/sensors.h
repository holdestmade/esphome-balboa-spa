#pragma once

#include <string>

#include "esphome/components/sensor/sensor.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class BalboaSpaSensors : public sensor::Sensor
    {
    public:
      enum class BalboaSpaSensorType : uint8_t
      {
        BLOWER = 1,
        HIGHRANGE = 2,
        CIRCULATION = 3,
        RESTMODE = 4,
        HEATSTATE = 5,
        FILTER1_REMAINING = 6,
        FILTER2_REMAINING = 7,
      };

    public:
      BalboaSpaSensors() {};
      void update(SpaState *spaState);

      void set_parent(BalboaSpa *parent);
      void set_sensor_type(BalboaSpaSensorType _type) { sensor_type = _type; }

    private:
      BalboaSpaSensorType sensor_type;
      BalboaSpa *parent_ = nullptr;
      uint32_t last_update_time_ = 0;
    };

  } // namespace balboa_spa
} // namespace esphome
