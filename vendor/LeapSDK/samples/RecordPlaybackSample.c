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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "LeapC.h"
#include "ExampleConnection.h"

int64_t lastFrameID = 0; //The last frame received

int main(int argc, char** argv) {
  OpenConnection();
  while(!IsConnected)
    millisleep(100); //wait a bit to let the connection complete

  printf("Connected.\n");
  LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
  if(deviceProps)
    printf("Using device %s.\n", deviceProps->serial);

  LEAP_RECORDING recordingHandle;
  LEAP_RECORDING_PARAMETERS params;

  //Open the recording for writing
  params.mode = eLeapRecordingFlags_Writing;
  eLeapRS result = LeapRecordingOpen(&recordingHandle, "leapRecording.lmt", params);
  if(LEAP_SUCCEEDED(result)){
    int frameCount = 0;
    while(frameCount < 10){
      LEAP_TRACKING_EVENT *frame = GetFrame();
      if(frame && (frame->tracking_frame_id > lastFrameID)){
        lastFrameID = frame->tracking_frame_id;
        frameCount++;
        uint64_t dataWritten = 0;
        result = LeapRecordingWrite(recordingHandle, frame, &dataWritten);
        printf("Recorded %"PRIu64" bytes for frame %"PRIu64" with %i hands.\n", dataWritten, frame->tracking_frame_id, frame->nHands);
      }
    }
    result = LeapRecordingClose(&recordingHandle);
    if(!LEAP_SUCCEEDED(result))
      printf("Failed to close recording: %s\n", ResultString(result));

    //Reopen the recording for reading
    params.mode = eLeapRecordingFlags_Reading;
    result = LeapRecordingOpen(&recordingHandle, "leapRecording.lmt", params);
    if(LEAP_SUCCEEDED(result)){
      LEAP_TRACKING_EVENT *frame = 0;
      while(frameCount-- > 0){
        uint64_t nextFrameSize = 0;
        result = LeapRecordingReadSize(recordingHandle, &nextFrameSize);
        if(!LEAP_SUCCEEDED(result))
          printf("Couldn't get next frame size: %s\n", ResultString(result));
        if(nextFrameSize > 0){
          frame = (LEAP_TRACKING_EVENT *)malloc((size_t)nextFrameSize);
          result = LeapRecordingRead(recordingHandle, frame, nextFrameSize);
          if(LEAP_SUCCEEDED(result)){
            printf("Read frame %"PRIu64" with %i hands.\n", frame->tracking_frame_id, frame->nHands);
          } else {
            printf("Could not read frame: %s\n", ResultString(result));
          }
        }
      }
      result = LeapRecordingClose(&recordingHandle);
      if(!LEAP_SUCCEEDED(result))
        printf("Failed to close recording: %s\n", ResultString(result));
    } else {
      printf("Failed to open recording for reading: %s\n", ResultString(result));
    }
  } else {
    printf("Failed to open recording for writing: %s\n", ResultString(result));
  }
  return 0;
}
//End-of-Sample
