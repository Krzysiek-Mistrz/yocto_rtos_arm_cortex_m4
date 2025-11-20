# Patch Zephyr to disable abort on Kconfig warnings
# This is needed for Yocto builds with external toolchains

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Fix broken GitHub URLs in scarthgap meta-zephyr
# The base recipe has /git/ in the path which doesn't exist
python () {
    src_uri = (d.getVar('SRC_URI') or "").split()
    new_src_uri = []
    for uri in src_uri:
        # Fix github.com/git/ URLs to github.com/
        if 'github.com/git/' in uri:
            uri = uri.replace('github.com/git/', 'github.com/')
        new_src_uri.append(uri)
    d.setVar('SRC_URI', ' '.join(new_src_uri))
}

# Patches removed - issues resolved by different methods
# SRC_URI += "file://0001-kconfig-disable-abort-on-warnings.patch \
#             file://0002-blackpill-remove-broken-pinctrl-include.patch \
#            "

# Disable stripping and debug splitting for Zephyr source package
# The source tree contains test files for other architectures (ESP32, ESP8266)
# that can't be processed with ARM toolchain
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_SYSROOT_STRIP = "1"

# Allow patch fuzz - patches apply correctly but context may not match exactly
ERROR_QA:remove = "patch-fuzz"
