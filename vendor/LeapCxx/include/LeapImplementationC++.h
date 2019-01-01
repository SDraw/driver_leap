/******************************************************************************\
* Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/
#pragma once

#include "LeapC++.h"
#include "LeapC.h"
#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <future>
#include <unordered_map>

namespace Leap {

// ListBaseImplementation

template<typename T>
class ListBaseImplementation : public Interface::Implementation {
public:
  ListBaseImplementation(std::vector<T>&& data) : m_Data(std::move(data)) {}
  ListBaseImplementation(const std::vector<T>& data = std::vector<T>()) : m_Data(data) {}

  int count() const {
    return static_cast<int>(m_Data.size());
  }

  bool empty() const {
    return m_Data.empty();
  }

  const T& at(int index) const {
    const int size = static_cast<int>(m_Data.size());
    if (index >= size || index < -size) {
      return T::invalid();
    } else if (index >= 0) {
      return m_Data[index];
    } else {
      return m_Data[size + index];
    }
  }

  ListBaseImplementation& append(const ListBaseImplementation& rhs) {
    if (!rhs.m_Data.empty()) {
      m_Data.reserve(m_Data.size() + rhs.m_Data.size());
      m_Data.insert(m_Data.end(), rhs.m_Data.begin(), rhs.m_Data.end());
    }
    return *this;
  }

  void setData(std::vector<T> other) {
    m_Data = std::move(other);
  }

  // For the eventual filters:
  template<typename FilterType>
  ListBaseImplementation filter() const {
    ListBaseImplementation result(m_Data);
    result.m_Data.erase(std::remove_if(result.m_Data.begin(), result.m_Data.end(), FilterType()), result.m_Data.end());
    return result;
  }

  const std::vector<T>& data() {
    return m_Data;
  }

private:
  std::vector<T> m_Data;
};

// DeviceImplementation

class DeviceImplementation : public Interface::Implementation {
public:
  DeviceImplementation() = default;
  explicit DeviceImplementation(const LEAP_DEVICE_REF& ref) {
    if (LeapOpenDevice(ref, &m_device) != eLeapRS_Success) {
      return;
    }
    m_info.size = static_cast<uint32_t>(sizeof(m_info));
    m_info.serial_length = static_cast<uint32_t>(m_serial.size());
    m_info.serial = const_cast<char*>(m_serial.data());
    eLeapRS status = LeapGetDeviceInfo(m_device, &m_info);
    if (status == eLeapRS_InsufficientBuffer) {
      m_serial.resize(m_info.serial_length + 1);
      m_info.serial = const_cast<char*>(m_serial.data());
      status = LeapGetDeviceInfo(m_device, &m_info);
    }
    if (status != eLeapRS_Success) {
      LeapCloseDevice(m_device);
      m_device = nullptr;
    } else {
      m_description = "Connected Device: " + m_serial;
    }
  }
  ~DeviceImplementation() {
    LeapCloseDevice(m_device);
  }
  float horizontalViewAngle() const { return m_info.h_fov; }
  float verticalViewAngle() const { return m_info.v_fov; }
  float range() const { return static_cast<float>(static_cast<double>(m_info.range)/1000.0); }
  float baseline() const { return static_cast<float>(static_cast<double>(m_info.baseline)/1000.0); }
  float distanceToBoundary(const Vector& position) const;
  bool isStreaming() const { return (m_info.status & eLeapDeviceStatus_Streaming) == eLeapDeviceStatus_Streaming; }
  bool isSmudged() const { return (m_info.status & eLeapDeviceStatus_Smudged) == eLeapDeviceStatus_Smudged; }
  bool isLightingBad() const { return false; }
  bool isValid() const { return !!m_device; }
  Device::Type type() const { return static_cast<Device::Type>(m_info.pid); }
  const std::string& toString() const { return m_serial; }
  const std::string& serialNumber() const { return m_serial; }
  eLeapDeviceStatus status() const { return static_cast<eLeapDeviceStatus>(m_info.status); }

protected:
  LEAP_DEVICE m_device = nullptr;
  LEAP_DEVICE_INFO m_info;
  std::string m_serial;
  std::string m_description = "Invalid Device";

  friend class ControllerImplementation;
};

// FailedDeviceImplementation

class FailedDeviceImplementation : public Interface::Implementation {
public:
  FailedDeviceImplementation() = default;
  FailedDeviceImplementation(const std::string& pnpId, FailedDevice::FailureType error) :
      m_pnpId(pnpId), m_error(error)
  {}
  const std::string& pnpId() const { return m_pnpId; }
  FailedDevice::FailureType failure() const { return m_error; }
  bool isValid() const { return !m_pnpId.empty(); }

protected:
  std::string m_pnpId;
  FailedDevice::FailureType m_error = FailedDevice::FAIL_UNKNOWN;
};

// BoneImplementation

class BoneImplementation : public Interface::Implementation {
public:
  BoneImplementation() : m_bone(s_invalid), m_type(Bone::TYPE_METACARPAL), m_name("Invalid Bone") {}
  BoneImplementation(const LEAP_BONE& bone, Bone::Type type, bool isLeftHand) :
    m_bone(bone), m_type(type), m_name([type]{
      std::ostringstream oss;
      oss << "Bone index:" << static_cast<int>(type);
      return oss.str();
    }()), m_isLeftHand(isLeftHand), m_isValid(true) {}
  Vector prevJoint() const { return m_bone.prev_joint.v; }
  Vector nextJoint() const { return m_bone.next_joint.v; }
  Vector center() const { return (Vector(m_bone.prev_joint.v) + Vector(m_bone.next_joint.v))*0.5f; }
  Vector direction() const { return (Vector(m_bone.next_joint.v) - Vector(m_bone.prev_joint.v)).normalized(); }
  float length() const { return (Vector(m_bone.next_joint.v) - Vector(m_bone.prev_joint.v)).magnitude(); }
  float width() const { return m_bone.width; }
  Bone::Type type() const { return m_type; }
  Matrix basis() const {
    if (!isValid())
      return Leap::Matrix::identity();
    const LEAP_QUATERNION& q = m_bone.rotation;
    Matrix mat(q.x, q.y, q.z, q.w);
    if (m_isLeftHand) {
      mat.xBasis = -mat.xBasis;
    }
    return mat;
  }
  bool isValid() const { return m_isValid; }
  const std::string& toString() const { return m_name; }

protected:
  static LEAP_BONE s_invalid;
  const LEAP_BONE& m_bone;
  const Bone::Type m_type;
  const std::string m_name;
  bool m_isLeftHand = false;
  bool m_isValid = false;
};

// ImageImplementation

class ImageImplementation : public Interface::Implementation {
public:
  ImageImplementation() : m_name("Invalid Image") {
    std::memset(&m_image_event, 0, sizeof(m_image_event));
    m_image_event.info.frame_id = -1;
    m_image_event.image[0].properties.bpp = 1;
    m_image_event.image[1].properties.bpp = 1;
  }
  explicit ImageImplementation(const std::shared_ptr<ControllerImplementation>& controllerImpl, const LEAP_IMAGE_EVENT& image_event, int32_t imageId);
  int64_t sequenceId() const { return m_image_event.info.frame_id; }
  int32_t id() const { return isValid() ? m_imageId : -1; }
  const unsigned char* data() const { return reinterpret_cast<const unsigned char*>(m_image_event.image[m_imageId].data) + m_image_event.image[m_imageId].offset; }
  const float* distortion() const {
    if (!isValid())
      throw std::runtime_error("Calibration data not found for image");
    return reinterpret_cast<const float*>(m_image_event.image[m_imageId].distortion_matrix);
  }
  int width() const { return static_cast<int>(m_image_event.image[m_imageId].properties.width); }
  int height() const { return static_cast<int>(m_image_event.image[m_imageId].properties.height); }
  int bytesPerPixel() const { return static_cast<int>(m_image_event.image[m_imageId].properties.bpp); }
  Image::FormatType format() const {
    switch (m_image_event.image[m_imageId].properties.format) {
      case eLeapImageFormat_IR: return Image::INFRARED;
      case eLeapImageFormat_RGBIr_Bayer: return Image::IBRG;
      default: return Image::INFRARED;
    }
  }
  int distortionWidth() const { return 2*LEAP_DISTORTION_MATRIX_N; } // This is the stride, not the width (it contains xy pairs, hence the 2x)
  int distortionHeight() const { return LEAP_DISTORTION_MATRIX_N; }
  float rayOffsetX() const { return 0.5f; }
  float rayOffsetY() const { return 0.5f; }
  float rayScaleX() const { return 0.5f/DISTORTION_RANGE; }
  float rayScaleY() const { return 0.5f/DISTORTION_RANGE; }
  Vector rectify(const Vector& uv) const;
  Vector warp(const Vector& xy) const;
  float calibOffsetX() const { return m_image_event.image[m_imageId].properties.x_offset; }
  float calibOffsetY() const { return m_image_event.image[m_imageId].properties.y_offset; }
  float calibScaleX() const { return m_image_event.image[m_imageId].properties.x_scale; }
  float calibScaleY() const { return m_image_event.image[m_imageId].properties.y_scale; }
  int64_t timestamp() const { return m_image_event.info.timestamp; }
  bool isValid() const { return m_image_event.info.frame_id != -1; }
  const std::string& toString() const { return m_name; }

  eLeapImageType type() const { return m_image_event.image[m_imageId].properties.type; }

  const float DISTORTION_RANGE = 4.0f;

protected:
  std::weak_ptr<ControllerImplementation> m_weakControllerImpl;
  std::shared_ptr<uint8_t> m_ref;
  LEAP_IMAGE_EVENT m_image_event;
  const int32_t m_imageId = 0;
  const std::string m_name;
};

// FingerImplementation

class FingerImplementation : public Interface::Implementation {
public:
  FingerImplementation() : m_digit(s_invalid), m_id(-1), m_name("Invalid Finger") {}
  FingerImplementation(const std::shared_ptr<HandImplementation>& handImpl,
                       const LEAP_DIGIT& digit);
  Frame frame() const;
  Hand hand() const;

  Finger::Type type() const { return static_cast<Finger::Type>(m_digit.finger_id); }
  int32_t id() const { return m_id; }
  Bone bone(Bone::Type boneIx) const {
    if (!isValid() || boneIx < Bone::TYPE_METACARPAL || boneIx > Bone::TYPE_DISTAL) {
      return Bone::invalid();
    }
    return Bone(std::make_shared<BoneImplementation>(m_digit.bones[boneIx], boneIx, m_isLeftHand).get());
  }
  Vector tipPosition() const { return m_digit.distal.next_joint.v; }
  Vector direction() const { return (Vector(m_digit.intermediate.next_joint.v) - Vector(m_digit.intermediate.prev_joint.v)).normalized(); }
  float width() const { return m_digit.proximal.width; }
  float length() const {
    if (m_length < 0) {
      // Computation is from platform (RawHand::ToPB)
      m_length = (m_digit.finger_id == Finger::TYPE_THUMB ? 0.0f : (Vector(m_digit.proximal.next_joint.v) - Vector(m_digit.proximal.prev_joint.v)).magnitude()*0.5f) +
                 (Vector(m_digit.intermediate.next_joint.v) - Vector(m_digit.intermediate.prev_joint.v)).magnitude() +
                 (Vector(m_digit.distal.next_joint.v) - Vector(m_digit.distal.prev_joint.v)).magnitude()*0.77f;

    }
    return m_length;
  }
  bool isExtended() const { return !!m_digit.is_extended; }
  float timeVisible() const;
  bool isValid() const { return m_id != -1; }
  const std::string& toString() const { return m_name; }

protected:
  static LEAP_DIGIT s_invalid;
  std::weak_ptr<HandImplementation> m_weakHandImpl;
  const LEAP_DIGIT& m_digit;
  const int32_t m_id;
  mutable float m_length = -1;
  const std::string m_name;
  bool m_isLeftHand = false;
};

// HandImplementation

class HandImplementation : public Interface::Implementation {
public:
  HandImplementation() : m_hand(s_invalid), m_name("Invalid Hand") {}
  HandImplementation(const std::shared_ptr<FrameImplementation>& frameImpl,
                     const LEAP_HAND& hand) :
    m_weakFrameImpl(frameImpl), m_hand(hand), m_name([&hand]{
      std::ostringstream oss;
      oss << "Hand Id:" << static_cast<int32_t>(hand.id);
      return oss.str();
    }()) {}
  int32_t id() const { return static_cast<int32_t>(m_hand.id); }
  Frame frame() const { auto frameImpl = m_weakFrameImpl.lock(); return frameImpl ? Frame(frameImpl.get()) : Frame::invalid(); }
  FingerList fingers() {
    std::vector<Finger> fingers;
    processFingers();
    fingers.reserve(m_fingers.size());
    for (const auto& finger : m_fingers) {
      fingers.emplace_back(finger.get());
    }
    return FingerList(std::make_shared<ListBaseImplementation<Finger>>(fingers));
  }
  Finger finger(int32_t id) {
    processFingers();
    for (const auto& finger : m_fingers) {
      if (finger->id() == id) {
        return Finger(finger.get());
      }
    }
    return Finger::invalid();
  }
  Vector palmPosition() const { return m_hand.palm.position.v; }
  Vector stabilizedPalmPosition() const { return m_hand.palm.stabilized_position.v; }
  Vector palmVelocity() const { return m_hand.palm.velocity.v; }
  Vector palmNormal() const { return m_hand.palm.normal.v; }
  float palmWidth() const { return m_hand.palm.width; }
  Vector direction() const { return Vector(m_hand.palm.direction.v).normalized(); }
  Matrix basis() const {
    if (!isValid())
      return Leap::Matrix::identity();

    const Vector palmNormal(m_hand.palm.normal.v);
    const Vector direction(Vector(m_hand.palm.direction.v).normalized());
    const Vector crossed = palmNormal.cross(direction);
    return Matrix(isLeft() ? -crossed : crossed, -palmNormal, -direction);
  }
  Arm arm() { return Arm(this); }
  float pinchDistance() const { return m_hand.pinch_distance; }
  float grabAngle() const { return m_hand.grab_angle; }
  float pinchStrength() const { return m_hand.pinch_strength; }
  float grabStrength() const { return m_hand.grab_strength; }
  float timeVisible() const { return static_cast<float>(static_cast<double>(m_hand.visible_time)*1e-6); }
  float confidence() const { return m_hand.confidence; }
  bool isLeft() const { return m_hand.type == eLeapHandType_Left; }
  bool isRight() const { return m_hand.type != eLeapHandType_Left; }
  bool isValid() const { return static_cast<int32_t>(m_hand.id) != -1; }
  const std::string& toString() const { return m_name; }

  // Arm
  float armWidth() const { return m_hand.arm.width; }
  Vector armDirection() const { return isValid() ? (Vector(m_hand.arm.next_joint.v) - Vector(m_hand.arm.prev_joint.v)).normalized() : Vector(0, 0, -1); }
  Matrix armBasis() const {
    if (!isValid())
      return Leap::Matrix::identity();
    const LEAP_QUATERNION& q = m_hand.arm.rotation;
    Matrix mat(q.x, q.y, q.z, q.w);
    if (isLeft()) {
      mat.xBasis = -mat.xBasis;
    }
    return mat;
  }
  Vector elbowPosition() const { return m_hand.arm.prev_joint.v; }
  Vector wristPosition() const { return m_hand.arm.next_joint.v; }
  Vector armCenter() const { return (Vector(m_hand.arm.prev_joint.v) + Vector(m_hand.arm.next_joint.v))*0.5f; }

  const std::vector<std::shared_ptr<FingerImplementation>>& fingersVector() {
    processFingers();
    return m_fingers;
  }

protected:
  void processFingers() {
    if (m_fingers.empty() && isValid()) {
      // Cache the FingerImplementations
      m_fingers.reserve(5);
      for (int i = 0; i < 5; i++) {
        m_fingers.emplace_back(std::make_shared<FingerImplementation>(std::static_pointer_cast<HandImplementation>(shared_from_this()), m_hand.digits[i]));
      }
    }
  }

  static LEAP_HAND s_invalid;

  std::weak_ptr<FrameImplementation> m_weakFrameImpl;
  const LEAP_HAND& m_hand;
  mutable std::vector<std::shared_ptr<FingerImplementation>> m_fingers;
  const std::string m_name;
};

class HeadPoseImplementation: public Interface::Implementation {
public:
  HeadPoseImplementation() { std::memset(&m_head_pose_event, 0, sizeof(m_head_pose_event)); }
  explicit HeadPoseImplementation(const LEAP_HEAD_POSE_EVENT& head_pose_event) : m_head_pose_event(head_pose_event) {}
  int64_t timestamp() const { return m_head_pose_event.timestamp; }
  Vector position() const { return m_head_pose_event.head_position.v; }
  Quaternion orientation() const { return m_head_pose_event.head_orientation.v; }
protected:
  LEAP_HEAD_POSE_EVENT m_head_pose_event;
};

// FrameImplementation

class FrameImplementation : public Interface::Implementation {
public:
  FrameImplementation() : m_name("Invalid Frame") { std::memset(&m_tracking_event, 0, sizeof(m_tracking_event)); m_tracking_event.info.frame_id = -1; }
  explicit FrameImplementation(const LEAP_TRACKING_EVENT& tracking_event) :
    m_tracking_event(tracking_event), m_name([&tracking_event]{
      std::ostringstream oss;
      oss << "Frame Id:" << tracking_event.info.frame_id;
      return oss.str();
    }()) {
    // Deep copy the tracking event
    m_raw_hands.clear();
    m_raw_hands.assign(tracking_event.pHands, tracking_event.pHands + tracking_event.nHands);
    m_tracking_event.pHands = !m_raw_hands.empty() ? &m_raw_hands[0] : nullptr;
  }
  int64_t id() const { return m_tracking_event.info.frame_id; }
  int64_t timestamp() const { return m_tracking_event.info.timestamp; }
  HandList hands() {
    std::vector<Hand> hands;
    if (m_tracking_event.nHands > 0) {
      processHands();
      hands.reserve(m_hands.size());
      for (const auto& hand : m_hands) {
        hands.emplace_back(hand.get());
      }
    }
    return HandList(std::make_shared<ListBaseImplementation<Hand>>(hands));
  }
  Hand hand(int32_t id) {
    if (m_tracking_event.nHands > 0) {
      processHands();
      for (const auto& hand : m_hands) {
        if (hand->id() == id) {
          return Hand(hand.get());
        }
      }
    }
    return Hand::invalid();
  }
  FingerList fingers() {
    std::vector<Finger> fingers;
    if (m_tracking_event.nHands > 0) {
      processFingers();
      fingers.reserve(m_fingers.size());
      for (const auto& finger : m_fingers) {
        fingers.emplace_back(finger.get());
      }
    }
    return FingerList(std::make_shared<ListBaseImplementation<Finger>>(fingers));
  }
  Finger finger(int32_t id) {
    if (m_tracking_event.nHands > 0) {
      processFingers();
      for (const auto& finger : m_fingers) {
        if (finger->id() == id) {
          return Finger(finger.get());
        }
      }
    }
    return Finger::invalid();
  }
  ImageList images() { return getImages(eLeapImageType_Default); }
  ImageList rawImages() { return getImages(eLeapImageType_Raw); }
  MapPointList mapPoints() {
    std::lock_guard<decltype(m_mapPointsMutex)> lk(m_mapPointsMutex);
    return MapPointList(std::make_shared<ListBaseImplementation<MapPoint>>(m_mapPoints));
  }
  float currentFramesPerSecond() const { return m_tracking_event.framerate; }
  bool isValid() const { return m_tracking_event.info.frame_id != -1; }
  const std::string& toString() const { return m_name; }

  ImageList getImages(eLeapImageType type) {
    std::vector<Image> images;
    images.reserve(2);
    {
      std::lock_guard<decltype(m_imageMutex)> lk(m_imageMutex);
      for (const auto& image : m_images) {
        if (image->type() == type) {
          images.emplace_back(image.get());
        }
      }
    }
    return ImageList(std::make_shared<ListBaseImplementation<Image>>(images));
  }

  void setImages(const std::vector<std::shared_ptr<ImageImplementation>>& images) {
    std::lock_guard<decltype(m_imageMutex)> lk(m_imageMutex);
    m_images = images;
  }

  void setMapPoints(const LEAP_POINT_MAPPING& pointMapping) {
    if (id() != pointMapping.frame_id) {
      return;
    }
    std::vector<MapPoint> mapPoints;
    if (pointMapping.nPoints > 0 && pointMapping.nPoints < ~0U) {
      mapPoints.reserve(pointMapping.nPoints);
      for (uint32_t index = 0; index < pointMapping.nPoints; index++) {
        mapPoints.emplace_back(MapPoint{pointMapping.pIDs[index], Vector{pointMapping.pPoints[index].v}});
      }
    }
    std::lock_guard<decltype(m_mapPointsMutex)> lk(m_mapPointsMutex);
    m_mapPoints = std::move(mapPoints);
  }

protected:
  void processHands() {
    if (m_hands.empty() && isValid()) {
      // Cache the HandImplementations
      m_hands.reserve(m_tracking_event.nHands);
      for (int i = 0; i < static_cast<int>(m_tracking_event.nHands); i++) {
        m_hands.emplace_back(std::make_shared<HandImplementation>(std::static_pointer_cast<FrameImplementation>(shared_from_this()), m_raw_hands[i]));
      }
    }
  }
  void processFingers() {
    if (m_fingers.empty() && isValid()) {
      processHands();
      // Cache the FingerImplementations
      m_fingers.reserve(m_hands.size() * 5);
      for (auto& hand : m_hands) {
        const auto fingers = hand->fingersVector();
        m_fingers.insert(m_fingers.end(), fingers.begin(), fingers.end());
      }
    }
  }

  LEAP_TRACKING_EVENT m_tracking_event;
  std::vector<LEAP_HAND> m_raw_hands;
  std::vector<std::shared_ptr<HandImplementation>> m_hands;
  std::vector<std::shared_ptr<FingerImplementation>> m_fingers;
  std::vector<std::shared_ptr<ImageImplementation>> m_images;
  std::vector<MapPoint> m_mapPoints;
  std::mutex m_imageMutex;
  std::mutex m_mapPointsMutex;
  const std::string m_name;
};

// ControllerImplementation

class ControllerImplementation : public Interface::Implementation {
public:
  ControllerImplementation(const Controller& controller, const char* server_namespace = nullptr) : m_controller(controller) {
    LEAP_CONNECTION_CONFIG config;
    config.size = static_cast<uint32_t>(sizeof(LEAP_CONNECTION_CONFIG));
    config.flags = 0;
    config.server_namespace = server_namespace;
    eLeapRS result = LeapCreateConnection(&config, &m_connection);
    if (result != eLeapRS_Success)
      return;
    result = LeapOpenConnection(m_connection);
    if (result != eLeapRS_Success)
      return;
    LEAP_ALLOCATOR allocator = { staticAllocate, staticDeallocate, this };
    LeapSetAllocator(m_connection, &allocator);
    m_isRunning = true;
    m_pollingThread = std::thread([this] {
      LEAP_CONNECTION_MESSAGE msg;
      uint32_t timeout = 250;
      while (m_isRunning) {
        if (LeapPollConnection(m_connection, timeout, &msg) != eLeapRS_Success)
          continue;
        switch (msg.type) {
          case eLeapEventType_Connection: onConnection(msg.connection_event); break;
          case eLeapEventType_ConnectionLost: onConnectionLost(msg.connection_lost_event); break;
          case eLeapEventType_Device: onDevice(msg.device_event); break;
          case eLeapEventType_DeviceLost: onDeviceLost(msg.device_event); break;
          case eLeapEventType_DeviceStatusChange: onDeviceStatusChange(msg.device_status_change_event); break;
          case eLeapEventType_DeviceFailure: onDeviceFailure(msg.device_failure_event); break;
          case eLeapEventType_Tracking: onTracking(msg.tracking_event); break;
          case eLeapEventType_ImageComplete: break; // Ignored
          case eLeapEventType_ImageRequestError: break; // Ignored
          case eLeapEventType_LogEvent: onLog(msg.log_event); break;
          case eLeapEventType_LogEvents: onLogs(msg.log_events); break;
          case eLeapEventType_Policy: onPolicy(msg.policy_event); break;
          case eLeapEventType_ConfigChange: onConfigChange(msg.config_change_event); break;
          case eLeapEventType_ConfigResponse: onConfigResponse(msg.config_response_event); break;
          case eLeapEventType_Image: onImage(msg.image_event); break;
          case eLeapEventType_PointMappingChange: onPointMappingChange(msg.point_mapping_change_event); break;
          case eLeapEventType_HeadPose: onHeadPose(msg.head_pose_event); break;
          default: break; // Ignored
        }
      }
    });
  }

  ~ControllerImplementation() {
    m_isRunning = false;
    LeapSetAllocator(m_connection, nullptr);
    LeapCloseConnection(m_connection);
    if (m_pollingThread.joinable())
      m_pollingThread.join();
    LeapDestroyConnection(m_connection);
  }

  bool isConnected() {
    std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
    return !m_devices.empty();
  }

  bool isServiceConnected() const {
    return m_isServiceConnected;
  }

  Controller::PolicyFlag policyFlags() {
    return static_cast<Controller::PolicyFlag>(m_policyFlags);
  }

  void setPolicyFlags(Controller::PolicyFlag flags) {
    m_policyFlags |= static_cast<uint32_t>(flags);
    LeapSetPolicyFlags(m_connection, static_cast<uint64_t>(flags), 0);
  }

  void setPolicy(Controller::PolicyFlag policy) {
    m_policyFlags |= static_cast<uint32_t>(policy);
    LeapSetPolicyFlags(m_connection, static_cast<uint64_t>(m_policyFlags), 0);
  }

  void clearPolicy(Controller::PolicyFlag policy) {
    m_policyFlags &= ~static_cast<uint32_t>(policy);
    LeapSetPolicyFlags(m_connection, 0, static_cast<uint64_t>(policy));
  }

  bool isPolicySet(Controller::PolicyFlag policy) {
    return (m_policyFlags & static_cast<uint32_t>(policy)) == static_cast<uint32_t>(policy);
  }

  bool addListener(Listener& listener) {
    std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
    const bool added = m_listeners.insert(&listener).second;
    if (added)
      listener.onInit(m_controller);
    if (m_isServiceConnected)
      listener.onServiceConnect(m_controller);
    if (!m_devices.empty())
      listener.onConnect(m_controller);
    return added;
  }

  bool removeListener(Listener& listener) {
    std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
    const bool removed = !!m_listeners.erase(&listener);
    if (removed)
      listener.onExit(m_controller);
    return removed;
  }

  Frame frame(int history) {
    {
      std::lock_guard<decltype(m_frameMutex)> lk(m_frameMutex);
      if (history >= 0 && history < static_cast<int>(m_frames.size()))
        return Frame(m_frames[history].get());
    }
    return Frame();
  }

  HeadPose headPose(int64_t timestamp) {
    LEAP_HEAD_POSE_EVENT event;
    if (LeapInterpolateHeadPose(m_connection, timestamp, &event) == eLeapRS_Success) {
      auto impl = std::make_shared<HeadPoseImplementation>(event);
      return HeadPose(impl.get());
    }
    return HeadPose();
  }

  Config config() {
    return Config(this);
  }

  Config::Value getConfigValue(const std::string& key, std::chrono::milliseconds timeout) {
    uint32_t id = 0;
    LeapRequestConfigValue(m_connection, key.c_str(), &id);
    std::future<Config::Value> future;
    {
      std::lock_guard<std::mutex> lock(m_configPromiseMutex);
      future = m_configPromises[id].get_future();
    }
    const auto status = future.wait_for(timeout);
    if (status == std::future_status::ready) {
      std::lock_guard<std::mutex> lock(m_configPromiseMutex);
      m_configPromises.erase(id);
      return future.get();
    } else {
      std::lock_guard<std::mutex> lock(m_configPromiseMutex);
      m_configPromises.erase(id);
      return {};
    }
  }

  bool setConfigValue(const std::string& key, const Config::Value& value) {
    uint32_t id = 0;
    LEAP_VARIANT leapVar;
    leapVar.type = static_cast<eLeapValueType>(value.type);
    switch(leapVar.type) {
    case Config::TYPE_BOOLEAN:
      leapVar.boolValue = value.bValue;
      break;
    case Config::TYPE_INT32:
      leapVar.iValue = value.iValue;
      break;
    case Config::TYPE_FLOAT:
      leapVar.fValue = value.fValue;
      break;
    case Config::TYPE_STRING:
      leapVar.strValue = value.strValue; //shallow copy ok since the lifetime of value is longer than that of this function
      break;
    default:
      break;
    }
    return LeapSaveConfigValue(m_connection, key.c_str(), &leapVar, &id) == eLeapRS_Success;
  }

  DeviceList devices() {
    std::vector<Device> devices;
    {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      devices.reserve(m_devices.size());
      for (const auto& device : m_devices) {
        devices.emplace_back(device.second.get());
      }
    }
    return DeviceList(std::make_shared<ListBaseImplementation<Device>>(devices));
  }

  FailedDeviceList failedDevices() {
    std::vector<FailedDevice> failedDevices;
    {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      failedDevices.reserve(m_devices.size());
      for (const auto& device : m_devices) {
        if (LEAP_FAILED(device.second->status())) {
          FailedDevice::FailureType failure;
          switch (device.second->status()) {
          case eLeapDeviceStatus_UnknownFailure: failure = FailedDevice::FAIL_UNKNOWN; break;
          case eLeapDeviceStatus_BadCalibration: failure = FailedDevice::FAIL_CALIBRATION; break;
          case eLeapDeviceStatus_BadFirmware: failure = FailedDevice::FAIL_FIRMWARE; break;
          case eLeapDeviceStatus_BadTransport: failure = FailedDevice::FAIL_TRANSPORT; break;
          case eLeapDeviceStatus_BadControl: failure = FailedDevice::FAIL_CONTROL; break;
          default: failure = FailedDevice::FAIL_UNKNOWN; break;
          }
          failedDevices.emplace_back(std::make_shared<FailedDeviceImplementation>(device.second->toString(), failure).get());
        }
      }
    }
    return FailedDeviceList(std::make_shared<ListBaseImplementation<FailedDevice>>(failedDevices));
  }

  void setPaused(bool pause) {
    LeapSetPause(m_connection, pause);
  }

  bool isPaused() {
    std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
    for (const auto& device : m_devices) {
      if (!LEAP_FAILED(device.second->status()) &&
          (device.second->status() & eLeapDeviceStatus_Streaming) == eLeapDeviceStatus_Streaming) {
        return false; // If any device is streaming, we aren't paused
      }
    }
    return true;
  }

  ImageList images() { return getImages(eLeapImageType_Default); }
  ImageList rawImages() { return getImages(eLeapImageType_Raw); }
  ImageList getImages(eLeapImageType type) {
    std::vector<Image> images;
    images.reserve(2);
    {
      std::lock_guard<decltype(m_imageMutex)> lk(m_imageMutex);
      if (!m_images.empty()) {
        for (const auto& image : m_images.front()) {
          if (image->type() == type) {
            images.emplace_back(image.get());
          }
        }
      }
    }
    return ImageList(std::make_shared<ListBaseImplementation<Image>>(images));
  }

  Vector rectify(eLeapPerspectiveType perspectiveType, const Vector& uv) const {
    LEAP_VECTOR pixel{uv.x, uv.y, 1.0f};
    return LeapPixelToRectilinear(m_connection, perspectiveType, pixel).v;
  }

  Vector warp(eLeapPerspectiveType perspectiveType, const Vector& xy) const {
    LEAP_VECTOR pixel{xy.x, xy.y, 1.0f};
    return LeapRectilinearToPixel(m_connection, perspectiveType, pixel).v;
  }

  std::shared_ptr<uint8_t> getSharedBufferReference(void* ptr) {
    {
      std::lock_guard<decltype(m_memoryMutex)> lk(m_memoryMutex);
      auto it = m_memory.find(ptr);
      if (it != m_memory.end()) {
        return it->second;
      }
    }
    return nullptr;
  }

protected:
  void onConnection(const LEAP_CONNECTION_EVENT *connection_event) {
    m_isServiceConnected = true;
    std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
    for (auto& listener : m_listeners) {
      listener->onServiceConnect(m_controller);
    }
    // Set the flags on connection so that requests prior to connection are honored
    setPolicyFlags(policyFlags());
  }

  void onConnectionLost(const LEAP_CONNECTION_LOST_EVENT *connection_lost_event) {
    m_isServiceConnected = false;
    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      for (auto& listener : m_listeners) {
        listener->onServiceDisconnect(m_controller);
      }
    }
    {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      m_devices.clear();
    }
  }

  void onDeviceStatus(const LEAP_DEVICE_REF& device) {
    const uint32_t id = device.id;
    auto impl = std::make_shared<DeviceImplementation>(device);
    bool newlyConnected;
    {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      newlyConnected = m_devices.empty();
      m_devices[id] = impl;
    }
    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      if (newlyConnected) {
        for (auto& listener : m_listeners) {
          listener->onConnect(m_controller);
        }
      }
      for (auto& listener : m_listeners) {
        listener->onDeviceChange(m_controller);
      }
    }
  }

  void onDevice(const LEAP_DEVICE_EVENT* device_event) {
    onDeviceStatus(device_event->device);
  }

  void onDeviceStatusChange(const LEAP_DEVICE_STATUS_CHANGE_EVENT* device_status_change_event) {
    onDeviceStatus(device_status_change_event->device);

    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      for (auto& listener : m_listeners) {
        const bool wasStreaming = device_status_change_event->last_status & eLeapDeviceStatus_Streaming;
        const bool isStreaming = device_status_change_event->status & eLeapDeviceStatus_Streaming;
        if(isStreaming && !wasStreaming)
          listener->onConnect(m_controller);
        else if(!isStreaming && wasStreaming)
          listener->onDisconnect(m_controller);
      }
    }
  }

  void onDeviceLost(const LEAP_DEVICE_EVENT* device_event) {
    std::shared_ptr<DeviceImplementation> impl;
    std::map<uint32_t, std::shared_ptr<DeviceImplementation>>::iterator it;
    bool newlyDisconnected = false;
    {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      it = m_devices.find(device_event->device.id);
      if (it == m_devices.end())
        return;
      impl = it->second;
      if (m_devices.size() == 1) {
        newlyDisconnected = true;
      }
    }
    if (impl)
      impl->m_info.status = device_event->status;
    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      for (auto& listener : m_listeners) {
        listener->onDeviceChange(m_controller);
      }
      if (newlyDisconnected) {
        for (auto& listener : m_listeners) {
          listener->onDisconnect(m_controller);
        }
      }
    }
    {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      m_devices.erase(it);
    }
  }

  void onDeviceFailure(const LEAP_DEVICE_FAILURE_EVENT* device_failure_event) {
    if (device_failure_event->hDevice) {
      std::lock_guard<decltype(m_deviceMutex)> lk(m_deviceMutex);
      for (const auto& device : m_devices) {
        if (device.second->m_device == device_failure_event->hDevice) {
          device.second->m_info.status = device_failure_event->status;
          break;
        }
      }
    }
    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      for (auto& listener : m_listeners) {
        listener->onDeviceFailure(m_controller);
      }
    }
  }

  void onTracking(const LEAP_TRACKING_EVENT *tracking_event) {
    auto impl = std::make_shared<FrameImplementation>(*tracking_event);
    const int64_t frame_id = impl->id();
    {
      std::lock_guard<decltype(m_imageMutex)> lk(m_imageMutex);
      for (const auto& images : m_images) {
        if (images.empty()) {
          continue;
        }
        const auto image_id = images[0]->sequenceId();
        if (frame_id == image_id) {
          impl->setImages(images);
          break;
        }
      }
    }
    {
      std::lock_guard<decltype(m_mapPointsMutex)> lk(m_mapPointsMutex);
      for (auto it = m_pointMappingBuffers.begin(); it != m_pointMappingBuffers.end(); ++it) {
        const LEAP_POINT_MAPPING& pointMapping = *reinterpret_cast<LEAP_POINT_MAPPING*>(it->get());
        if (frame_id == pointMapping.frame_id) {
          impl->setMapPoints(pointMapping);
          m_pointMappingBuffers.erase(it, m_pointMappingBuffers.end()); // Discard all older map points
          break;
        }
      }
    }
    {
      std::lock_guard<decltype(m_frameMutex)> lk(m_frameMutex);
      m_frames.emplace_front(std::move(impl));
      if (m_frames.size() > DEFAULT_FRAME_HISTORY_SIZE) {
        m_frames.pop_back();
      }
    }
    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      for (auto& listener : m_listeners) {
        listener->onFrame(m_controller);
      }
    }
  }

  void onLog(const LEAP_LOG_EVENT *log_event) {
    std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
    for (auto& listener : m_listeners) {
      listener->onLogMessage(m_controller, static_cast<MessageSeverity>(log_event->severity), log_event->timestamp, log_event->message);
    }
  }

  void onLogs(const LEAP_LOG_EVENTS *log_events) {
    std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
    for (int i = 0; i < static_cast<int>(log_events->nEvents); i++) {
      const LEAP_LOG_EVENT& log_event = log_events->events[i];
      for (auto& listener : m_listeners) {
        listener->onLogMessage(m_controller, static_cast<MessageSeverity>(log_event.severity), log_event.timestamp, log_event.message);
      }
    }
  }

  void onPolicy(const LEAP_POLICY_EVENT *policy_event) {
    m_policyFlags = policy_event->current_policy;
  }

  void onConfigChange(const LEAP_CONFIG_CHANGE_EVENT *config_change_event) {
//    ConnectionCallbacks.on_config_change(config_change_event->requestID, config_change_event->status);
  }

  void onConfigResponse(const LEAP_CONFIG_RESPONSE_EVENT *config_response_event) {
    std::lock_guard<std::mutex> lock(m_configPromiseMutex);
    //Perform the conversion here so that we can aquire the string - the config response event
    //is only garunteed to be valid inside this function.
    Config::Value value;
    const auto& leapVar = config_response_event->value;
    value.type = static_cast<Config::ValueType>(leapVar.type);
    switch (value.type) {
    case Config::TYPE_BOOLEAN:
      value.bValue = leapVar.boolValue;
      break;
    case Config::TYPE_INT32:
      value.iValue = leapVar.iValue;
      break;
    case Config::TYPE_FLOAT:
      value.fValue = leapVar.fValue;
      break;
    case Config::TYPE_STRING:
      value.strValue = leapVar.strValue ? strdup(leapVar.strValue) : nullptr;
      break;
    default:
      break;
    }
    //  If the promise hasn't been added already, the service responded faster
    //  than we could aquire the lock and the wait on getting the value will become a noop.
    m_configPromises[config_response_event->requestID].set_value(value);
  }

  void onImage(const LEAP_IMAGE_EVENT *image_event) {
    {
      std::vector<std::shared_ptr<ImageImplementation>> images;
      images.emplace_back(std::make_shared<ImageImplementation>(std::static_pointer_cast<ControllerImplementation>(shared_from_this()), *image_event, 0));
      images.emplace_back(std::make_shared<ImageImplementation>(std::static_pointer_cast<ControllerImplementation>(shared_from_this()), *image_event, 1));
      const int64_t image_id = images[0]->sequenceId();
      {
        std::lock_guard<decltype(m_imageMutex)> lk(m_imageMutex);
        m_images.emplace_front(images);
        if (m_images.size() > DEFAULT_FRAME_HISTORY_SIZE) {
          m_images.pop_back();
        }
      }
      {
        std::lock_guard<decltype(m_frameMutex)> lk(m_frameMutex);
        if (!m_frames.empty()) {
          for (const auto& frame : m_frames) {
            const int64_t frame_id = frame->id();
            if (image_id == frame_id) {
              frame->setImages(images);
              break;
            }
            if (image_id > frame_id) {
              break; // Image is newer than latest frame
            }
          }
        }
      }
    }
    std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
    for (auto& listener : m_listeners) {
      listener->onImages(m_controller);
    }
  }

  void onPointMappingChange(const LEAP_POINT_MAPPING_CHANGE_EVENT *point_mapping_change_event) {
    uint64_t size = 0;
    std::shared_ptr<uint8_t> buffer;
    LEAP_POINT_MAPPING* pointMapping = nullptr;
    for (;;) {
      pointMapping = reinterpret_cast<LEAP_POINT_MAPPING*>(buffer.get());
      auto status = LeapGetPointMapping(m_connection, pointMapping, &size);
      if (status == eLeapRS_InsufficientBuffer) {
        buffer = std::shared_ptr<uint8_t>(new uint8_t[size], std::default_delete<uint8_t[]>());
        continue;
      }
      if (status != eLeapRS_Success) {
        return;
      }
      break;
    }
    if (pointMapping->nPoints == 0) {
      return;
    }
    const int64_t map_id = pointMapping->frame_id;
    {
      std::lock_guard<decltype(m_frameMutex)> lk(m_frameMutex);
      if (!m_frames.empty()) {
        for (const auto& frame : m_frames) {
          const int64_t frame_id = frame->id();
          if (map_id == frame_id) {
            frame->setMapPoints(*pointMapping);
            return;
          }
          if (map_id > frame_id) {
            break; // Map points are newer than latest frame
          }
        }
      }
    }
    std::lock_guard<decltype(m_mapPointsMutex)> lk(m_mapPointsMutex);
    m_pointMappingBuffers.emplace_front(std::move(buffer));
    if (m_pointMappingBuffers.size() > DEFAULT_FRAME_HISTORY_SIZE) {
      m_pointMappingBuffers.pop_back();
    }
  }

  void onHeadPose(const LEAP_HEAD_POSE_EVENT *head_pose_event) {
    {
      std::lock_guard<decltype(m_listenerMutex)> lk(m_listenerMutex);
      for (auto& listener : m_listeners) {
        listener->onHeadPose(m_controller, head_pose_event->timestamp);
      }
    }
  }

  void* allocate(uint32_t size) {
    std::shared_ptr<uint8_t> buffer(new uint8_t[size], std::default_delete<uint8_t[]>());
    void* ptr = buffer.get();
    if (ptr) {
      std::lock_guard<decltype(m_memoryMutex)> lk(m_memoryMutex);
      m_memory.insert(std::make_pair(ptr, buffer));
    }
    return ptr;
  }

  void deallocate(void* ptr) {
    std::lock_guard<decltype(m_memoryMutex)> lk(m_memoryMutex);
    auto it = m_memory.find(ptr);
    if (it != m_memory.end()) {
      m_memory.erase(it);
    }
  }

  static void* staticAllocate(uint32_t size, eLeapAllocatorType typeHint, void* state) {
    return reinterpret_cast<ControllerImplementation*>(state)->allocate(size);
  };

  static void staticDeallocate(void* ptr, void* state) {
    if (!ptr)
      return;
    reinterpret_cast<ControllerImplementation*>(state)->deallocate(ptr);
  };

  const Controller& m_controller;
  LEAP_CONNECTION m_connection = nullptr;
  std::thread m_pollingThread;
  std::set<Listener*> m_listeners;
  std::map<uint32_t, std::shared_ptr<DeviceImplementation>> m_devices;
  std::deque<std::shared_ptr<FrameImplementation>> m_frames;
  std::deque<std::vector<std::shared_ptr<ImageImplementation>>> m_images;
  std::deque<std::shared_ptr<uint8_t>> m_pointMappingBuffers;
  std::mutex m_listenerMutex;
  std::mutex m_deviceMutex;
  std::mutex m_frameMutex;
  std::mutex m_imageMutex;
  std::mutex m_memoryMutex;
  std::mutex m_mapPointsMutex;
  std::mutex m_configPromiseMutex;
  std::map<uint32_t, std::promise<Leap::Config::Value>> m_configPromises;
  std::unordered_map<void*, std::shared_ptr<uint8_t>> m_memory;
  uint32_t m_policyFlags = 0;
  std::atomic<bool> m_isRunning{ false };
  bool m_isServiceConnected = false;

  static const int DEFAULT_FRAME_HISTORY_SIZE = 60;
};

}
