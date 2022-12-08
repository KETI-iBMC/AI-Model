SET(
    SOURCE_FILES
    ${STATIC_LIBS_DIR}/apps/apps.cpp
    ${STATIC_LIBS_DIR}/apps/setting_service.cpp
    ${STATIC_LIBS_DIR}/channel/channel.cpp
    ${STATIC_LIBS_DIR}/chassis/chassis.cpp
    ${STATIC_LIBS_DIR}/dcmi/dcmi.cpp
    ${STATIC_LIBS_DIR}/fru/fru.cpp
    ${STATIC_LIBS_DIR}/ipmi/ipmi.cpp
    ${STATIC_LIBS_DIR}/network/network.cpp
    ${STATIC_LIBS_DIR}/sdr/sdr.cpp
    ${STATIC_LIBS_DIR}/sdr/sensor_define.cpp
    ${STATIC_LIBS_DIR}/sel/sel.cpp
    ${STATIC_LIBS_DIR}/sel/efte.cpp
    ${STATIC_LIBS_DIR}/sel/pef_policy.cpp
    ${STATIC_LIBS_DIR}/session/session.cpp
    ${STATIC_LIBS_DIR}/user/user.cpp
    ${STATIC_LIBS_DIR}/gpio/gpio.cpp
    ${STATIC_LIBS_DIR}/lightning/lightning_sensor.cpp
    ${STATIC_LIBS_DIR}/ipmb/ipmb.cpp
    ${STATIC_LIBS_DIR}/redfish/record.cpp
    ${STATIC_LIBS_DIR}/redfish/resource.cpp
    ${STATIC_LIBS_DIR}/redfish/util.cpp 
    ${STATIC_LIBS_DIR}/util/smbus.cpp
    ${STATIC_LIBS_DIR}/util/pmbus.cpp
    ${STATIC_LIBS_DIR}/sol/sol.cpp
    ${STATIC_LIBS_DIR}/util/KETI_stdx.cpp
    ${STATIC_LIBS_DIR}/web/rest_get.cpp
    ${STATIC_LIBS_DIR}/web/rest_post.cpp
    ${STATIC_LIBS_DIR}/web/rest_put.cpp
    ${STATIC_LIBS_DIR}/web/rest_delete.cpp
    ${STATIC_LIBS_DIR}/util/dbus_system.cpp

)

ADD_LIBRARY(apps STATIC ${SOURCE_FILES})
target_include_directories(
    apps PUBLIC ${INC_DIR}
)
