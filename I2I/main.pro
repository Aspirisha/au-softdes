TEMPLATE = subdirs
CONFIG+=ordered
SUBDIRS = \
    src \
    i2i \
    test
i2i.depends = src
tests.depends = src
