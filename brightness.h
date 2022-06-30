#include <iostream>
#include <vector>
#include <conio.h>
#include <hidapi/hidapi.h>

using std::vector;
const uint16_t vendor_id = 0x43e;
const uint16_t product_id = 0x9a40;
const uint16_t max_brightness = 0xd2f0;
const uint16_t min_brightness = 0x0190;

#pragma pack(1)
struct Data {
    unsigned char flag = 0;
    uint16_t brightness = 0; // 屏幕亮度
    uint16_t e1 = 0;
    uint16_t e2 = 0;
};
#pragma pack()

void reverse(uint8_t *data, size_t size) {
    int i = 0;
    int j = (int) size - 1;
    while (i < j) {
        auto t = data[i];
        data[i] = data[j];
        data[j] = t;
        i++;
        j--;
    }
}

template<class T>
T reverse(T from) {
    uint8_t *data = (uint8_t *) &from;
    reverse(data, sizeof(T));
    return *((T *) data);
}

static_assert(sizeof(Data) == 7);

static hid_device * handle = nullptr;

// 0~100
int get_brightness() {
    Data data;
    int res = hid_get_feature_report(handle, (unsigned char *) &data, sizeof(data));
    if (res < 0) {
        printf("Unable to get a feature report.\n");
        printf("%ls", hid_error(handle));
    }
    return (data.brightness / 540);
}

// val: 0~100
void set_brightness(int val) {
    Data data;
    if (val < 0) {
        val = 0;
    }
    if (val > 100) {
        val = 100;
    }
    data.brightness = val * 540;
    int res = hid_send_feature_report(handle, (const unsigned char*)&data, sizeof(data));
    if (res < 0) {
        printf("Unable to set a feature report.\n");
        printf("%ls", hid_error(handle));
    }
}

// val: 0~100
void set_brightness_smoothly(int val) {
    if (val < 0) {
        val = 0;
    }
    if (val > 100) {
        val = 100;
    }
    int cur = get_brightness();
    if (cur < val) {
        while (cur < val) {
            cur += 5;
            set_brightness(cur);
        }
    } else {
        while (cur > val) {
            cur -= 5;
            set_brightness(cur);
        }
    }
}

char *getdeviceinfo() {
    struct hid_device_info *devs, *cur_dev;
    char *ultrafine_monitor_path = NULL;

    if (hid_init())
        return NULL;

    devs = hid_enumerate(0x043e, 0x0); // 0x043e means LG
    cur_dev = devs;
    while (cur_dev) {
//        printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id,
//               cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
//        printf("\n");
//        printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
//        printf("  Product:      %ls\n", cur_dev->product_string);
//        printf("  Release:      %hx\n", cur_dev->release_number);
//        printf("  Interface:    %d\n", cur_dev->interface_number);
//        printf("\n");

        if (cur_dev->product_string && wcsstr(cur_dev->product_string, L"BRIGHTNESS")) {
            ultrafine_monitor_path = cur_dev->path;
            break;
        }

        cur_dev = cur_dev->next;
    }

    return ultrafine_monitor_path;
}

hid_device *handledevice() {
    hid_device *handle;
    char *ultrafine_monitor_path = getdeviceinfo();

    handle = hid_open_path(ultrafine_monitor_path);
    if (!handle) {
        return handle;
    }
    return handle;
}

void brightness_init() {
    handle = handledevice();
}

