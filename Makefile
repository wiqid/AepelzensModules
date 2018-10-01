SLUG = "repelzen"
VERSION = 0.6.0

RACK_DIR ?= ../..
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res

include $(RACK_DIR)/plugin.mk
