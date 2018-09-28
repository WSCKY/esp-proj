#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

USER_DIRS += $(USER_PATH)/components/drivers/
USER_DIRS += $(USER_PATH)/components/middlewares/

EXTRA_COMPONENT_DIRS += $(USER_DIRS)
