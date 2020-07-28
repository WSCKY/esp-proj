set(COMPONENT_SRCS
  picDec/bmpDec.c
  picDec/imgDecoder.cpp
  picDec/jpgDec.c
  )

set(COMPONENT_ADD_INCLUDEDIRS
  picDec
  )

set(COMPONENT_PRIV_INCLUDEDIRS
  )

set(COMPONENT_REQUIRES driver)
set(COMPONENT_PRIV_REQUIRES freertos nvs_flash)

register_component()
