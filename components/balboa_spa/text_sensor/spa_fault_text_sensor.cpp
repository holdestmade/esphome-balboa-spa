#include "spa_fault_text_sensor.h"

namespace esphome
{
    namespace balboa_spa
    {

        void SpaFaultTextSensor::set_parent(BalboaSpa *parent)
        {
            parent->register_fault_listener(
                [this](SpaFaultLog *spaFaultLog)
                {
                    this->update(spaFaultLog);
                });
        }

        void SpaFaultTextSensor::update(SpaFaultLog *spaFaultLog)
        {
            std::string payload = "Code " + std::to_string(spaFaultLog->fault_code) + ": " + spaFaultLog->fault_message;
            if (payload != last_payload_)
            {
                this->publish_state(payload);
                last_payload_ = payload;
            }
        }

    } // namespace balboa_spa
} // namespace esphome
