TEMPLATE = subdirs
CONFIG += ordered

isEmpty(LBAWORLD_BIN_DIR) {
    LBAWORLD_BIN_DIR = $$clean_path($$OUT_PWD/bin)
}
cache(LBAWORLD_BIN_DIR, set)

isEmpty(LIBQTR3D_PATH) {
    LIBQTR3D_PATH = $$clean_path($$_PRO_FILE_PWD_/../libqtr3d)
}
cache(LIBQTR3D_PATH, set)

SUBDIRS = \
           src/unittest_liblbadata \
           src/pal2gimp \
           src/png2rawimage \
           src/hqrpak \
           src/lbaspriteviewer \
           src/lbamodelviewer \
           src/lbwmodelviewer \
           src/lbwmapviewer \
           src/flaplayer \
           src/lbaworld \
           data

check.commands = $$shell_quote($$LBAWORLD_BIN_DIR/unittest_liblbadata)
check.depends = sub-src-unittest_liblbadata
QMAKE_EXTRA_TARGETS += check
