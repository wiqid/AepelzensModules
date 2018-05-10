SLUG = "Aepelzens Modules"
VERSION = 0.6.0dev

RACK_DIR ?= ../..

#FLAGS += -D v_050_dev
LDFLAGS += -Ldep/lib -lsndfile

FLAGS += -Idep/include
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Static libs
libsndfile := dep/lib/libsndfile.a
OBJECTS += $(libsndfile)

# Dependencies
$(shell mkdir -p dep)
DEP_LOCAL := dep
DEPS += $(libsndfile)

$(libsndfile):
	cd dep && $(WGET) http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz
	cd dep && $(UNTAR) libsndfile-1.0.28.tar.gz
	cd dep/libsndfile-1.0.28 && $(CONFIGURE)
	cd dep/libsndfile-1.0.28/src && $(MAKE)
	cd dep/libsndfile-1.0.28/src && $(MAKE) install

include $(RACK_DIR)/plugin.mk