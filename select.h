#ifndef SELECT_H_
#define SELECT_H_

#include <stddef.h>

#include "device.h"

int select_keyboard(const device_entry *list, size_t count);

#endif // SELECT_H_
