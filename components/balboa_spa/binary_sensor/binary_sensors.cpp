#include "esphome/core/log.h"
#include "binary_sensors.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "BalboaSpa.binary_sensors";
        static const int MINUTES_PER_DAY = 24 * 60;

        static bool is_filter_cycle_running(const SpaFilterSettings *settings, uint8_t current_hour, uint8_t current_minute, bool filter_cycle_2)
        {
            uint8_t start_hour = settings->filter1_hour;
            uint8_t start_minute = settings->filter1_minute;
            uint8_t duration_hour = settings->filter1_duration_hour;
            uint8_t duration_minute = settings->filter1_duration_minute;

            if (filter_cycle_2)
            {
                if (!settings->filter2_enable)
                {
                    return false;
                }

                start_hour = settings->filter2_hour;
                start_minute = settings->filter2_minute;
                duration_hour = settings->filter2_duration_hour;
                duration_minute = settings->filter2_duration_minute;
            }

            const int start_total = static_cast<int>(start_hour) * 60 + static_cast<int>(start_minute);
            const int duration_total = static_cast<int>(duration_hour) * 60 + static_cast<int>(duration_minute);
            if (duration_total <= 0)
            {
                return false;
            }

            const int now_total = static_cast<int>(current_hour) * 60 + static_cast<int>(current_minute);
            const int end_total = start_total + duration_total;

            if (end_total >= MINUTES_PER_DAY)
            {
                return now_total >= start_total || now_total < (end_total - MINUTES_PER_DAY);
            }

            return now_total >= start_total && now_total < end_total;
        }

        void BalboaSpaBinarySensors::set_parent(BalboaSpa *parent)
        {
            this->spa = parent;
            parent->register_listener([this](SpaState *spaState)
                                      { this->update(spaState); });
        }

        void BalboaSpaBinarySensors::update(SpaState *spaState)
        {
            bool sensor_state_value;
            if (spa == nullptr || (!spa->is_communicating() && sensor_type != BalboaSpaBinarySensorType::CONNECTED))
            {
                this->publish_state(NAN);
                return;
            }

            uint8_t state_value = 0;
            switch (sensor_type)
            {
            case BalboaSpaBinarySensorType::BLOWER:
                sensor_state_value = spaState->blower;
                break;
            case BalboaSpaBinarySensorType::HIGHRANGE:
                sensor_state_value = spaState->highrange;
                break;
            case BalboaSpaBinarySensorType::CIRCULATION:
                sensor_state_value = spaState->circulation;
                break;
            case BalboaSpaBinarySensorType::FILTER_CYCLE_1:
                if (!spa->filter_settings_valid())
                {
                    this->publish_state(NAN);
                    return;
                }
                sensor_state_value = is_filter_cycle_running(spa->get_current_filter_settings(), spaState->hour, spaState->minutes, false);
                break;
            case BalboaSpaBinarySensorType::FILTER_CYCLE_2:
                if (!spa->filter_settings_valid())
                {
                    this->publish_state(NAN);
                    return;
                }
                sensor_state_value = is_filter_cycle_running(spa->get_current_filter_settings(), spaState->hour, spaState->minutes, true);
                break;
            case BalboaSpaBinarySensorType::RESTMODE:
                state_value = spaState->rest_mode;
                sensor_state_value = state_value;
                if (state_value == 254)
                {
                    // This indicate no value
                    return;
                }
                break;
            case BalboaSpaBinarySensorType::HEATSTATE:
                state_value = spaState->heat_state;
                sensor_state_value = state_value;
                if (state_value == 254)
                {
                    // no value
                    return;
                }
                break;
            case BalboaSpaBinarySensorType::CONNECTED:
                sensor_state_value = spa->is_communicating();
                break;
            default:
                ESP_LOGD(TAG, "Spa/BSensors/UnknownSensorType: SensorType Number: %d", sensor_type);
                // Unknown enum value. Ignore
                return;
            }

            if (this->state != sensor_state_value || this->last_update_time + 300000 < millis())
            {
                this->publish_state(sensor_state_value);
                last_update_time = millis();
            }
        }

        BalboaSpaBinarySensors::BalboaSpaBinarySensors()
        {
            spa = nullptr;
            sensor_type = BalboaSpaBinarySensorType::UNKNOWN;
            last_update_time = 0;
        }

    }
}
