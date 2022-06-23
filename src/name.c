#include <usb_names.h>

#define MANUFACTURER_NAME    {'n','v','p', '+', 'o', 'a', 'm'}
#define MANUFACTURER_NAME_LEN 7
#define PRODUCT_NAME    {'t','e','l', 'e', '-', 't', 'o', 'u', 'c', 'h'}
#define PRODUCT_NAME_LEN 10

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
        2 + MANUFACTURER_NAME_LEN * 2,
        3,
        MANUFACTURER_NAME
};

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + PRODUCT_NAME_LEN * 2,
        3,
        PRODUCT_NAME
};