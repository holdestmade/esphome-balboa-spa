#pragma once
#include <string>

#include "esphome/components/text_sensor/text_sensor.h"
#include "../balboaspa.h"

namespace esphome
{
  namespace balboa_spa
  {

    class SpaFaultTextSensor : public text_sensor::TextSensor
    {
    public:
      void set_parent(BalboaSpa *parent);
      void update(SpaFaultLog *spaFaultLog);

    private:
      std::string last_payload_;
    };

  } // namespace balboa_spa
} // namespace esphome
