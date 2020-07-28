set(COMPONENT_SRCS
  lcd/glcdfont.c
  lcd/lcd.cpp
  lcd/adaptation.cpp
  lcd/font7s.c
  lcd/st7735s.c
  lcd/Adafruit-GFX-Library/Adafruit_GFX.cpp
  sdcard/sdcard.cpp
  )

set(COMPONENT_ADD_INCLUDEDIRS
  lcd/include
  lcd/Adafruit-GFX-Library
  lcd/Adafruit-GFX-Library/Fonts
  sdcard
  )

set(COMPONENT_PRIV_INCLUDEDIRS
  )

set(COMPONENT_REQUIRES driver)
set(COMPONENT_PRIV_REQUIRES freertos nvs_flash fatfs)

register_component()
