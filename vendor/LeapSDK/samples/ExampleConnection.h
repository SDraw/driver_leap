/* Copyright (C) 2012-2017 Ultraleap Limited. All rights reserved.
 *
 * Use of this code is subject to the terms of the Ultraleap SDK agreement
 * available at https://central.leapmotion.com/agreements/SdkAgreement unless
 * Ultraleap has signed a separate license agreement with you or your
 * organisation.
 *
 */

#ifndef ExampleConnection_h
#define ExampleConnection_h

#include "LeapC.h"

/* Client functions */
LEAP_CONNECTION* OpenConnection();
void CloseConnection();
void DestroyConnection();
LEAP_TRACKING_EVENT* GetFrame(); //Used in polling example
LEAP_DEVICE_INFO* GetDeviceProperties(); //Used in polling example
const char* ResultString(eLeapRS r);

/* State */
extern bool IsConnected;

/* Callback function pointers */
typedef void (*connection_callback)     ();
typedef void (*device_callback)         (const LEAP_DEVICE_INFO *device);
typedef void (*device_lost_callback)    ();
typedef void (*device_failure_callback) (const eLeapDeviceStatus failure_code,
                                         const LEAP_DEVICE failed_device);
typedef void (*policy_callback)         (const uint32_t current_policies);
typedef void (*tracking_callback)       (const LEAP_TRACKING_EVENT *tracking_event);
typedef void (*log_callback)            (const eLeapLogSeverity severity,
                                         const int64_t timestamp,
                                         const char* message);
typedef void (*config_change_callback)  (const uint32_t requestID, const bool success);
typedef void (*config_response_callback)(const uint32_t requestID, LEAP_VARIANT value);
typedef void (*image_callback)          (const LEAP_IMAGE_EVENT *image_event);
typedef void (*point_mapping_change_callback)(const LEAP_POINT_MAPPING_CHANGE_EVENT *point_mapping_change_event);
typedef void (*head_pose_callback)(const LEAP_HEAD_POSE_EVENT *head_pose_event);

struct Callbacks{
  connection_callback      on_connection;
  connection_callback      on_connection_lost;
  device_callback          on_device_found;
  device_lost_callback     on_device_lost;
  device_failure_callback  on_device_failure;
  policy_callback          on_policy;
  tracking_callback        on_frame;
  log_callback             on_log_message;
  config_change_callback   on_config_change;
  config_response_callback on_config_response;
  image_callback           on_image;
  point_mapping_change_callback on_point_mapping_change;
  head_pose_callback       on_head_pose;
};
extern struct Callbacks ConnectionCallbacks;
extern void millisleep(int milliseconds);
#endif /* ExampleConnection_h */
