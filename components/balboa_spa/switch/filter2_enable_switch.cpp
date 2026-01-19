#include "filter2_enable_switch.h"

namespace esphome
{
    namespace balboa_spa
    {
        void Filter2EnableSwitch::update(SpaFilterSettings *settings)
        {
            if (this->state != settings->filter2_enable)
            {
                this->publish_state(settings->filter2_enable);
            }
        }

        void Filter2EnableSwitch::set_parent(BalboaSpa *parent)
        {
            spa = parent;
            parent->register_filter_listener([this](SpaFilterSettings *settings)
                                             { this->update(settings); });
        }

        void Filter2EnableSwitch::write_state(bool state)
        {
            if (state)
            {
                auto *settings = spa->get_current_filter_settings();
                spa->set_filter2_config(settings->filter2_hour,
                                        settings->filter2_minute,
                                        settings->filter2_duration_hour,
                                        settings->filter2_duration_minute);
            }
            else
            {
                spa->disable_filter2();
            }

            spa->request_filter_settings_update();
        }

    } // namespace balboa_spa
} // namespace esphome
