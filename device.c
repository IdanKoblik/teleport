#include "device.h"

#include <libudev.h>
#include <stdlib.h>
#include <string.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"

#define EXPAND_LIST(list_ptr, capacity_var) do {                    \
    (capacity_var) = (capacity_var) == 0 ? 16 : (capacity_var) * 2; \
    device_entry *temp = realloc((list_ptr), (capacity_var) * sizeof(device_entry)); \
    if (!temp) { \
        ERROR("memory reallocation failed"); \
        free(list_ptr);             \
        exit(EXIT_FAILURE); \
    } \
    (list_ptr) = temp; \
} while(0)

static int prop_is_one(struct udev_device *dev, const char *prop) {
    const char *v = udev_device_get_property_value(dev, prop);
    return v && strcmp(v, "1") == 0;
}

device_entry *fetch_devices(device_type types, size_t *out_count) {
    if (out_count) *out_count = 0;

    struct udev *udev = udev_new();
    if (!udev) {
        ERROR("failed to create udev");
        return NULL;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);

    size_t capacity = 16;
    size_t count = 0;

    device_entry *list = malloc(capacity * sizeof(device_entry));
    if (!list) {
        ERROR("memory allocation failed");
        udev_enumerate_unref(enumerate);
        udev_unref(udev);
        return NULL;
    }

    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        if (!dev) continue;

        const char *node = udev_device_get_devnode(dev);

        int is_kbd = prop_is_one(dev, "ID_INPUT_KEYBOARD");
        int is_mouse = prop_is_one(dev, "ID_INPUT_MOUSE");
        int is_touchpad = prop_is_one(dev, "ID_INPUT_TOUCHPAD");
        int is_tablet = prop_is_one(dev, "ID_INPUT_TABLET");
        int is_joystick = prop_is_one(dev, "ID_INPUT_JOYSTICK");
        int is_touchscreen = prop_is_one(dev, "ID_INPUT_TOUCHSCREEN");

        struct udev_device *parent =
            udev_device_get_parent_with_subsystem_devtype(dev, "input", NULL);
        const char *name = parent ? udev_device_get_sysattr_value(parent, "name") : NULL;

        int kbd_only = is_kbd && !is_mouse && !is_touchpad && !is_tablet && !is_joystick && !is_touchscreen;
        int mouse_only = is_mouse && !is_kbd && !is_touchpad && !is_tablet && !is_joystick && !is_touchscreen;

        int kbd_match = (types & KEYBOARD) && kbd_only;
        int mouse_match = (types & MOUSE) && mouse_only;

        if (node && name && (kbd_match || mouse_match) && strstr(node, "/event")) {
            if (count >= capacity) {
                EXPAND_LIST(list, capacity);
            }

            snprintf(list[count].name, sizeof(list[count].name), "%s", name);
            snprintf(list[count].devnode, sizeof(list[count].devnode), "%s", node);
            count++;
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    if (count == 0) {
        WARN("no devices found");
        free(list);
        return NULL;
    }

    if (out_count) *out_count = count;
    return list;
}

int create_vdevice() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        ERROR("failed to open /dev/uinput");
        return -1;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        ERROR("UI_SET_EVBIT EV_KEY failed");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0) {
        ERROR("UI_SET_EVBIT EV_SYN failed");
        close(fd);
        return -1;
    }

    for (int code = 1; code < KEY_MAX; code++) {
        if (ioctl(fd, UI_SET_KEYBIT, code) < 0) {
            ERROR("UI_SET_KEYBIT failed for code %d", code);
            close(fd);
            return -1;
        }
    }

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "Teleport virtual device");

    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        ERROR("UI_DEV_SETUP failed");
        close(fd);
        return -1;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        ERROR("UI_DEV_CREATE failed");
        close(fd);
        return -1;
    }

    return fd;
}

void emit_key(int fd, int type, int code, int val) {
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    if (write(fd, &ie, sizeof(ie)) != sizeof(ie)) {
        ERROR("write key event failed (type=%d code=%d val=%d)", type, code, val);
        return;
    }

    struct input_event syn;
    syn.type = EV_SYN;
    syn.code = SYN_REPORT;
    syn.value = 0;
    syn.time.tv_sec = 0;
    syn.time.tv_usec = 0;

    if (write(fd, &syn, sizeof(syn)) != sizeof(syn)) {
        ERROR("write SYN_REPORT failed");
    }
}
