/* Copyright (C) 2012-2017 Ultraleap Limited. All rights reserved.
 *
 * Use of this code is subject to the terms of the Ultraleap SDK agreement
 * available at https://central.leapmotion.com/agreements/SdkAgreement unless
 * Ultraleap has signed a separate license agreement with you or your
 * organisation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "LeapC.h"
#include "ExampleConnection.h"

static LEAP_CONNECTION* connectionHandle;

/** Callback for when the connection opens. */
static void OnConnect(){
  printf("Connected.\n");
}

/** Callback for when a device is found. */
static void OnDevice(const LEAP_DEVICE_INFO *props){
  printf("Found device %s.\n", props->serial);
}

/** Callback for when a frame of tracking data is available. */
static void OnFrame(const LEAP_TRACKING_EVENT *frame){
  if (frame->info.frame_id % 60 == 0)
    printf("Frame %lli with %i hands.\n", (long long int)frame->info.frame_id, frame->nHands);

  for(uint32_t h = 0; h < frame->nHands; h++){
    LEAP_HAND* hand = &frame->pHands[h];
    printf("    Hand id %i is a %s hand with position (%f, %f, %f).\n",
                hand->id,
                (hand->type == eLeapHandType_Left ? "left" : "right"),
                hand->palm.position.x,
                hand->palm.position.y,
                hand->palm.position.z);
  }
}

static void OnImage(const LEAP_IMAGE_EVENT *image){
  printf("Image %lli  => Left: %d x %d (bpp=%d), Right: %d x %d (bpp=%d)\n",
      (long long int)image->info.frame_id,
      image->image[0].properties.width,image->image[0].properties.height,image->image[0].properties.bpp*8,
      image->image[1].properties.width,image->image[1].properties.height,image->image[1].properties.bpp*8);
}

static void OnLogMessage(const eLeapLogSeverity severity, const int64_t timestamp,
                         const char* message) {
  const char* severity_str;
  switch(severity) {
    case eLeapLogSeverity_Critical:
      severity_str = "Critical";
      break;
    case eLeapLogSeverity_Warning:
      severity_str = "Warning";
      break;
    case eLeapLogSeverity_Information:
      severity_str = "Info";
      break;
    default:
      severity_str = "";
      break;
  }
  printf("[%s][%lli] %s\n", severity_str, (long long int)timestamp, message);
}

static void* allocate(uint32_t size, eLeapAllocatorType typeHint, void* state) {
  void* ptr = malloc(size);
  return ptr;
}

static void deallocate(void* ptr, void* state) {
  if (!ptr)
    return;
  free(ptr);
}

void OnPointMappingChange(const LEAP_POINT_MAPPING_CHANGE_EVENT *change){
  if (!connectionHandle)
    return;

  uint64_t size = 0;
  if (LeapGetPointMappingSize(*connectionHandle, &size) != eLeapRS_Success || !size)
    return;

  LEAP_POINT_MAPPING* pointMapping = (LEAP_POINT_MAPPING*)malloc((size_t)size);
  if (!pointMapping)
    return;

  if (LeapGetPointMapping(*connectionHandle, pointMapping, &size) == eLeapRS_Success &&
      pointMapping->nPoints > 0) {
    printf("Managing %u points as of frame %lld at %lld\n", pointMapping->nPoints, (long long int)pointMapping->frame_id, (long long int)pointMapping->timestamp);
  }
  free(pointMapping);
}

void OnHeadPose(const LEAP_HEAD_POSE_EVENT *event) {
  printf("Head pose:\n");
  printf("    Head position (%f, %f, %f).\n",
    event->head_position.x,
    event->head_position.y,
    event->head_position.z);
  printf("    Head orientation (%f, %f, %f, %f).\n",
    event->head_orientation.w,
    event->head_orientation.x,
    event->head_orientation.y,
    event->head_orientation.z);
  printf("    Head linear velocity (%f, %f, %f).\n",
    event->head_linear_velocity.x,
    event->head_linear_velocity.y,
    event->head_linear_velocity.z);
  printf("    Head angular velocity (%f, %f, %f).\n",
    event->head_angular_velocity.x,
    event->head_angular_velocity.y,
    event->head_angular_velocity.z);
 }

int main(int argc, char** argv) {
  //Set callback function pointers
  ConnectionCallbacks.on_connection          = &OnConnect;
  ConnectionCallbacks.on_device_found        = &OnDevice;
  ConnectionCallbacks.on_frame               = &OnFrame;
  ConnectionCallbacks.on_image               = &OnImage;
  ConnectionCallbacks.on_point_mapping_change = &OnPointMappingChange;
  ConnectionCallbacks.on_log_message         = &OnLogMessage;
  ConnectionCallbacks.on_head_pose           = &OnHeadPose;

  connectionHandle = OpenConnection();
  {
    LEAP_ALLOCATOR allocator = { allocate, deallocate, NULL };
    LeapSetAllocator(*connectionHandle, &allocator);
  }
  LeapSetPolicyFlags(*connectionHandle, eLeapPolicyFlag_Images | eLeapPolicyFlag_MapPoints, 0);

  printf("Press Enter to exit program.\n");
  getchar();
  
  CloseConnection();
  DestroyConnection();

  return 0;
}
//End-of-Sample
