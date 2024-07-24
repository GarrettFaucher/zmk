/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_null_bind

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>  // Include for HID utilities

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Data structure to hold the state for each instance of this behavior
struct behavior_null_bind_data {
    bool a_pressed;  // True if 'A' is currently pressed
    bool d_pressed;  // True if 'D' is currently pressed
};

// Function to handle key presses for the 'null bind' behavior
static int on_key_binding_pressed(struct zmk_behavior_binding *binding,
                                  struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_null_bind_data *data = dev->data;
    
    uint16_t keycode = binding->param1;  // Get the keycode from the binding parameters

    // Check if the pressed key is 'A'
    if (keycode == HID_USAGE_KEY_KEYBOARD_A) {
        if (!data->a_pressed) {
            data->a_pressed = true;  // Mark 'A' as pressed
            if (data->d_pressed) {  // Check if 'D' is already pressed
                zmk_hid_keyboard_release(HID_USAGE_KEY_KEYBOARD_D);  // Release 'D'
                data->d_pressed = false;  // Mark 'D' as not pressed
            }
        }
    }
    // Check if the pressed key is 'D'
    else if (keycode == HID_USAGE_KEY_KEYBOARD_D) {
        if (!data->d_pressed) {
            data->d_pressed = true;  // Mark 'D' as pressed
            if (data->a_pressed) {  // Check if 'A' is already pressed
                zmk_hid_keyboard_release(HID_USAGE_KEY_KEYBOARD_A);  // Release 'A'
                data->a_pressed = false;  // Mark 'A' as not pressed
            }
        }
    }

    return ZMK_BEHAVIOR_OPAQUE;  // Return OPAQUE to prevent further processing of the event
}

// Function to handle key releases for the 'null bind' behavior
static int on_key_binding_released(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_null_bind_data *data = dev->data;
    
    uint16_t keycode = binding->param1;  // Get the keycode from the binding parameters

    // Update state based on which key is released
    if (keycode == HID_USAGE_KEY_KEYBOARD_A) {
        data->a_pressed = false;  // Mark 'A' as not pressed
    }
    else if (keycode == HID_USAGE_KEY_KEYBOARD_D) {
        data->d_pressed = false;  // Mark 'D' as not pressed
    }

    return ZMK_BEHAVIOR_OPAQUE;  // Return OPAQUE to prevent further processing of the event
}

// API structure for interacting with this behavior
static const struct behavior_driver_api behavior_null_bind_driver_api = {
    .binding_pressed = on_key_binding_pressed,
    .binding_released = on_key_binding_released,
};

// Initialization function for this behavior
static int behavior_null_bind_init(const struct device *dev) {
    struct behavior_null_bind_data *data = dev->data;
    data->a_pressed = false;  // Initialize 'A' key pressed state to 'false'
    data->d_pressed = false;  // Initialize 'D' key pressed state to 'false'
    return 0;
}

// Macro to create an instance of this behavior
#define NB_INST(n)                                                                                 \
    static struct behavior_null_bind_data behavior_null_bind_data_##n;                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_null_bind_init, NULL, &behavior_null_bind_data_##n,        \
                            NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,               \
                            &behavior_null_bind_driver_api);

DT_INST_FOREACH_STATUS_OKAY(NB_INST)
