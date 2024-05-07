// platform specific stuff

#include "common.h"
#include "device.h"
#include "platform.h"
#include "ui.h"

#include "blowfish_dev_bin.h"
#include "blowfish_retail_bin.h"

#include <cstdarg>

namespace {
using namespace flashcart_core;

int loglevel = LOG_INFO;

char const *const priority_strings[] = {
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARN",
    "ERROR"
};

char const * prioritytostr(log_priority priority) {
    return (priority >= LOG_PRIORITY_MAX) ? "?!#$" : priority_strings[priority];
}
}

char const * loglevel_str() {
    return prioritytostr((log_priority)loglevel);
}

void toggleLoglevel(void) {
    if (loglevel == 0) {
        loglevel = LOG_PRIORITY_MAX;
    }
    loglevel = loglevel - 1;
}

void toggleSpoofFlashchip(void) {
	spoofFlashchip = !spoofFlashchip;
}

namespace flashcart_core {
bool spoofFlashchip = false;
const uint16_t flashchip_ids[] = {
    0x041F, 0x051F, 0x1A37, 0x3437, 0x49C2, 0x5BC2, 0x80BF, 0x9020, 0x9120, 0x9B37,
    0xA01F, 0xA31F, 0xA7C2, 0xA8C2, 0xBA01, 0xBA04, 0xBA1C, 0xBA4A, 0xBAC2, 0xB537,
    0xB91C, 0xC11F, 0xC298, 0xC31F, 0xC420, 0xC4C2, 0xEE20, 0xEF20, 0xED01,

    // untested "other" types:
    0x49B0, 0x9089, 0x912C, 0x9189, 0x922C, 0x9289, 0x9320, 0x9389, 0x9489, 0x9589,
    0x9689, 0x9789
};
size_t flashchip_count = sizeof(flashchip_ids)/sizeof(flashchip_ids[0]);
uint16_t spoofedFlashchip = flashchip_ids[0];

namespace platform {
void showProgress(uint32_t current, uint32_t total, const char* status_string) {
    ShowProgress(BOTTOM_SCREEN, current, total, status_string);
}

int logMessage(log_priority priority, const char *fmt, ...) {
    if (priority < loglevel) return 0;

    static bool first_open = true;
    // Overwrite if this is our first time opening the file.
    FILE *logfile = fopen("fat1:/ntrboot/ntrboot.log", first_open ? "w" : "a");
    if (!logfile) return -1;
    first_open = false;

    const char *priority_str = prioritytostr(priority);

    char *log_fmt;
    if (asprintf(&log_fmt, "[%s]: %s\n", priority_str, fmt) < 0) {
        return -1; // would pass the actual return value back, but I don't think we care.
    }

    va_list args;
    va_start(args, fmt);
    int result = vfprintf(logfile, log_fmt, args);
    va_end(args);

    fclose(logfile);
    if (priority == LOG_NOTICE) {
        char *tmp_str;
        va_start(args, fmt);
        vasprintf(&tmp_str, fmt, args);
        va_end(args);
        ShowString(BOTTOM_SCREEN, tmp_str);
        free(tmp_str);
    }

    free(log_fmt);
    return result;
}

auto getBlowfishKey(BlowfishKey key) -> const std::uint8_t(&)[0x1048] {
    switch (key) {
        default:
        case BlowfishKey::NTR:
            return *reinterpret_cast<const std::uint8_t(*)[0x1048]>(0x01FFE428);
        case BlowfishKey::B9Retail:
            return *static_cast<const std::uint8_t(*)[0x1048]>(static_cast<const void *>(blowfish_retail_bin));
        case BlowfishKey::B9Dev:
            return *static_cast<const std::uint8_t(*)[0x1048]>(static_cast<const void *>(blowfish_dev_bin));
    }
}
}
}
