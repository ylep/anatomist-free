TEMPLATE	= bundle
TARGET		= anaprofile${BUILDMODEEXT}

#!include ../../config

SOURCES =			\
    bundle/profilebundle.cc

HEADERS =


LIBS = $(LIBS_WITH_QWT)
LIB += -lanaprofile${BUILDMODEEXT}
