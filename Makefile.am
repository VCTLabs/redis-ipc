SUBDIRS = src
#SUBDIRS = src inc test

EXTRA_DIST = autogen.sh README.rst debian inc/json.hh

# Clear dependency_libs for multiarch
all-local:
	sed -i "/dependency_libs/ s/'.*'/''/" `find . -name '*.la'`
	sed -i "/dependency_libs/ s/'.*'/''/" `find . -name '*.lai'`

