import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from .. import (
    balboa_spa_ns,
    BalboaSpa,
    CONF_SPA_ID
)

DEPENDENCIES = ["balboa_spa"]

SpaSensor = balboa_spa_ns.class_("BalboaSpaSensors", sensor.Sensor)
SpaSensorTypeEnum = SpaSensor.enum("BalboaSpaSensorType", True)

CONF_BLOWER = "blower"
CONF_HIGHRANGE = "highrange"
CONF_CIRCULATION = "circulation"
CONF_RESTMODE = "restmode"
CONF_HEATSTATE = "heatstate"
CONF_FILTER1_REMAINING = "filter1_remaining"
CONF_FILTER2_REMAINING = "filter2_remaining"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_BLOWER): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_HIGHRANGE): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_CIRCULATION): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_RESTMODE): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_HEATSTATE): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_FILTER1_REMAINING): sensor.sensor_schema(
            SpaSensor,
            unit_of_measurement="min",
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_FILTER2_REMAINING): sensor.sensor_schema(
            SpaSensor,
            unit_of_measurement="min",
            accuracy_decimals=0,
        ),
    })

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])

    for sensor_type in [
        CONF_BLOWER,
        CONF_HIGHRANGE,
        CONF_CIRCULATION,
        CONF_RESTMODE,
        CONF_HEATSTATE,
        CONF_FILTER1_REMAINING,
        CONF_FILTER2_REMAINING,
    ]:
        if conf := config.get(sensor_type):
            var = await sensor.new_sensor(conf)
            cg.add(var.set_parent(parent))
            sensor_type_value = getattr(SpaSensorTypeEnum, sensor_type.upper())
            cg.add(var.set_sensor_type(sensor_type_value))
