TEMPLATE=	subdirs
SUBDIRS	=	lib

#!include ../config

module(anaqwt):SUBDIRS	+= profile \
                           roibase

PSUBDIRS = \
		anatomist \
		mkhierarchy \
		hierarchyeditor

module(anaalgo):PSUBDIRS	+= \
		anasurfmatch \
		surfpaint \
		volrender \
                ameshpaint

module(anaalgo)-module(anaqwt):PSUBDIRS	+= \
		perfusion \
		roialgo \
                histogram

module(anavtk):PSUBDIRS += anavtk

darwin-module(anaqwt):PSUBDIRS	+= \
		bundle_profile \
		bundle_roibase

