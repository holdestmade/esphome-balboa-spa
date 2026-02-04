#include "esphome.h"
#include "esphome/core/log.h"
#include "spa_water_heater.h"

namespace esphome
{
  namespace balboa_spa
  {

    water_heater::WaterHeaterTraits BalboaSpaWaterHeater::traits()
    {
      auto traits = water_heater::WaterHeaterTraits();
      traits.set_supported_modes({
        water_heater::WATER_HEATER_MODE_OFF,
        water_heater::WATER_HEATER_MODE_ECO,
        water_heater::WATER_HEATER_MODE_PERFORMANCE,
      });
      traits.set_supports_current_temperature(true);
      return traits;
    }

    void BalboaSpaWaterHeater::control(const water_heater::WaterHeaterCall &call)
    {
      if (!std::isnan(call.get_target_temperature()))
      {
        spa->set_temp(call.get_target_temperature());
      }

      if (call.get_mode().has_value())
      {
        auto requested_mode = *call.get_mode();

        bool is_in_rest = spa->get_restmode();

        if (requested_mode == water_heater::WATER_HEATER_MODE_OFF)
        {
          if (!is_in_rest)
          {
            ESP_LOGD("spa_water_heater", "Toggle from Ready to Rest");
            spa->toggle_heat();
          }
          return;
        }

        if (is_in_rest)
        {
          ESP_LOGD("spa_water_heater", "Toggle from Rest to Ready");
          spa->toggle_heat();
        }

        if (requested_mode == water_heater::WATER_HEATER_MODE_ECO)
        {
          spa->set_highrange(false);
        }
        else if (requested_mode == water_heater::WATER_HEATER_MODE_PERFORMANCE)
        {
          spa->set_highrange(true);
        }
      }
    }

    water_heater::WaterHeaterCallInternal BalboaSpaWaterHeater::make_call()
    {
      return water_heater::WaterHeaterCallInternal(this);
    }

    void BalboaSpaWaterHeater::set_parent(BalboaSpa *parent)
    {
      spa = parent;
      parent->register_listener([this](SpaState *spaState)
                                { this->update(spaState); });
    }

    bool inline is_diff_no_nan(float a, float b)
    {
      if (std::isnan(a))
      {
        return false;
      }
      return std::isnan(b) || b != a;
    }

    void BalboaSpaWaterHeater::update(SpaState *spaState)
    {
      bool needs_update = false;

      if (!spa->is_communicating())
      {
        this->target_temperature_ = NAN;
        this->current_temperature_ = NAN;
        return;
      }

      float target_temp = spaState->target_temp;
      needs_update = is_diff_no_nan(target_temp, this->target_temperature_) || needs_update;
      this->target_temperature_ = !std::isnan(target_temp) ? target_temp : this->target_temperature_;

      auto current_temp = spaState->current_temp;
      needs_update = is_diff_no_nan(current_temp, this->current_temperature_) || needs_update;
      this->current_temperature_ = !std::isnan(current_temp) ? current_temp : this->current_temperature_;

      water_heater::WaterHeaterMode new_mode;
      if (spaState->rest_mode == 1)
      {
        new_mode = water_heater::WATER_HEATER_MODE_OFF;
      }
      else
      {
        new_mode = spaState->highrange == 1 ? water_heater::WATER_HEATER_MODE_PERFORMANCE : water_heater::WATER_HEATER_MODE_ECO;
      }
      needs_update = new_mode != this->mode_ || needs_update;
      this->mode_ = new_mode;

      needs_update = this->last_update_time == 0 || needs_update;
      needs_update = this->last_update_time + 300000 < millis() || needs_update;

      if (needs_update)
      {
        this->publish_state();
        this->last_update_time = millis();
      }
    }

  } // namespace balboa_spa
} // namespace esphome
