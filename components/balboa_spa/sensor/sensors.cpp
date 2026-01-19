#include "esphome/core/log.h"
#include "sensors.h"

namespace esphome
{
    namespace balboa_spa
    {

        static const char *TAG = "BalboaSpa.sensors";
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

        static int calculate_filter_cycle_remaining_minutes(const SpaFilterSettings *settings, uint8_t current_hour, uint8_t current_minute, bool filter_cycle_2)
        {
            uint8_t start_hour = settings->filter1_hour;
            uint8_t start_minute = settings->filter1_minute;
            uint8_t duration_hour = settings->filter1_duration_hour;
            uint8_t duration_minute = settings->filter1_duration_minute;

            if (filter_cycle_2)
            {
                if (!settings->filter2_enable)
                {
                    return 0;
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
                return 0;
            }

            if (!is_filter_cycle_running(settings, current_hour, current_minute, filter_cycle_2))
            {
                return 0;
            }

            const int now_total = static_cast<int>(current_hour) * 60 + static_cast<int>(current_minute);
            int end_total = start_total + duration_total;
            if (end_total >= MINUTES_PER_DAY)
            {
                end_total -= MINUTES_PER_DAY;
                if (now_total >= start_total)
                {
                    return (MINUTES_PER_DAY - now_total) + end_total;
                }
                return end_total - now_total;
            }

            return end_total - now_total;
        }

        void BalboaSpaSensors::set_parent(BalboaSpa *parent)
        {
            parent_ = parent;
            parent->register_listener([this](SpaState *spaState)
                                      { this->update(spaState); });
        }

        void BalboaSpaSensors::update(SpaState *spaState)
        {
            float sensor_state_value;

            switch (sensor_type)
            {
            case BalboaSpaSensorType::BLOWER:
                sensor_state_value = spaState->blower;
                break;
            case BalboaSpaSensorType::HIGHRANGE:
                sensor_state_value = spaState->highrange;
                break;
            case BalboaSpaSensorType::CIRCULATION:
                sensor_state_value = spaState->circulation;
                break;
            case BalboaSpaSensorType::RESTMODE:
                sensor_state_value = spaState->rest_mode;
                if (sensor_state_value == 254)
                {
                    // This indicate no value
                    return;
                }
                break;
            case BalboaSpaSensorType::HEATSTATE:
                sensor_state_value = spaState->heat_state;
                if (sensor_state_value == 254)
                {
                    // no value
                    return;
                }
                break;
            case BalboaSpaSensorType::FILTER1_REMAINING:
                if (parent_ == nullptr || !parent_->filter_settings_valid())
                {
                    this->publish_state(NAN);
                    return;
                }
                sensor_state_value = calculate_filter_cycle_remaining_minutes(parent_->get_current_filter_settings(), spaState->hour, spaState->minutes, false);
                break;
            case BalboaSpaSensorType::FILTER2_REMAINING:
                if (parent_ == nullptr || !parent_->filter_settings_valid())
                {
                    this->publish_state(NAN);
                    return;
                }
                sensor_state_value = calculate_filter_cycle_remaining_minutes(parent_->get_current_filter_settings(), spaState->hour, spaState->minutes, true);
                break;
            default:
                ESP_LOGD(TAG, "Spa/Sensors/UnknownSensorType: SensorType Number: %d", sensor_type);
                // Unknown enum value. Ignore
                return;
            }

            if (this->state != sensor_state_value)
            {
                this->publish_state(sensor_state_value);
            }
        }
    }
}
