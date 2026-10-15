TEMPLATE = aux
isEmpty(LBAWORLD_BIN_DIR) {
    LBAWORLD_BIN_DIR = $$clean_path($$OUT_PWD/../bin)
}
first.commands = $(MAKE) -C $$shell_quote($$PWD/ress.hqr) \
    BUILD_DIR=$$shell_quote($$OUT_PWD/ress.hqr/build) \
    BIN_DIR=$$shell_quote($$LBAWORLD_BIN_DIR) && \
    $(MAKE) -C $$shell_quote($$PWD/maps.hqr) \
    BIN_DIR=$$shell_quote($$LBAWORLD_BIN_DIR)
QMAKE_EXTRA_TARGETS += first
