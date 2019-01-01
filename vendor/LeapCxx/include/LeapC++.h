/******************************************************************************\
* Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#if !defined(__Leap_h__)
#define __Leap_h__

#include "LeapMath.h"
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>

// Define Leap export macros
#ifndef LEAP_EXPORT
  #if defined(_MSC_VER) // Visual C++
    #if defined(LEAP_CPP_DYNAMIC)
      #if LEAP_CPP_IMPLEMENTATION
        #define LEAP_EXPORT __declspec(dllexport)
      #else
        #define LEAP_EXPORT __declspec(dllimport)
      #endif
    #else
      #define LEAP_EXPORT
    #endif
    #define LEAP_EXPORT_CLASS
  #elif !defined(SWIG)
    #define LEAP_EXPORT __attribute__((visibility("default")))
  #else
    #define LEAP_EXPORT
  #endif
#endif

#ifndef LEAP_EXPORT_CLASS
#if defined(_MSC_VER) // Visual C++
  #define LEAP_EXPORT_CLASS
#elif !defined(SWIG)
  #define LEAP_EXPORT_CLASS __attribute__((visibility("default")))
#else
  #define LEAP_EXPORT_CLASS
#endif
#endif

namespace Leap {

  // Interface for internal use only
  class LEAP_EXPORT_CLASS Interface {
  public:
    struct Implementation
#if !defined(SWIG)
    : std::enable_shared_from_this<Implementation>
#endif
    {
      virtual ~Implementation() {}
    };
  protected:
    Interface(const std::shared_ptr<Implementation>& impl) : m_impl(impl) {}
    virtual ~Interface() {}
    template<typename T> std::shared_ptr<T> as() const { return std::static_pointer_cast<T>(m_impl); }
    LEAP_EXPORT static void deleteCString(const char* cstr);
    std::shared_ptr<Implementation> m_impl;
  };

  // Forward declarations for internal use only
  class BoneImplementation;
  class FingerImplementation;
  class HandImplementation;
  class DeviceImplementation;
  class FailedDeviceImplementation;
  class ImageImplementation;
  class FrameImplementation;
  class HeadPoseImplementation;
  class ControllerImplementation;
  template<typename T> class ListBaseImplementation;

  // Forward declarations
  class FingerList;
  class HandList;
  class ImageList;
  class MapPointList;
  class Hand;
  class Frame;
  class HeadPose;
  class Listener;

  /**
   * The Arm class represents the forearm.
   *
   */
  class Arm : public Interface {
  public:
    // For internal use only.
    Arm(HandImplementation*);

    /**
    * Constructs an invalid Arm object.
    *
    * Get valid Arm objects from a Hand object.
    *
    * \include Arm_get.txt
    *
    * @since 2.0.3
    */
    LEAP_EXPORT Arm();

    /**
    * The average width of the arm.
    *
    * \include Arm_width.txt
    *
    * @since 2.0.3
    */
    LEAP_EXPORT float width() const;

    /**
    * The normalized direction in which the arm is pointing (from elbow to wrist).
    *
    * \include Arm_direction.txt
    *
    * @since 2.0.3
    */
    LEAP_EXPORT Vector direction() const;

    /**
     * The orthonormal basis vectors for the Arm bone as a Matrix.
     *
     * Basis vectors specify the orientation of a bone.
     *
     * **xBasis** Perpendicular to the longitudinal axis of the
     *   bone; exits the arm laterally through the sides of the wrist.
     *
     * **yBasis or up vector** Perpendicular to the longitudinal
     *   axis of the bone; exits the top and bottom of the arm. More positive
     *   in the upward direction.
     *
     * **zBasis** Aligned with the longitudinal axis of the arm bone.
     *   More positive toward the wrist.
     *
     * \include Arm_basis.txt
     *
     * The bases provided for the right arm use the right-hand rule; those for
     * the left arm use the left-hand rule. Thus, the positive direction of the
     * x-basis is to the right for the right arm and to the left for the left
     * arm. You can change from right-hand to left-hand rule by multiplying the
     * z basis vector by -1.
     *
     * Note that converting the basis vectors directly into a quaternion
     * representation is not mathematically valid. If you use quaternions,
     * create them from the derived rotation matrix not directly from the bases.
     *
     * @returns The basis of the arm bone as a matrix.
     * @since 2.0.3
     */
    LEAP_EXPORT Matrix basis() const;

    /**
    * The position of the elbow.
    *
    * \include Arm_elbowPosition.txt
    *
    * If not in view, the elbow position is estimated based on typical human
    * anatomical proportions.
    *
    * @since 2.0.3
    */
    LEAP_EXPORT Vector elbowPosition() const;

    /**
    * The position of the wrist.
    *
    * \include Arm_wristPosition.txt
    *
    * Note that the wrist position is not collocated with the end of any bone in
    * the hand. There is a gap of a few centimeters since the carpal bones are
    * not included in the skeleton model.
    *
    * @since 2.0.3
    */
    LEAP_EXPORT Vector wristPosition() const;

    /**
    * The center of the forearm.
    *
    * This location represents the midpoint of the arm between the wrist position
    * and the elbow position.
    *
    * @since 2.1.0
    */
    LEAP_EXPORT Vector center() const;

    /**
    * Reports whether this is a valid Arm object.
    *
    * \include Arm_isValid.txt
    *
    * @returns True, if this Arm object contains valid tracking data.
    * @since 2.0.3
    */
    LEAP_EXPORT bool isValid() const;

    /**
     * Returns an invalid Arm object.
     *
     * \include Arm_invalid.txt
     *
     * @returns The invalid Arm instance.
     * @since 2.0.3
     */
    LEAP_EXPORT static const Arm& invalid();

    /**
    * Compare Arm object equality.
    *
    * \include Arm_operator_equals.txt
    *
    * Two Arm objects are equal if and only if both Arm objects represent the
    * exact same physical arm in the same frame and both Arm objects are valid.
    * @since 2.0.3
    */
    LEAP_EXPORT bool operator==(const Arm&) const;

    /**
    * Compare Arm object inequality.
    *
    * \include Arm_operator_not_equals.txt
    *
    * Two Arm objects are equal if and only if both Arm objects represent the
    * exact same physical arm in the same frame and both Arm objects are valid.
    * @since 2.0.3
    */
    LEAP_EXPORT bool operator!=(const Arm&) const;

    /**
    * Writes a brief, human readable description of the Arm object to an output stream.
    *
    * \include Arm_stream.txt
    *
    * @since 2.0.3
    */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Arm&);

    /**
    * A string containing a brief, human readable description of the Arm object.
    *
    * \include Arm_toString.txt
    *
    * @returns A description of the Arm object as a string.
    * @since 2.0.3
    */
    std::string toString() const {
      return isValid() ? "Valid Arm" : "Invalid Arm";
    }
  };

  /**
   * The Bone class represents a tracked bone.
   *
   * All fingers contain 4 bones that make up the anatomy of the finger.
   * Get valid Bone objects from a Finger object.
   *
   * Bones are ordered from base to tip, indexed from 0 to 3.  Additionally, the
   * bone's Type enum may be used to index a specific bone anatomically.
   *
   * \include Bone_iteration.txt
   *
   * The thumb does not have a base metacarpal bone and therefore contains a valid,
   * zero length bone at that location.
   *
   * Note that Bone objects can be invalid, which means that they do not contain
   * valid tracking data and do not correspond to a physical bone. Invalid Bone
   * objects can be the result of asking for a Bone object from an invalid finger,
   * indexing a bone out of range, or constructing a new bone.
   * Test for validity with the Bone::isValid() function.
   * @since 2.0
   */
  class Bone : public Interface {
  public:
    /**
     * Enumerates the names of the bones.
     *
     * Members of this enumeration are returned by Bone::type() to identify a
     * Bone object.
     * @since 2.0
     */
    enum Type {
      TYPE_METACARPAL = 0,   /**< Bone connected to the wrist inside the palm */
      TYPE_PROXIMAL = 1,     /**< Bone connecting to the palm */
      TYPE_INTERMEDIATE = 2, /**< Bone between the tip and the base*/
      TYPE_DISTAL = 3,       /**< Bone at the tip of the finger */
    };

    // For internal use only.
    Bone(BoneImplementation*);

     /**
     * Constructs an invalid Bone object.
     *
     * \include Bone_invalid.txt
     *
     * Get valid Bone objects from a Finger object.
     *
     * @since 2.0
     */
    LEAP_EXPORT Bone();

    /**
     * The base of the bone, closest to the wrist.
     *
     * In anatomical terms, this is the proximal end of the bone.

     * \include Bone_prevJoint.txt
     *
     * @returns The Vector containing the coordinates of the previous joint position.
     * @since 2.0
     */
    LEAP_EXPORT Vector prevJoint() const;

    /**
     * The end of the bone, closest to the finger tip.
     *
     * In anatomical terms, this is the distal end of the bone.
     *
     * \include Bone_nextJoint.txt
     *
     * @returns The Vector containing the coordinates of the next joint position.
     * @since 2.0
     */
    LEAP_EXPORT Vector nextJoint() const;

    /**
     * The midpoint of the bone.
     *
     * \include Bone_center.txt
     *
     * @returns The midpoint in the center of the bone.
     * @since 2.0
     */
    LEAP_EXPORT Vector center() const;

    /**
     * The normalized direction of the bone from base to tip.
     *
     * \include Bone_direction.txt
     *
     * @returns The normalized direction of the bone from base to tip.
     * @since 2.0
     */
    LEAP_EXPORT Vector direction() const;

    /**
     * The estimated length of the bone in millimeters.
     *
     * \include Bone_length.txt
     *
     * @returns The length of the bone in millimeters.
     * @since 2.0
     */
    LEAP_EXPORT float length() const;

    /**
     * The average width of the flesh around the bone in millimeters.
     *
     * \include Bone_width.txt
     *
     * @returns The width of the flesh around the bone in millimeters.
     * @since 2.0
     */
    LEAP_EXPORT float width() const;

    /**
     * The name of this bone.
     *
     * \include Bone_type.txt
     *
     * @returns The anatomical type of this bone as a member of the Bone::Type
     * enumeration.
     * @since 2.0
     */
    LEAP_EXPORT Type type() const;

    /**
     * The orthonormal basis vectors for this Bone as a Matrix.
     *
     * Basis vectors specify the orientation of a bone.
     *
     * **xBasis** Perpendicular to the longitudinal axis of the
     *   bone; exits the sides of the finger.
     *
     * **yBasis or up vector** Perpendicular to the longitudinal
     *   axis of the bone; exits the top and bottom of the finger. More positive
     *   in the upward direction.
     *
     * **zBasis** Aligned with the longitudinal axis of the bone.
     *   More positive toward the base of the finger.
     *
     * The bases provided for the right hand use the right-hand rule; those for
     * the left hand use the left-hand rule. Thus, the positive direction of the
     * x-basis is to the right for the right hand and to the left for the left
     * hand. You can change from right-hand to left-hand rule by multiplying the
     * z basis vector by -1.
     *
     * You can use the basis vectors for such purposes as measuring complex
     * finger poses and skeletal animation.
     *
     * Note that converting the basis vectors directly into a quaternion
     * representation is not mathematically valid. If you use quaternions,
     * create them from the derived rotation matrix not directly from the bases.
     *
     * \include Bone_basis.txt
     *
     * @returns The basis of the bone as a matrix.
     * @since 2.0
     */
    LEAP_EXPORT Matrix basis() const;

    /**
     * Reports whether this is a valid Bone object.
     *
     * \include Bone_isValid.txt
     *
     * @returns True, if this Bone object contains valid tracking data.
     * @since 2.0
     */
    LEAP_EXPORT bool isValid() const;

    /**
     * Returns an invalid Bone object.
     *
     * You can use the instance returned by this function in comparisons testing
     * whether a given Bone instance is valid or invalid. (You can also use the
     * Bone::isValid() function.)
     *
     * \include Bone_invalid.txt
     *
     * @returns The invalid Bone instance.
     * @since 2.0
     */
    LEAP_EXPORT static const Bone& invalid();

    /**
     * Compare Bone object equality.
     *
     * Two Bone objects are equal if and only if both Bone objects represent the
     * exact same physical bone in the same frame and both Bone objects are valid.
     * @since 2.0
     */
    LEAP_EXPORT bool operator==(const Bone&) const;

    /**
     * Compare Bone object inequality.
     *
     * Two Bone objects are equal if and only if both Bone objects represent the
     * exact same physical bone in the same frame and both Bone objects are valid.
     * @since 2.0
     */
    LEAP_EXPORT bool operator!=(const Bone&) const;

    /**
     * Writes a brief, human readable description of the Bone object to an output stream.
     *
     * @since 2.0
     */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Bone&);

    /**
     * A string containing a brief, human readable description of the Bone object.
     *
     * \include Bone_toString.txt
     *
     * @returns A description of the Bone object as a string.
     * @since 2.0
     */
    std::string toString() const {
      size_t length = 0;
      const char* str = toCString(length);
      return std::string(str, length);
    }

  private:
    LEAP_EXPORT const char* toCString(size_t& length) const;
  };

  /**
   * The Finger class represents a tracked finger.
   *
   * Fingers are objects that the Leap Motion software has classified as a finger.
   * Get valid Finger objects from a Frame or a Hand object.
   *
   * Fingers may be permanently associated to a hand. In this case the angular order of the finger IDs
   * will be invariant. As fingers move in and out of view it is possible for the guessed ID
   * of a finger to be incorrect. Consequently, it may be necessary for finger IDs to be
   * exchanged. All tracked properties, such as velocity, will remain continuous in the API.
   * However, quantities that are derived from the API output (such as a history of positions)
   * will be discontinuous unless they have a corresponding ID exchange.
   *
   * Note that Finger objects can be invalid, which means that they do not contain
   * valid tracking data and do not correspond to a physical finger. Invalid Finger
   * objects can be the result of asking for a Finger object using an ID from an
   * earlier frame when no Finger objects with that ID exist in the current frame.
   * A Finger object created from the Finger constructor is also invalid.
   * Test for validity with the Finger::isValid() function.
   * @since 1.0
   */
  class Finger : public Interface {
  public:
    /**
     * Enumerates the names of the fingers.
     *
     * Members of this enumeration are returned by Finger::type() to identify a
     * Finger object.
     * @since 2.0
     */
    enum Type {
      TYPE_THUMB  = 0, /**< The thumb */
      TYPE_INDEX  = 1, /**< The index or fore-finger */
      TYPE_MIDDLE = 2, /**< The middle finger */
      TYPE_RING   = 3, /**< The ring finger */
      TYPE_PINKY  = 4  /**< The pinky or little finger */
    };

    // For internal use only.
    Finger(FingerImplementation*);

    /**
     * Constructs a Finger object.
     *
     * An uninitialized finger is considered invalid.
     * Get valid Finger objects from a Frame or a Hand object.
     * @since 1.0
     */
    LEAP_EXPORT Finger();

    /**
     * The Frame associated with this Finger object.
     *
     * \include Finger_frame.txt
     *
     * @returns The associated Frame object, if available; otherwise,
     * an invalid Frame object is returned.
     * @since 1.0
     */
    LEAP_EXPORT Frame frame() const;

    /**
     * The Hand associated with a finger.
     *
     * \include Finger_hand.txt
     *
     * This function always returns an invalid Hand object.
     *
     * @returns The associated Hand object, if available; otherwise,
     * an invalid Hand object is returned.
     * @since 1.0
     */
    LEAP_EXPORT Hand hand() const;

    /**
     * The name of this finger.
     *
     * \include Finger_type.txt
     *
     * @returns The anatomical type of this finger as a member of the Finger::Type
     * enumeration.
     * @since 2.0
     */
    LEAP_EXPORT Type type() const;

    /**
     * A unique ID assigned to this Finger object, whose value remains the
     * same across consecutive frames while the tracked finger remains
     * visible. If tracking is lost (for example, when a finger is occluded by
     * another finger or when it is withdrawn from the Leap Motion Controller field of view), the
     * Leap Motion software may assign a new ID when it detects the entity in a future frame.
     *
     * \include Finger_id.txt
     *
     * @returns The ID assigned to this Finger object.
     * @since 1.0
     */
    LEAP_EXPORT int32_t id() const;

    /**
     * The bone at a given bone index on this finger.
     *
     * \include Bone_iteration.txt
     *
     * @param boneIx An index value from the Bone::Type enumeration identifying the
     * bone of interest.
     * @returns The Bone that has the specified bone type.
     * @since 2.0
     */
    LEAP_EXPORT Bone bone(Bone::Type boneIx) const;

    /**
     * The tip position in millimeters from the Leap Motion origin.
     *
     * \include Finger_tipPosition.txt
     *
     * @returns The Vector containing the coordinates of the tip position.
     * @since 1.0
     */
    LEAP_EXPORT Vector tipPosition() const;

    /**
     * The direction in which this finger is pointing.
     *
     * \include Finger_direction.txt
     *
     * The direction is expressed as a unit vector pointing in the same
     * direction as the tip.
     *
     * \image html images/Leap_Finger_Model.png
     *
     * @returns The Vector pointing in the same direction as the tip of this
     * Finger object.
     * @since 1.0
     */
    LEAP_EXPORT Vector direction() const;

    /**
     * The estimated width of the finger in millimeters.
     *
     * \include Finger_width.txt
     *
     * @returns The estimated width of this Finger object.
     * @since 1.0
     */
    LEAP_EXPORT float width() const;

    /**
     * The estimated length of the finger in millimeters.
     *
     * \include Finger_length.txt
     *
     * @returns The estimated length of this Finger object.
     * @since 1.0
     */
    LEAP_EXPORT float length() const;

    /**
     * Whether or not this Finger is in an extended posture.
     *
     * A finger is considered extended if it is extended straight from the hand as if
     * pointing. A finger is not extended when it is bent down and curled towards the
     * palm.
     *
     * \include Finger_isExtended.txt
     *
     * @returns True, if the finger is extended.
     * @since 2.0
     */
    LEAP_EXPORT bool isExtended() const;

    /**
     * The duration of time this Finger has been visible to the Leap Motion Controller.
     *
     * \include Finger_timeVisible.txt
     *
     * @returns The duration (in seconds) that this Finger has been tracked.
     * @since 1.0
     */
    LEAP_EXPORT float timeVisible() const;

    /**
     * Reports whether this is a valid Finger object.
     *
     * \include Finger_isValid.txt
     *
     * @returns True, if this Finger object contains valid tracking data.
     * @since 1.0
     */
    LEAP_EXPORT bool isValid() const;

    /**
     * Compare Finger object equality.
     *
     * \include Finger_operator_equals.txt
     *
     * Two Finger objects are equal if and only if both Finger objects represent the
     * exact same physical entities in the same frame and both Finger objects are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator==(const Finger&) const;

    /**
     * Compare Finger object inequality.
     *
     * \include Finger_operator_not_equal.txt
     *
     * Two Finger objects are equal if and only if both Finger objects represent the
     * exact same physical entities in the same frame and both Finger objects are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator!=(const Finger&) const;

    /**
     * Writes a brief, human readable description of the Finger object to an output stream.
     *
     * \include Fingere_operator_stream.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Finger&);

    /**
     * Returns an invalid Finger object.
     *
     * You can use the instance returned by this function in comparisons testing
     * whether a given Finger instance is valid or invalid. (You can also use the
     * Finger::isValid() function.)
     *
     * \include Finger_invalid.txt
     *
     * @returns The invalid Finger instance.
     * @since 1.0
     */
    LEAP_EXPORT static const Finger& invalid();

    /**
     * A string containing a brief, human readable description of the Finger object.
     *
     * \include Finger_toString.txt
     *
     * @returns A description of the Finger object as a string.
     * @since 1.0
     */
    std::string toString() const {
      size_t length = 0;
      const char* str = toCString(length);
      return std::string(str, length);
    }

  private:
    LEAP_EXPORT const char* toCString(size_t& length) const;
  };

  /**
   * The Hand class reports the physical characteristics of a detected hand.
   *
   * Hand tracking data includes a palm position and velocity; vectors for
   * the palm normal and direction to the fingers; properties of a sphere fit
   * to the hand; and lists of the attached fingers.
   *
   * Get Hand objects from a Frame object:
   *
   * \include Hand_Get_First.txt
   *
   * Note that Hand objects can be invalid, which means that they do not contain
   * valid tracking data and do not correspond to a physical entity. Invalid Hand
   * objects can be the result of asking for a Hand object using an ID from an
   * earlier frame when no Hand objects with that ID exist in the current frame.
   * A Hand object created from the Hand constructor is also invalid.
   * Test for validity with the Hand::isValid() function.
   * @since 1.0
   */
  class Hand : public Interface {
  public:
    // For internal use only.
    Hand(HandImplementation*);

    /**
     * Constructs a Hand object.
     *
     * An uninitialized hand is considered invalid.
     * Get valid Hand objects from a Frame object.
     *
     * \include Hand_Hand.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT Hand();

    /**
     * A unique ID assigned to this Hand object, whose value remains the same
     * across consecutive frames while the tracked hand remains visible. If
     * tracking is lost (for example, when a hand is occluded by another hand
     * or when it is withdrawn from or reaches the edge of the Leap Motion Controller field of view),
     * the Leap Motion software may assign a new ID when it detects the hand in a future frame.
     *
     * Use the ID value with the Frame::hand() function to find this Hand object
     * in future frames:
     *
     * \include Hand_Get_ID.txt
     *
     * @returns The ID of this hand.
     * @since 1.0
     */
    LEAP_EXPORT int32_t id() const;

    /**
     * The Frame associated with this Hand.
     *
     * \include Hand_frame.txt
     *
     * @returns The associated Frame object, if available; otherwise,
     * an invalid Frame object is returned.
     * @since 1.0
     */
    LEAP_EXPORT Frame frame() const;

    /**
     * The list of Finger objects detected in this frame that are attached to
     * this hand, given in order from thumb to pinky.  The list cannot be empty.
     *
     * Use FingerList::extended() to remove non-extended fingers from the list.
     *
     * \include Hand_Get_Fingers.txt
     *
     * @returns The FingerList containing all Finger objects attached to this hand.
     * @since 1.0
     */
    LEAP_EXPORT FingerList fingers() const;

    /**
     * The Finger object with the specified ID attached to this hand.
     *
     * Use the Hand::finger() function to retrieve a Finger object attached to
     * this hand using an ID value obtained from a previous frame.
     * This function always returns a Finger object, but if no finger
     * with the specified ID is present, an invalid Finger object is returned.
     *
     * \include Hand_finger.txt
     *
     * Note that ID values persist across frames, but only until tracking of a
     * particular object is lost. If tracking of a finger is lost and subsequently
     * regained, the new Finger object representing that finger may have a
     * different ID than that representing the finger in an earlier frame.
     *
     * @param id The ID value of a Finger object from a previous frame.
     * @returns The Finger object with the matching ID if one exists for this
     * hand in this frame; otherwise, an invalid Finger object is returned.
     * @since 1.0
     */
    LEAP_EXPORT Finger finger(int32_t id) const;

    /**
     * The center position of the palm in millimeters from the Leap Motion Controller origin.
     *
     * \include Hand_palmPosition.txt
     *
     * @returns The Vector representing the coordinates of the palm position.
     * @since 1.0
     */
    LEAP_EXPORT Vector palmPosition() const;

    /**
     * The stabilized palm position of this Hand.
     *
     * Smoothing and stabilization is performed in order to make
     * this value more suitable for interaction with 2D content. The stabilized
     * position lags behind the palm position by a variable amount, depending
     * primarily on the speed of movement.
     *
     * \include Hand_stabilizedPalmPosition.txt
     *
     * @returns A modified palm position of this Hand object
     * with some additional smoothing and stabilization applied.
     * @since 1.0
     */
    LEAP_EXPORT Vector stabilizedPalmPosition() const;

    /**
     * The rate of change of the palm position in millimeters/second.
     *
     * \include Hand_palmVelocity.txt
     *
     * @returns The Vector representing the coordinates of the palm velocity.
     * @since 1.0
     */
    LEAP_EXPORT Vector palmVelocity() const;

    /**
     * The normal vector to the palm. If your hand is flat, this vector will
     * point downward, or "out" of the front surface of your palm.
     *
     * \image html images/Leap_Palm_Vectors.png
     *
     * The direction is expressed as a unit vector pointing in the same
     * direction as the palm normal (that is, a vector orthogonal to the palm).
     *
     * You can use the palm normal vector to compute the roll angle of the palm with
     * respect to the horizontal plane:
     *
     * \include Hand_Get_Angles.txt
     *
     * @returns The Vector normal to the plane formed by the palm.
     * @since 1.0
     */
    LEAP_EXPORT Vector palmNormal() const;

    /**
     * The estimated width of the palm when the hand is in a flat position.
     *
     * \include Hand_palmWidth.txt
     *
     * @returns The width of the palm in millimeters
     * @since 2.0
     */
    LEAP_EXPORT float palmWidth() const;

    /**
     * The direction from the palm position toward the fingers.
     *
     * The direction is expressed as a unit vector pointing in the same
     * direction as the directed line from the palm position to the fingers.
     *
     * You can use the palm direction vector to compute the pitch and yaw angles of the palm with
     * respect to the horizontal plane:
     *
     * \include Hand_Get_Angles.txt
     *
     * @returns The Vector pointing from the palm position toward the fingers.
     * @since 1.0
     */
    LEAP_EXPORT Vector direction() const;

    /**
     * The orientation of the hand as a basis matrix.
     *
     * The basis is defined as follows:
     *
     * **xAxis** Positive in the direction of the pinky
     *
     * **yAxis** Positive above the hand
     *
     * **zAxis** Positive in the direction of the wrist
     *
     * Note: Since the left hand is a mirror of the right hand, the
     * basis matrix will be left-handed for left hands.
     *
     * \include Hand_basis.txt
     *
     * @returns The basis of the hand as a matrix.
     * @since 2.0
     */
    LEAP_EXPORT Matrix basis() const;

    /**
     * The arm to which this hand is attached.
     *
     * If the arm is not completely in view, Arm attributes are estimated based on
     * the attributes of entities that are in view combined with typical human anatomy.
     *
     * \include Arm_get.txt
     *
     * @returns The Arm object for this hand.
     * @since 2.0.3
     */
    LEAP_EXPORT Arm arm() const;

    /**
     * The position of the wrist of this hand.
     *
     * @returns A vector containing the coordinates of the wrist position in millimeters.
     * @since 2.0.3
     */
    LEAP_EXPORT Vector wristPosition() const;

    /**
     * The distance between the thumb and index finger of a pinch hand pose.
     *
     * The distance is computed by looking at the shortest distance between
     * the last 2 phalanges of the thumb and those of the index finger.
     * This pinch measurement only takes thumb and index finger into account.
     *
     * \include Hand_pinchDistance.txt
     *
     * @returns The distance between the thumb and index finger of a pinch hand
     * pose in millimeters.
     * @since 3.0
     */
    LEAP_EXPORT float pinchDistance() const;

    /**
     * The angle between the fingers and the hand of a grab hand pose.
     *
     * The angle is computed by looking at the angle between the direction of the
     * 4 fingers and the direction of the hand. Thumb is not considered when
     * computing the angle.
     * The angle is 0 radian for an open hand, and reaches pi radians when the pose
     * is a tight fist.
     *
     * \include Hand_grabAngle.txt
     *
     * @returns The angle of a grab hand pose between 0 and pi radians (0 and 180 degrees).
     * @since 3.0
     */
    LEAP_EXPORT float grabAngle() const;

    /**
     * Deprecated. Use pinchDistance() instead.
     *
     * @since 2.0
     * @deprecated 3.0
     */
    LEAP_EXPORT float pinchStrength() const;

    /**
     * Deprecated. Use grabAngle() instead.
     *
     * @since 2.0
     * @deprecated 3.0
     */
    LEAP_EXPORT float grabStrength() const;

    /**
     * The duration of time this Hand has been visible to the Leap Motion Controller.
     *
     * \include Hand_timeVisible.txt
     *
     * @returns The duration (in seconds) that this Hand has been tracked.
     * @since 1.0
     */
    LEAP_EXPORT float timeVisible() const;

    /**
    * How confident we are with a given hand pose.
    *
    * The confidence level ranges between 0.0 and 1.0 inclusive.
    *
    * \include Hand_confidence.txt
    *
    * @since 2.0
    */
    LEAP_EXPORT float confidence() const;

    /**
     * Identifies whether this Hand is a left hand.
     *
     * \include Hand_isLeft.txt
     *
     * @returns True if the hand is identified as a left hand.
     * @since 2.0
     */
    LEAP_EXPORT bool isLeft() const;

    /**
     * Identifies whether this Hand is a right hand.
     *
     * \include Hand_isRight.txt
     *
     * @returns True if the hand is identified as a right hand.
     * @since 2.0
     */
    LEAP_EXPORT bool isRight() const;

    /**
     * Reports whether this is a valid Hand object.
     *
     * \include Hand_isValid.txt
     *
     * @returns True, if this Hand object contains valid tracking data.
     * @since 1.0
     */
    LEAP_EXPORT bool isValid() const;

    /**
     * Returns an invalid Hand object.
     *
     * \include Hand_invalid.txt
     *
     * You can use the instance returned by this function in comparisons testing
     * whether a given Hand instance is valid or invalid. (You can also use the
     * Hand::isValid() function.)
     *
     * @returns The invalid Hand instance.
     * @since 1.0
     */
    LEAP_EXPORT static const Hand& invalid();

    /**
     * Compare Hand object equality.
     *
     * \include Hand_operator_equals.txt
     *
     * Two Hand objects are equal if and only if both Hand objects represent the
     * exact same physical hand in the same frame and both Hand objects are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator==(const Hand&) const;

    /**
     * Compare Hand object inequality.
     *
     * \include Hand_operator_not_equals.txt
     *
     * Two Hand objects are equal if and only if both Hand objects represent the
     * exact same physical hand in the same frame and both Hand objects are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator!=(const Hand&) const;

    /**
     * Writes a brief, human readable description of the Hand object to an output stream.
     *
     * \include Hand_operator_stream.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Hand&);

    /**
     * A string containing a brief, human readable description of the Hand object.
     *
     * @returns A description of the Hand as a string.
     * @since 1.0
     */
    std::string toString() const {
      size_t length = 0;
      const char* str = toCString(length);
      return std::string(str, length);
    }

  private:
    LEAP_EXPORT const char* toCString(size_t& length) const;
  };

  /**
   * The structure containing a MapPoint. It consists of a unique
   * identifier along with the 3D point.
   */
  struct MapPoint {
    uint32_t id;
    Vector point;

    static const MapPoint& invalid() {
      static MapPoint s_invalid = {0};
      return s_invalid;
    }
  };

  /**
   * The Device class represents a physically connected device.
   *
   * The Device class contains information related to a particular connected
   * device such as device id, field of view relative to the device,
   * and the position and orientation of the device in relative coordinates.
   *
   * The position and orientation describe the alignment of the device relative to the user.
   * The alignment relative to the user is only descriptive. Aligning devices to users
   * provides consistency in the parameters that describe user interactions.
   *
   * Note that Device objects can be invalid, which means that they do not contain
   * valid device information and do not correspond to a physical device.
   * Test for validity with the Device::isValid() function.
   * @since 1.0
   */
  class Device : public Interface {
  public:

    // For internal use only.
    Device(DeviceImplementation*);

    /**
     * Constructs a Device object.
     *
     * An uninitialized device is considered invalid.
     * Get valid Device objects from a DeviceList object obtained using the
     * Controller::devices() method.
     *
     * \include Device_Device.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT Device();

    /**
     * The angle of view along the x axis of this device.
     *
     * \image html images/Leap_horizontalViewAngle.png
     *
     * The Leap Motion controller scans a region in the shape of an inverted pyramid
     * centered at the device's center and extending upwards. The horizontalViewAngle
     * reports the view angle along the long dimension of the device.
     *
     * \include Device_horizontalViewAngle.txt
     *
     * @returns The horizontal angle of view in radians.
     * @since 1.0
     */
    LEAP_EXPORT float horizontalViewAngle() const;

    /**
     * The angle of view along the z axis of this device.
     *
     * \image html images/Leap_verticalViewAngle.png
     *
     * The Leap Motion controller scans a region in the shape of an inverted pyramid
     * centered at the device's center and extending upwards. The verticalViewAngle
     * reports the view angle along the short dimension of the device.
     *
     * \include Device_verticalViewAngle.txt
     *
     * @returns The vertical angle of view in radians.
     * @since 1.0
     */
    LEAP_EXPORT float verticalViewAngle() const;

    /**
     * The maximum reliable tracking range from the center of this device.
     *
     * The range reports the maximum recommended distance from the device center
     * for which tracking is expected to be reliable. This distance is not a hard limit.
     * Tracking may be still be functional above this distance or begin to degrade slightly
     * before this distance depending on calibration and extreme environmental conditions.
     *
     * \include Device_range.txt
     *
     * @returns The recommended maximum range of the device in mm.
     * @since 1.0
     */
    LEAP_EXPORT float range() const;

    /**
     * The distance between the center points of the stereo sensors.
     *
     * The baseline value, together with the maximum resolution, influence the
     * maximum range.
     *
     * @returns The separation distance between the center of each sensor, in mm.
     * @since 2.2.5
     */
    LEAP_EXPORT float baseline() const;

    /**
    * The distance to the nearest edge of the Leap Motion controller's view volume.
    *
    * The view volume is an axis-aligned, inverted pyramid centered on the device origin
    * and extending upward to the range limit. The walls of the pyramid are described
    * by the horizontalViewAngle and verticalViewAngle and the roof by the range.
    * This function estimates the distance between the specified input position and the
    * nearest wall or roof of the view volume.
    *
    * \include Device_distanceToBoundary.txt
    *
    * @param position The point to use for the distance calculation.
    * @returns The distance in millimeters from the input position to the nearest boundary.
    * @since 1.0
    */
    LEAP_EXPORT float distanceToBoundary(const Vector& position) const;

    /**
     * Reports whether this device is streaming data to your application.
     *
     * Currently only one controller can provide data at a time.
     * @since 1.2
     */
    LEAP_EXPORT bool isStreaming() const;

    /**
     * The software has detected a possible smudge on the translucent cover
     * over the Leap Motion cameras.
     *
     * \include Device_isSmudged.txt
     *
     * @since 3.0
     */
    LEAP_EXPORT bool isSmudged() const;

    /**
     * The software has detected excessive IR illumination, which may interfere
     * with tracking. If robust mode is enabled, the system will enter robust mode when
     * isLightingBad() is true.
     *
     * \include Device_isLightingBad.txt
     *
     * @since 3.0
     */
    LEAP_EXPORT bool isLightingBad() const;

    /**
     * An alphanumeric serial number unique to each device.
     *
     * Consumer device serial numbers consist of 2 letters followed by 11 digits.
     *
     * When using multiple devices, the serial number provides an unambiguous
     * identifier for each device.
     * @since 2.2.2
     */
    std::string serialNumber() const {
      size_t length = 0;
      const char* str = serialNumberCString(length);
      return std::string(str, length);
    }

    /**
     * An enum representing different possible types of device.
     * @since 3.0
     */
    enum Type {
      UNKNOWN = 0,
      TYPE_PERIPHERAL = 0x0003,
      TYPE_NIGHTCRAWLER = 0x1201,
      TYPE_RIGEL = 0x1202
    };

    /**
     * A static function that returns the device type as a string
     * @since 4.0
     */
    static LEAP_EXPORT const char* typeString(Type t);

    /**
     * A convienence function that returns this device's type as a string
     * @since 4.0
     */
    LEAP_EXPORT const char* typeString() { return typeString(type()); }

    LEAP_EXPORT Type type() const;

    /**
     * Reports whether this is a valid Device object.
     *
     * \include Device_isValid.txt
     *
     * @returns True, if this Device object contains valid data.
     * @since 1.0
     */
    LEAP_EXPORT bool isValid() const;

    /**
     * Returns an invalid Device object.
     *
     * You can use the instance returned by this function in comparisons testing
     * whether a given Device instance is valid or invalid. (You can also use the
     * Device::isValid() function.)
     *
     * \include Device_invalid.txt
     *
     * @returns The invalid Device instance.
     * @since 1.0
     */
    LEAP_EXPORT static const Device& invalid();

    /**
     * Compare Device object equality.
     *
     * \include Device_operator_equals.txt
     *
     * Two Device objects are equal if and only if both Device objects represent the
     * exact same Device and both Devices are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator==(const Device&) const;

    /**
     * Compare Device object inequality.
     *
     * \include Device_operator_not_equals.txt
     *
     * Two Device objects are equal if and only if both Device objects represent the
     * exact same Device and both Devices are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator!=(const Device&) const;

    /**
     * Writes a brief, human readable description of the Device object.
     *
     * \include Device_operator_stream.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Device&);

    /**
     * A string containing a brief, human readable description of the Device object.
     *
     * @returns A description of the Device as a string.
     * @since 1.0
     */
    std::string toString() const {
      size_t length = 0;
      const char* str = toCString(length);
      return std::string(str, length);
    }

  private:
    LEAP_EXPORT const char* toCString(size_t& length) const;
    LEAP_EXPORT const char* serialNumberCString(size_t& length) const;
  };

  /**
  * The FailedDevice class provides information about Leap Motion hardware that
  * has been physically connected to the client computer, but is not operating
  * correctly.
  *
  * Failed devices do not provide any tracking data and do not show up in the
  * Controller:devices() list.
  *
  * Get the list of failed devices using Controller::failedDevices().
  *
  * \include FailedDevice_class.txt
  *
  * @since 3.0
  */
  class FailedDevice : public Interface {
  public:
    /**
    * The errors that can cause a device to fail to properly connect to the service.
    *
    * @since 3.0
    */
    enum FailureType {
      /** The cause of the error is unknown.
      * @since 3.0
      */
      FAIL_UNKNOWN,
      /** The device has a bad calibration record.
      * @since 3.0
      */
      FAIL_CALIBRATION,
      /** The device firmware is corrupt or failed to update.
      * @since 3.0
      */
      FAIL_FIRMWARE,
      /** The device is unresponsive.
      * @since 3.0
      */
      FAIL_TRANSPORT,
      /** The service cannot establish the required USB control interfaces.
      * @since 3.0
      */
      FAIL_CONTROL,
      /** Not currently used.
      * @since 3.0
      */
      FAIL_COUNT
    };

    // For internal use only.
    FailedDevice(FailedDeviceImplementation*);
    LEAP_EXPORT FailedDevice();

    /**
    * The device plug-and-play id string.
    * @since 3.0
    */
    std::string pnpId() const {
      size_t length = 0;
      const char* str = pnpIdCString(length);
      return std::string(str, length);
    }

    /**
    * The reason for device failure.
    *
    * The failure reasons are defined as members of the FailureType enumeration:
    *
    * **FailureType::FAIL_UNKNOWN**  The cause of the error is unknown.
    *
    * **FailureType::FAIL_CALIBRATION** The device has a bad calibration record.
    *
    * **FailureType::FAIL_FIRMWARE** The device firmware is corrupt or failed to update.
    *
    * **FailureType::FAIL_TRANSPORT** The device is unresponsive.
    *
    * **FailureType::FAIL_CONTROL** The service cannot establish the required USB control interfaces.
    *
    * **FailureType::FAIL_COUNT** Not currently used.
    *
    * @since 3.0
    */
    LEAP_EXPORT FailureType failure() const;

    /**
    * Reports whether this FailedDevice object contains valid data.
    * An invalid FailedDevice does not represent a physical device and can
    * be the result of creating a new FailedDevice object with the constructor.
    * Get FailedDevice objects from Controller::failedDevices() only.
    * @since 3.0
    */
    LEAP_EXPORT bool isValid() const;
    /**
    * An invalid FailedDevice object.
    *
    * @since 3.0
    */
    LEAP_EXPORT static const FailedDevice& invalid();

    /**
    * Test FailedDevice equality.
    * True if the devices are the same.
    * @since 3.0
    */
    LEAP_EXPORT bool operator==(const FailedDevice&) const;
    /**
    * Test FailedDevice inequality.
    * False if the devices are different.
    * @since 3.0
    */
    LEAP_EXPORT bool operator!=(const FailedDevice&) const;

  private:
    LEAP_EXPORT const char* pnpIdCString(size_t& length) const;
  };

  /**
   * The Image class represents a single image from one of the Leap Motion cameras.
   *
   * In addition to image data, the Image object provides a distortion map for correcting
   * lens distortion.
   *
   * \include Image_raw.txt
   *
   * Note that Image objects can be invalid, which means that they do not contain
   * valid image data. Get valid Image objects from Frame::frames(). Test for
   * validity with the Image::isValid() function.
   * @since 2.1.0
   */
  class Image : public Interface {
  public:

    // For internal use only.
    Image(ImageImplementation*);

    /**
     * Constructs a Image object.
     *
     * An uninitialized image is considered invalid.
     * Get valid Image objects from a ImageList object obtained from the
     * Frame::images() method.
     *
     *
     * @since 2.1.0
     */
    LEAP_EXPORT Image();

    /**
     * The image sequence ID.
     *
     * \include Image_sequenceId.txt
     *
     * @since 2.2.1
     */
    LEAP_EXPORT int64_t sequenceId() const;

    /**
     * The image ID.
     *
     * Images with ID of 0 are from the left camera; those with an ID of 1 are from the
     * right camera (with the device in its standard operating position with the
     * green LED facing the operator).
     *
     * @since 2.1.0
     */
    LEAP_EXPORT int32_t id() const;

    /**
     * The image data.
     *
     * The image data is a set of 8-bit intensity values. The buffer is
     * ``Image::width() * Image::height() * Image::bytesPerPixel()`` bytes long.
     *
     * \include Image_data_1.txt
     *
     * @return The array of unsigned char containing the sensor brightness values.
     * @since 2.1.0
     */
    LEAP_EXPORT const unsigned char* data() const;

    /**
     * The distortion calibration map for this image.
     *
     * The calibration map is a 64x64 grid of points. Each point is defined by
     * a pair of 32-bit floating point values. Each point in the map
     * represents a ray projected into the camera. The value of
     * a grid point defines the pixel in the image data containing the brightness
     * value produced by the light entering along the corresponding ray. By
     * interpolating between grid data points, you can find the brightness value
     * for any projected ray. Grid values that fall outside the range [0..1] do
     * not correspond to a value in the image data and those points should be ignored.
     *
     * \include Image_distortion_1.txt
     *
     * The calibration map can be used to render an undistorted image as well as to
     * find the true angle from the camera to a feature in the raw image. The
     * distortion map itself is designed to be used with GLSL shader programs.
     * In non-realtime contexts, it may be more convenient to use the Image::rectify()
     * and Image::warp() functions.
     *
     * If using shaders is not possible, you can use the distortion map directly.
     * This can be faster than using the ``warp()`` function, if carefully optimized:
     *
     * \include Image_distortion_using.txt
     *
     * Distortion is caused by the lens geometry as well as imperfections in the
     * lens and sensor window. The calibration map is created by the calibration
     * process run for each device at the factory (and which can be rerun by the
     * user).
     *
     * Note, in a future release, there may be two distortion maps per image;
     * one containing the horizontal values and the other containing the vertical values.
     *
     * @returns The float array containing the camera lens distortion map.
     * @since 2.1.0
     */
    LEAP_EXPORT const float* distortion() const;

    /*
     * Do not call this version of data(). It is intended only as a helper for
     * Java and Python SWIG bindings. Use the primary version of data() which
     * returns a pointer.
     *
     * @since 2.1.0
     */
    void data(unsigned char* dst) const {
      const unsigned char* src = data();
      memcpy(dst, src, width() * height() * bytesPerPixel() * sizeof(unsigned char));
    }

    /*
     * Do not call this version of distortion(). It is intended only as a helper for
     * Java and Python SWIG bindings. Use the primary version of distortion() which
     * returns a pointer.
     *
     * @since 2.1.0
     */
    void distortion(float* dst) const {
      const float* src = distortion();
      memcpy(dst, src, distortionWidth() * distortionHeight() * sizeof(float));
    }

    /**
     * The image width.
     *
     * \include Image_image_width_1.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT int width() const;

    /**
     * The image height.
     *
     * \include Image_image_height_1.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT int height() const;

    /**
     * The number of bytes per pixel.
     *
     * Use this value along with ``Image::width()`` and ``Image:::height()``
     * to calculate the size of the data buffer.
     *
     * \include Image_bytesPerPixel.txt
     *
     * @since 2.2.0
     */
    LEAP_EXPORT int bytesPerPixel() const;

    /**
     * Enumerates the possible image formats.
     *
     * The Image::format() function returns an item from the FormatType enumeration.
     * @since 2.2.0
     */
    enum FormatType {
      INFRARED = 0,
      IBRG = 1
    };

    /**
     * The image format.
     *
     * \include Image_format.txt
     *
     * @since 2.2.0
     */
    LEAP_EXPORT FormatType format() const;

    /**
     * The stride of the distortion map.
     *
     * Since each point on the 64x64 element distortion map has two values in the
     * buffer, the stride is 2 times the size of the grid. (Stride is currently fixed
     * at 2 * 64 = 128).
     *
     * \include Image_distortion_width_1.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT int distortionWidth() const;

    /**
     * The distortion map height.
     *
     * Currently fixed at 64.
     *
     * \include Image_distortion_height_1.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT int distortionHeight() const;

    /**
     * The horizontal ray offset.
     *
     * Used to convert between normalized coordinates in the range [0..1] and the
     * ray slope range [-4..4].
     *
     * \include Image_ray_factors_1.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT float rayOffsetX() const;

    /**
     * The vertical ray offset.
     *
     * Used to convert between normalized coordinates in the range [0..1] and the
     * ray slope range [-4..4].
     *
     * \include Image_ray_factors_2.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT float rayOffsetY() const;

    /**
     * The horizontal ray scale factor.
     *
     * Used to convert between normalized coordinates in the range [0..1] and the
     * ray slope range [-4..4].
     *
     * \include Image_ray_factors_1.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT float rayScaleX() const;

    /**
     * The vertical ray scale factor.
     *
     * Used to convert between normalized coordinates in the range [0..1] and the
     * ray slope range [-4..4].
     *
     * \include Image_ray_factors_2.txt
     *
     * @since 2.1.0
     */
    LEAP_EXPORT float rayScaleY() const;

    /**
     * Provides the corrected camera ray intercepting the specified point on the image.
     *
     * Given a point on the image, ``rectify()`` corrects for camera distortion
     * and returns the true direction from the camera to the source of that image point
     * within the Leap Motion field of view.
     *
     * This direction vector has an x and y component [x, y, 0], with the third element
     * always one. Note that this vector uses the 2D camera coordinate system
     * where the x-axis parallels the longer (typically horizontal) dimension and
     * the y-axis parallels the shorter (vertical) dimension. The camera coordinate
     * system does not correlate to the 3D Leap Motion coordinate system.
     *
     * \include Image_rectify_1.txt
     *
     * @param uv A Vector containing the position of a pixel in the image.
     * @returns A Vector containing the ray direction (the z-component of the vector is always 1).
     * @since 2.1.0
     */
    LEAP_EXPORT Vector rectify(const Vector& uv) const; // returns a vector (x, y, 1). The z-component is ignored

    /**
     * Provides the point in the image corresponding to a ray projecting
     * from the camera.
     *
     * Given a ray projected from the camera in the specified direction, ``warp()``
     * corrects for camera distortion and returns the corresponding pixel
     * coordinates in the image.
     *
     * The ray direction is specified in relationship to the camera. The first
     * vector element corresponds to the "horizontal" view angle; the second
     * corresponds to the "vertical" view angle.
     *
     * \include Image_warp_1.txt
     *
     * The ``warp()`` function returns pixel coordinates outside of the image bounds
     * if you project a ray toward a point for which there is no recorded data.
     *
     * ``warp()`` is typically not fast enough for realtime distortion correction.
     * For better performance, use a shader program executed on a GPU.
     *
     * @param xy A Vector containing the ray direction.
     * @returns A Vector containing the pixel coordinates [x, y, 1] (with z always one).
     * @since 2.1.0
     */
    LEAP_EXPORT Vector warp(const Vector& xy) const; // returns vector (u, v, 0). The z-component is ignored

    /**
     * Returns a timestamp indicating when this frame began being captured on the device.
     *
     * @since 2.2.7
     */
    LEAP_EXPORT int64_t timestamp() const;

    /**
     * Reports whether this Image instance contains valid data.
     *
     * @returns true, if and only if the image is valid.
     * @since 2.1.0
     */
    LEAP_EXPORT bool isValid() const;

    /**
     * Returns an invalid Image object.
     *
     * You can use the instance returned by this function in comparisons testing
     * whether a given Image instance is valid or invalid. (You can also use the
     * Image::isValid() function.)
     *
     * @returns The invalid Image instance.
     * @since 2.1.0
     */
    LEAP_EXPORT static const Image& invalid();

    /**
     * Compare Image object equality.
     *
     * Two Image objects are equal if and only if both Image objects represent the
     * exact same Image and both Images are valid.
     * @since 2.1.0
     */
    LEAP_EXPORT bool operator==(const Image&) const;

    /**
     * Compare Image object inequality.
     *
     *
     * Two Image objects are equal if and only if both Image objects represent the
     * exact same Image and both Images are valid.
     * @since 2.1.0
     */
    LEAP_EXPORT bool operator!=(const Image&) const;

    /**
     * Writes a brief, human readable description of the Image object.
     *
     * @since 2.1.0
     */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Image&);

    /**
     * A string containing a brief, human readable description of the Image object.
     *
     * @returns A description of the Image as a string.
     * @since 2.1.0
     */
    std::string toString() const {
      size_t length = 0;
      const char* str = toCString(length);
      return std::string(str, length);
    }

  private:
    LEAP_EXPORT const char* toCString(size_t& length) const;
  };

  // For internal use only.
  template<typename L, typename T>
  class ConstListIterator {
  public:
    ConstListIterator<L,T>() : m_list(0), m_index(-1) {}
    ConstListIterator<L,T>(const L& list, int index) : m_list(&list), m_index(index) {}

    const T operator*() const { return (*m_list)[m_index]; }
    const ConstListIterator<L,T> operator++(int) { ConstListIterator<L,T> ip(*this); ++m_index; return ip; }
    const ConstListIterator<L,T>& operator++() { ++m_index; return *this; }
    bool operator!=(const ConstListIterator<L,T>& rhs) const { return m_index != rhs.m_index; }
    bool operator==(const ConstListIterator<L,T>& rhs) const { return m_index == rhs.m_index; }

    typedef std::ptrdiff_t difference_type;
    typedef T value_type;
    typedef const T* pointer;
    typedef const T& reference;
    typedef std::forward_iterator_tag iterator_category;

  private:
    const L* m_list;
    int m_index;
  };

  /**
   * The FingerList class represents a list of Finger objects.
   *
   * Get a FingerList object by calling Frame::fingers().
   *
   * \include FingerList_FingerList.txt
   *
   * @since 1.0
   */
  class FingerList : public Interface {
  public:
    // For internal use only.
    FingerList(const std::shared_ptr< ListBaseImplementation<Finger> >&);

    /**
     * Constructs an empty list of fingers.
     * @since 1.0
     */
    LEAP_EXPORT FingerList();

    /**
     * Returns the number of fingers in this list.
     *
     * \include FingerList_count.txt
     *
     * @returns The number of fingers in this list.
     * @since 1.0
     */
    LEAP_EXPORT int count() const;

    /**
     * Reports whether the list is empty.
     *
     * \include FingerList_isEmpty.txt
     *
     * @returns True, if the list has no members.
     * @since 1.0
     */
    LEAP_EXPORT bool isEmpty() const;

    /**
     * Access a list member by its position in the list.
     *
     * \include FingerList_index.txt
     *
     * @param index The zero-based list position index.
     * @returns The Finger object at the specified index.
     * @since 1.0
     */
    LEAP_EXPORT Finger operator[](int index) const;

    /**
     * Appends the members of the specified FingerList to this FingerList.
     * @param other A FingerList object containing Finger objects
     * to append to the end of this FingerList.
     * @since 1.0
     */
    LEAP_EXPORT FingerList& append(const FingerList& other);

    /**
     * The member of the list that is farthest to the left within the standard
     * Leap Motion frame of reference (i.e has the smallest X coordinate).
     *
     * \include FingerList_leftmost.txt
     *
     * @returns The leftmost finger, or invalid if list is empty.
     * @since 1.0
     */
    LEAP_EXPORT Finger leftmost() const;

    /**
     * The member of the list that is farthest to the right within the standard
     * Leap Motion frame of reference (i.e has the largest X coordinate).
     *
     * \include FingerList_rightmost.txt
     *
     * @returns The rightmost finger, or invalid if list is empty.
     * @since 1.0
     */
    LEAP_EXPORT Finger rightmost() const;

    /**
     * The member of the list that is farthest to the front within the standard
     * Leap Motion frame of reference (i.e has the smallest Z coordinate).
     *
     * \include FingerList_frontmost.txt
     *
     * @returns The frontmost finger, or invalid if list is empty.
     * @since 1.0
     */
    LEAP_EXPORT Finger frontmost() const;

    /**
     * Returns a new list containing those fingers in the current list that are
     * extended.
     *
     * \include FingerList_extended.txt
     *
     * @returns The list of extended fingers from the current list.
     * @since 2.0
     */
    LEAP_EXPORT FingerList extended() const;

    /**
     * Returns a list containing fingers from the current list of a given finger type by
     * modifying the existing list.
     *
     * \include FingerList_fingerType.txt
     *
    * @returns The list of matching fingers from the current list.
     * @since 2.0
     */
    LEAP_EXPORT FingerList fingerType(Finger::Type type) const;

    /**
     * A C++ iterator type for FingerList objects.
     *
     * \include FingerList_iterator.txt
     *
     * @since 1.0
     */
    typedef ConstListIterator<FingerList, Finger> const_iterator;

    /**
     * The C++ iterator set to the beginning of this FingerList.
     *
     * \include FingerList_begin.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT const_iterator begin() const;

    /**
     * The C++ iterator set to the end of this FingerList.
     *
     * \include FingerList_end.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT const_iterator end() const;
  };

  /**
   * The HandList class represents a list of Hand objects.
   *
   * Get a HandList object by calling Frame::hands().
   *
   * \include HandList_HandList.txt
   *
   * @since 1.0
   */
  class HandList : public Interface {
  public:
    // For internal use only.
    HandList(const std::shared_ptr< ListBaseImplementation<Hand> >&);

    /**
     * Constructs an empty list of hands.
     * @since 1.0
     */
    LEAP_EXPORT HandList();

    /**
     * Returns the number of hands in this list.
     *
     * \include HandList_count.txt
     * @returns The number of hands in this list.
     * @since 1.0
     */
    LEAP_EXPORT int count() const;

    /**
     * Reports whether the list is empty.
     *
     * \include HandList_isEmpty.txt
     *
     * @returns True, if the list has no members.
     * @since 1.0
     */
    LEAP_EXPORT bool isEmpty() const;

    /**
     * Access a list member by its position in the list.
     *
     * \include HandList_operator_index.txt
     *
     * @param index The zero-based list position index.
     * @returns The Hand object at the specified index.
     * @since 1.0
     */
    LEAP_EXPORT Hand operator[](int index) const;

    /**
     * Appends the members of the specified HandList to this HandList.
     * @param other A HandList object containing Hand objects
     * to append to the end of this HandList.
     */
    LEAP_EXPORT HandList& append(const HandList& other);

    /**
     * The member of the list that is farthest to the left within the standard
     * Leap Motion frame of reference (i.e has the smallest X coordinate).
     *
     * Note: to determine whether a hand is the left hand, use the Hand::isLeft() function.
     *
     * \include HandList_leftmost.txt
     *
     * @returns The leftmost hand, or invalid if list is empty.
     * @since 1.0
     */
    LEAP_EXPORT Hand leftmost() const;

    /**
     * The member of the list that is farthest to the right within the standard
     * Leap Motion frame of reference (i.e has the largest X coordinate).
     *
     * Note: to determine whether a hand is the right hand, use the Hand::isRight() function.
     *
     * \include HandList_rightmost.txt
     *
     * @returns The rightmost hand, or invalid if list is empty.
     * @since 1.0
     */
    LEAP_EXPORT Hand rightmost() const;

    /**
     * The member of the list that is farthest to the front within the standard
     * Leap Motion frame of reference (i.e has the smallest Z coordinate).
     *
     * \include HandList_frontmost.txt
     *
     * @returns The frontmost hand, or invalid if list is empty.
     * @since 1.0
     */
    LEAP_EXPORT Hand frontmost() const;

    /**
     * A C++ iterator type for this HandList objects.
     *
     * \include HandList_iterator.txt
     *
     * @since 1.0
     */
    typedef ConstListIterator<HandList, Hand> const_iterator;

    /**
     * The C++ iterator set to the beginning of this HandList.
     *
     * \include HandList_begin.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT const_iterator begin() const;

    /**
     * The C++ iterator set to the end of this HandList.
     *
     * \include HandList_end.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT const_iterator end() const;
  };

  /**
   * The DeviceList class represents a list of Device objects.
   *
   * Get a DeviceList object by calling Controller::devices().
   * @since 1.0
   */
  class DeviceList : public Interface {
  public:
    // For internal use only.
    DeviceList(const std::shared_ptr< ListBaseImplementation<Device> >&);

    /**
     * Constructs an empty list of devices.
     * @since 1.0
     */
    LEAP_EXPORT DeviceList();

    /**
     * Returns the number of devices in this list.
     * @returns The number of devices in this list.
     * @since 1.0
     */
    LEAP_EXPORT int count() const;

    /**
     * Reports whether the list is empty.
     *
     * \include DeviceList_isEmpty.txt
     *
     * @returns True, if the list has no members.
     * @since 1.0
     */
    LEAP_EXPORT bool isEmpty() const;

    /**
     * Access a list member by its position in the list.
     * @param index The zero-based list position index.
     * @returns The Device object at the specified index.
     * @since 1.0
     */
    LEAP_EXPORT Device operator[](int index) const;

    /**
     * Appends the members of the specified DeviceList to this DeviceList.
     * @param other A DeviceList object containing Device objects
     * to append to the end of this DeviceList.
     * @since 1.0
     */
    LEAP_EXPORT DeviceList& append(const DeviceList& other);

    /**
     * A C++ iterator type for the DeviceList class.
     * @since 1.0
     */
    typedef ConstListIterator<DeviceList, Device> const_iterator;

    /**
     * The C++ iterator set to the beginning of this DeviceList.
     * @since 1.0
     */
    LEAP_EXPORT const_iterator begin() const;

    /**
     * The C++ iterator set to the end of this DeviceList.
     * @since 1.0
     */
    LEAP_EXPORT const_iterator end() const;
  };

  /**
  * The list of FailedDevice objects contains an entry for every failed Leap Motion
  * hardware device connected to the client computer. FailedDevice objects report
  * the device pnpID string and reason for failure.
  *
  * Get the list of FailedDevice objects from Controller::failedDevices().
  *
  * @since 3.0
  */
  class FailedDeviceList : public Interface {
  public:
    // For internal use only.
    FailedDeviceList(const std::shared_ptr< ListBaseImplementation<FailedDevice> >&);

    /**
    * Constructs an empty list.
    * @since 3.0
    */
    LEAP_EXPORT FailedDeviceList();

    /**
    * The number of members in the list.
    * @since 3.0
    */
    LEAP_EXPORT int count() const;

    /**
    * Reports whether the list is empty.
    * @since 3.0
    */
    LEAP_EXPORT bool isEmpty() const;

    /**
    * Array-style access to list members.
    * @since 3.0
    */
    LEAP_EXPORT FailedDevice operator[](int index) const;

    /**
    * Appends the contents of another FailedDeviceList to this one.
    * @since 3.0
    */
    LEAP_EXPORT FailedDeviceList& append(const FailedDeviceList& other);

    /**
    * The FailedDeviceList iterator type.
    * @since 3.0
    */
    typedef ConstListIterator<FailedDeviceList, FailedDevice> const_iterator;

    /**
    * The list iterator pointing to the beginning of the list.
    * @since 3.0
    */
    LEAP_EXPORT const_iterator begin() const;

    /**
    * The list iterator pointing to the end of the list.
    * @since 3.0
    */
    LEAP_EXPORT const_iterator end() const;
  };

  /**
   * The ImageList class represents a list of Image objects.
   *
   * Get the ImageList object associated with the a Frame of tracking data
   * by calling Frame::images(). Get the most recent set of images, which can be
   * newer than the images used to create the current frame, by calling
   * Controller::images().
   *
   * @since 2.1.0
   */
  class ImageList : public Interface {
  public:
    // For internal use only.
    ImageList(const std::shared_ptr< ListBaseImplementation<Image> >&);

    /**
     * Constructs an empty list of images.
     * @since 2.1.0
     */
    LEAP_EXPORT ImageList();

    /**
     * The number of images in this list.
     *
     * @returns The number of images in this list.
     * @since 2.1.0
     */
    LEAP_EXPORT int count() const;

    /**
     * Reports whether the list is empty.
     *
     * \include ImageList_isEmpty.txt
     *
     * @returns True, if the list has no members.
     * @since 2.1.0
     */
    LEAP_EXPORT bool isEmpty() const;

    /**
     * Access a list member by its position in the list.
     * @param index The zero-based list position index.
     * @returns The Image object at the specified index.
     * @since 2.1.0
     */
    LEAP_EXPORT Image operator[](int index) const;

    /**
     * Appends the members of the specified ImageList to this ImageList.
     * @param other A ImageList object containing Image objects
     * to append to the end of this ImageList.
     * @since 2.1.0
     */
    LEAP_EXPORT ImageList& append(const ImageList& other);

    /**
     * A C++ iterator type for ImageList objects.
     * @since 2.1.0
     */
    typedef ConstListIterator<ImageList, Image> const_iterator;

    /**
     * The C++ iterator set to the beginning of this ImageList.
     * @since 2.1.0
     */
    LEAP_EXPORT const_iterator begin() const;

    /**
     * The C++ iterator set to the end of this ImageList.
     * @since 2.1.0
     */
    LEAP_EXPORT const_iterator end() const;
  };

  class MapPointList : public Interface {
  public:
    // For internal use only.
    MapPointList(const std::shared_ptr< ListBaseImplementation<MapPoint> >&);

    /**
     * Constructs an empty list of images.
     * @since 4.0
     */
    LEAP_EXPORT MapPointList();

    /**
     * The number of images in this list.
     *
     * @returns The number of images in this list.
     * @since 4.0
     */
    LEAP_EXPORT int count() const;

    /**
     * Reports whether the list is empty.
     *
     * \include MapPointList_isEmpty.txt
     *
     * @returns True, if the list has no members.
     * @since 4.0
     */
    LEAP_EXPORT bool isEmpty() const;

    /**
     * Access a list member by its position in the list.
     * @param index The zero-based list position index.
     * @returns The MapPoint object at the specified index.
     * @since 4.0
     */
    LEAP_EXPORT MapPoint operator[](int index) const;

    /**
     * Appends the members of the specified MapPointList to this MapPointList.
     * @param other A MapPointList object containing MapPoint objects
     * to append to the end of this MapPointList.
     * @since 4.0
     */
    LEAP_EXPORT MapPointList& append(const MapPointList& other);

    /**
     * A C++ iterator type for MapPointList objects.
     * @since 4.0
     */
    typedef ConstListIterator<MapPointList, MapPoint> const_iterator;

    /**
     * The C++ iterator set to the beginning of this MapPointList.
     * @since 4.0
     */
    LEAP_EXPORT const_iterator begin() const;

    /**
     * The C++ iterator set to the end of this MapPointList.
     * @since 4.0
     */
    LEAP_EXPORT const_iterator end() const;
  };

  class HeadPose: public Interface {
  public:
    // For internal use only.
    HeadPose(HeadPoseImplementation*);

    LEAP_EXPORT HeadPose();

    LEAP_EXPORT int64_t timestamp() const;

    LEAP_EXPORT Vector position() const;

    LEAP_EXPORT Quaternion orientation() const;
  };

  /**
   * The Frame class represents a set of hand and finger tracking data detected
   * in a single frame.
   *
   * The Leap Motion software detects hands, fingers and tools within the tracking area, reporting
   * their positions and orientations in frames at the Leap Motion frame rate.
   *
   * Access Frame objects through an instance of the Controller class:
   *
   * \include Controller_Frame_1.txt
   *
   * Implement a Listener subclass to receive a callback event when a new Frame is available.
   * @since 1.0
   */
  class Frame : public Interface {
  public:
    // For internal use only.
    Frame(FrameImplementation*);

    /**
     * Constructs a Frame object.
     *
     * Frame instances created with this constructor are invalid.
     * Get valid Frame objects by calling the Controller::frame() function.
     *
     * \include Frame_Frame.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT Frame();

    /**
     * A unique ID for this Frame.
     *
     * Consecutive frames processed by the Leap Motion software have consecutive
     * increasing values. You can use the frame ID to avoid processing the same
     * Frame object twice:
     *
     * \include Frame_Duplicate.txt
     *
     * As well as to make sure that your application processes every frame:
     *
     * \include Frame_Skipped.txt
     *
     * @returns The frame ID.
     * @since 1.0
     */
    LEAP_EXPORT int64_t id() const;

    /**
     * The frame capture time in microseconds elapsed since an arbitrary point in
     * time in the past.
     *
     * Use Controller::now() to calculate the age of the frame.
     *
     * \include Frame_timestamp.txt
     *
     * @returns The timestamp in microseconds.
     * @since 1.0
     */
    LEAP_EXPORT int64_t timestamp() const;

    /**
     * The list of Hand objects detected in this frame, given in arbitrary order.
     * The list can be empty if no hands are detected.
     *
     * \include Frame_hands.txt
     *
     * @returns The HandList containing all Hand objects detected in this frame.
     * @since 1.0
     */
    LEAP_EXPORT HandList hands() const;

    /**
     * The Hand object with the specified ID in this frame.
     *
     * Use the Frame::hand() function to retrieve the Hand object from
     * this frame using an ID value obtained from a previous frame.
     * This function always returns a Hand object, but if no hand
     * with the specified ID is present, an invalid Hand object is returned.
     *
     * \include Frame_hand.txt
     *
     * Note that ID values persist across frames, but only until tracking of a
     * particular object is lost. If tracking of a hand is lost and subsequently
     * regained, the new Hand object representing that physical hand may have
     * a different ID than that representing the physical hand in an earlier frame.
     *
     * @param id The ID value of a Hand object from a previous frame.
     * @returns The Hand object with the matching ID if one exists in this frame;
     * otherwise, an invalid Hand object is returned.
     * @since 1.0
     */
    LEAP_EXPORT Hand hand(int32_t id) const;

    /**
     * The list of Finger objects detected in this frame, given in arbitrary order.
     * The list can be empty if no fingers are detected.
     *
     * Use FingerList::extended() to remove non-extended fingers from the list.
     *
     * \include Frame_fingers.txt
     *
     * @returns The FingerList containing all Finger objects detected in this frame.
     * @since 1.0
     */
    LEAP_EXPORT FingerList fingers() const;

    /**
     * The Finger object with the specified ID in this frame.
     *
     * Use the Frame::finger() function to retrieve the Finger object from
     * this frame using an ID value obtained from a previous frame.
     * This function always returns a Finger object, but if no finger
     * with the specified ID is present, an invalid Finger object is returned.
     *
     * \include Frame_finger.txt
     *
     * Note that ID values persist across frames, but only until tracking of a
     * particular object is lost. If tracking of a finger is lost and subsequently
     * regained, the new Finger object representing that physical finger may have
     * a different ID than that representing the finger in an earlier frame.
     *
     * @param id The ID value of a Finger object from a previous frame.
     * @returns The Finger object with the matching ID if one exists in this frame;
     * otherwise, an invalid Finger object is returned.
     * @since 1.0
     */
    LEAP_EXPORT Finger finger(int32_t id) const;

    /**
     * The list of IR images from the Leap Motion cameras.
     * If the system is in Robust mode, these will be slightly post processed
     *
     * @return An ImageList object containing the camera images analyzed to create this Frame.
     * @deprecated 3.0
     */
    LEAP_EXPORT ImageList images() const;

    /**
     * The list of Raw images from the Leap Motion cameras.
     * These will never be postprocessed and closely match the raw sensor output.
     *
     * @return An ImageList object containing the camera images analyzed to create this Frame.
     * @since 2.3
     */
    LEAP_EXPORT ImageList rawImages() const;

    /**
     * The list of 3D map points being tracked in the frame.
     *
     * @return A MapPointList object containing the currently tracked 3D map points.
     * @since 4.0
     */
     LEAP_EXPORT MapPointList mapPoints() const;

    /**
     * The instantaneous frame rate.
     *
     * The rate at which the Leap Motion software is providing frames of data
     * (in frames per second). The frame rate can fluctuate depending on available computing
     * resources, activity within the device field of view, software tracking settings,
     * and other factors.
     *
     * \include Frame_currentFramesPerSecond.txt
     *
     * @returns An estimate of frames per second of the Leap Motion Controller.
     * @since 1.0
     */
    LEAP_EXPORT float currentFramesPerSecond() const;

    /**
     * Reports whether this Frame instance is valid.
     *
     * A valid Frame is one generated by the Leap::Controller object that contains
     * tracking data for all detected entities. An invalid Frame contains no
     * actual tracking data, but you can call its functions without risk of a
     * null pointer exception. The invalid Frame mechanism makes it more
     * convenient to track individual data across the frame history. For example,
     * you can invoke:
     *
     * \include Frame_Valid_Chain.txt
     *
     * for an arbitrary Frame history value, "n", without first checking whether
     * frame(n) returned a null object. (You should still check that the
     * returned Finger instance is valid.)
     *
     * @returns True, if this is a valid Frame object; false otherwise.
     * @since 1.0
     */
    LEAP_EXPORT bool isValid() const;

    /**
     * Returns an invalid Frame object.
     *
     * You can use the instance returned by this function in comparisons testing
     * whether a given Frame instance is valid or invalid. (You can also use the
     * Frame::isValid() function.)
     *
     * \include Frame_Invalid_Demo.txt
     *
     * @returns The invalid Frame instance.
     * @since 1.0
     */
    LEAP_EXPORT static const Frame& invalid();

    /**
     * Compare Frame object equality.
     *
     * \include Frame_operator_equals.txt
     *
     * Two Frame objects are equal if and only if both Frame objects represent
     * the exact same frame of tracking data and both Frame objects are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator==(const Frame&) const;

    /**
     * Compare Frame object inequality.
     *
     * \include Frame_operator_not_equals.txt
     *
     * Two Frame objects are equal if and only if both Frame objects represent
     * the exact same frame of tracking data and both Frame objects are valid.
     * @since 1.0
     */
    LEAP_EXPORT bool operator!=(const Frame&) const;

    /**
     * Writes a brief, human readable description of the Frame object to an output stream.
     *
     * \include Frame_operator_stream.txt
     *
     * @since 1.0
     */
    LEAP_EXPORT friend std::ostream& operator<<(std::ostream&, const Frame&);

    /**
     * A string containing a brief, human readable description of the Frame object.
     *
     * @returns A description of the Frame as a string.
     * @since 1.0
     */
    std::string toString() const {
      size_t length = 0;
      const char* str = toCString(length);
      return std::string(str, length);
    }

  private:
    LEAP_EXPORT const char* toCString(size_t& length) const;
  };

  /**
   * The Config class provides access to Leap Motion system configuration information.
   *
   * After setting a configuration value, you must call the Config::save() method
   * to commit the changes. You can save after the Controller has connected to
   * the Leap Motion service/daemon. In other words, after the Controller
   * has dispatched the serviceConnected or connected events or
   * Controller::isConnected is true. The configuration value changes are
   * not persistent; your application needs to set the values every time it runs.
   *
   * @since 4.0
   */
  class Config : public Interface {
  public:
    /**
     * Constructs a Config object.
     * Do not create your own Config objects. Get a Config object using
     * the Controller::config() function.
     *
     * \include Config_Constructor.txt
     *
     * @since 4.0
     */
    Config(ControllerImplementation* c);

    enum ValueType : int32_t {
      TYPE_UNKNOWN,
      TYPE_BOOLEAN,
      TYPE_INT32,
      TYPE_FLOAT,
      TYPE_STRING
    };
    struct Value {
      Value() { memset(this, 0, sizeof(Value)); }
      Value(const Value& v) { type = TYPE_UNKNOWN; *this = v; }
      Value& operator=(const Value& v) {
        clear();
        type = v.type;
        switch (type) {
        case TYPE_BOOLEAN:
          bValue = v.bValue;
          break;
        case TYPE_INT32:
          iValue = v.iValue;
          break;
        case TYPE_FLOAT:
          fValue = v.fValue;
          break;
        case TYPE_STRING:
          strValue = v.strValue ? strdup(v.strValue) : nullptr;
        default:
          break;
        }
        return *this;
      }
      ~Value() { clear(); }
      void clear() {
        if (type == TYPE_STRING && strValue) {
          free(strValue);
        }
        memset(this, 0, sizeof(Value));
      }

      ValueType type;

      union {
        bool bValue;
        int32_t iValue;
        float fValue;
        char* strValue;
      };

    };
    /**
     * Reports the value as a union.
     *
     * This is a blocking wrapper around LeapC, so there was not really a
     * good way to emulate the old api's behavior without requesting and
     * caching every possible config value. Instead, we have opted to provide
     * an optional timeout argument. The default should be sufficient for
     * round trip time on modern systems.
     *
     * @since 4.0
     */
    Value LEAP_EXPORT value(const char* key, uint32_t timeoutMilliseconds = 20) const;

    /**
     * Set the value as a union.
     *
     * @since 4.0
     */
    bool LEAP_EXPORT setValue(const char* key, const Value& value);

    Value value(const std::string& key, uint32_t timeoutMilliseconds = 20) const {
      return value(key.c_str(), timeoutMilliseconds);
    }
    bool setValue(const std::string& key, const Value& value) {
      return setValue(key.c_str(), value);
    }
    bool setValue(const std::string& key, bool value) {
      Value v;
      v.type = ValueType::TYPE_BOOLEAN;
      v.bValue = value;
      return setValue(key, v);
    }
    bool setValue(const std::string& key, int32_t value) {
      Value v;
      v.type = ValueType::TYPE_INT32;
      v.iValue = value;
      return setValue(key, v);
    }
    bool setValue(const std::string& key, float value) {
      Value v;
      v.type = ValueType::TYPE_FLOAT;
      v.fValue = value;
      return setValue(key, v);
    }
    bool setValue(const std::string& key, const char* value) {
      Value v;
      v.type = ValueType::TYPE_STRING;
      v.strValue = value ? strdup(value) : nullptr;
      return setValue(key, v);
    }
    bool setValue(const std::string& key, const std::string& value) {
      Value v;
      v.type = ValueType::TYPE_STRING;
      v.strValue = !value.empty() ? strdup(value.c_str()) : nullptr;
      return setValue(key, v);
    }
  };

  /**
   * The Controller class is your main interface to the Leap Motion Controller.
   *
   * Create an instance of this Controller class to access frames of tracking
   * data and configuration information. Frame data can be polled at any time
   * using the Controller::frame() function. Call frame() or frame(0) to get the
   * most recent frame. Set the history parameter to a positive integer to access
   * previous frames. A controller stores up to 60 frames in its frame history.
   *
   * Polling is an appropriate strategy for applications which already have an
   * intrinsic update loop, such as a game. You can also add an instance of a
   * subclass of Leap::Listener to the controller to handle events as they occur.
   * The Controller dispatches events to the listener upon initialization and exiting,
   * on connection changes, when the application gains and loses the OS input focus,
   * and when a new frame of tracking data is available.
   * When these events occur, the controller object invokes the appropriate
   * callback function defined in your subclass of Listener.
   *
   * To access frames of tracking data as they become available:
   *
   * 1. Implement a subclass of the Listener class and override the
   *    Listener::onFrame() function.
   * 2. In your Listener::onFrame() function, call the Controller::frame()
   *    function to access the newest frame of tracking data.
   * 3. To start receiving frames, create a Controller object and add an instance
   *    of the Listener subclass to the Controller::addListener() function.
   *
   * When an instance of a Listener subclass is added to a Controller object,
   * it calls the Listener::onInit() function when the listener is ready for use.
   * When a connection is established between the controller and the Leap Motion software,
   * the controller calls the Listener::onConnect() function. At this point, your
   * application will start receiving frames of data. The controller calls the
   * Listener::onFrame() function each time a new frame is available. If the
   * controller loses its connection with the Leap Motion software or device for any
   * reason, it calls the Listener::onDisconnect() function. If the listener is
   * removed from the controller or the controller is destroyed, it calls the
   * Listener::onExit() function. At that point, unless the listener is added to
   * another controller again, it will no longer receive frames of tracking data.
   *
   * The Controller object is multithreaded and calls the Listener functions on
   * its own thread, not on an application thread.
   * @since 1.0
   */
  class LEAP_EXPORT_CLASS Controller : public Interface {
  public:
    // For internal use only.
    Controller(ControllerImplementation*);

    /**
     * Constructs a Controller object.
     *
     * When creating a Controller object, you may optionally pass in a
     * reference to an instance of a subclass of Leap::Listener. Alternatively,
     * you may add a listener using the Controller::addListener() function. *
     * @since 1.0
     */
    LEAP_EXPORT Controller(const char* server_namespace = nullptr);
    LEAP_EXPORT virtual ~Controller();
    /**
     * Constructs a Controller object.
     *
     * When creating a Controller object, you may optionally pass in a
     * reference to an instance of a subclass of Leap::Listener. Alternatively,
     * you may add a listener using the Controller::addListener() function.
     *
     * \include Controller_Controller.txt
     *
     * @param listener An instance of Leap::Listener implementing the callback
     * functions for the Leap Motion events you want to handle in your application.
     * @since 1.0
     */
    LEAP_EXPORT Controller(Listener& listener, const char* server_namespace = nullptr);

    /**
     * Reports whether this Controller is connected to the Leap Motion service and
     * the Leap Motion hardware is plugged in.
     *
     * When you first create a Controller object, isConnected() returns false.
     * After the controller finishes initializing and connects to the Leap Motion
     * software and if the Leap Motion hardware is plugged in, isConnected() returns true.
     *
     * You can either handle the onConnect event using a Listener instance or
     * poll the isConnected() function if you need to wait for your
     * application to be connected to the Leap Motion software before performing some other
     * operation.
     *
     * \include Controller_isConnected.txt
     * @returns True, if connected; false otherwise.
     * @since 1.0
     */
    LEAP_EXPORT bool isConnected() const;

    /**
     * Reports whether your application has a connection to the Leap Motion
     * daemon/service. Can be true even if the Leap Motion hardware is not available.
     * @since 1.2
     */
    LEAP_EXPORT bool isServiceConnected() const;

    /**
     * The supported controller policies.
     *
     * The supported policy flags are:
     *
     * **POLICY_IMAGES** -- request that your application receives images from the
     *   device cameras. The "Allow Images" checkbox must be enabled in the
     *   Leap Motion Control Panel or this policy will be denied.
     *
     *   The images policy determines whether an application receives image data from
     *   the Leap Motion sensors with each frame of data. By default, this data is
     *   not sent. Only applications that use the image data should request this policy.
     *
     *
     * **POLICY_OPTIMIZE_HMD** -- request that the tracking be optimized for head-mounted
     *   tracking.
     *
     *   The optimize HMD policy improves tracking in situations where the Leap
     *   Motion hardware is attached to a head-mounted display. This policy is
     *   not granted for devices that cannot be mounted to an HMD, such as
     *   Leap Motion controllers embedded in a laptop or keyboard.
     *
     * **POLICY_ALLOW_PAUSE_RESUME** -- request that the application be allowed
     *   to pause and unpause the Leap Motion service.
     *
     * Some policies can be denied if the user has disabled the feature on
     * their Leap Motion control panel.
     *
     * @since 1.0
     */
    enum PolicyFlag {
      /**
       * The default policy.
       * @since 1.0
       */
      POLICY_DEFAULT = 0,

      /**
       * Receive images from sensor cameras.
       * @since 2.1.0
       */
      POLICY_IMAGES = (1 << 1),

      /**
       * Optimize the tracking for head-mounted device.
       * @since 2.1.2
       */
      POLICY_OPTIMIZE_HMD = (1 << 2),

      /**
      * Allow pausing and unpausing of the Leap Motion service.
      * @since 3.0
      */
      POLICY_ALLOW_PAUSE_RESUME = (1 << 3),

      /**
       * Receive raw images.
       */
      POLICY_RAW_IMAGES = (1 << 6),

      /**
       * Receive map points.
       */
      POLICY_MAP_POINTS = (1 << 7),
    };

    /**
     * This function has been deprecated. Use isPolicySet() instead.
     * @deprecated 2.1.6
     */
    LEAP_EXPORT PolicyFlag policyFlags() const;

    /**
     * This function has been deprecated. Use setPolicy() and clearPolicy() instead.
     * @deprecated 2.1.6
     */
    LEAP_EXPORT void setPolicyFlags(PolicyFlag flags) const;

    /**
     * Requests setting a policy.
     *
     * A request to change a policy is subject to user approval and a policy
     * can be changed by the user at any time (using the Leap Motion settings dialog).
     * The desired policy flags must be set every time an application runs.
     *
     * Policy changes are completed asynchronously and, because they are subject
     * to user approval or system compatibility checks, may not complete successfully. Call
     * Controller::isPolicySet() after a suitable interval to test whether
     * the change was accepted.
     *
     * \include Controller_setPolicy.txt
     *
     * @param policy A PolicyFlag value indicating the policy to request.
     * @since 2.1.6
     */
    LEAP_EXPORT void setPolicy(PolicyFlag policy) const;

    /**
     * Requests clearing a policy.
     *
     * Policy changes are completed asynchronously and, because they are subject
     * to user approval or system compatibility checks, may not complete successfully. Call
     * Controller::isPolicySet() after a suitable interval to test whether
     * the change was accepted.
     *
     * \include Controller_clearPolicy.txt
     *
     * @param flags A PolicyFlag value indicating the policy to request.
     * @since 2.1.6
     */
    LEAP_EXPORT void clearPolicy(PolicyFlag policy) const;

    /**
     * Gets the active setting for a specific policy.
     *
     * Keep in mind that setting a policy flag is asynchronous, so changes are
     * not effective immediately after calling setPolicyFlag(). In addition, a
     * policy request can be declined by the user. You should always set the
     * policy flags required by your application at startup and check that the
     * policy change request was successful after an appropriate interval.
     *
     * If the controller object is not connected to the Leap Motion software, then the default
     * state for the selected policy is returned.
     *
     * \include Controller_isPolicySet.txt
     *
     * @param flags A PolicyFlag value indicating the policy to query.
     * @returns A boolean indicating whether the specified policy has been set.
     * @since 2.1.6
     */
    LEAP_EXPORT bool isPolicySet(PolicyFlag policy) const;

    /**
     * Adds a listener to this Controller.
     *
     * The Controller dispatches Leap Motion events to each associated listener. The
     * order in which listener callback functions are invoked is arbitrary. If
     * you pass a listener to the Controller's constructor function, it is
     * automatically added to the list and can be removed with the
     * Controller::removeListener() function.
     *
     * \include Controller_addListener.txt
     *
     * The Controller does not keep a strong reference to the Listener instance.
     * Ensure that you maintain a reference until the listener is removed from
     * the controller.
     *
     * @param listener A subclass of Leap::Listener implementing the callback
     * functions for the Leap Motion events you want to handle in your application.
     * @returns Whether or not the listener was successfully added to the list
     * of listeners.
     * @since 1.0
     */
    LEAP_EXPORT bool addListener(Listener& listener);

    /**
     * Remove a listener from the list of listeners that will receive Leap Motion
     * events. A listener must be removed if its lifetime is shorter than the
     * controller to which it is listening.
     *
     * \include Controller_removeListener.txt
     *
     * @param listener The listener to remove.
     * @returns Whether or not the listener was successfully removed from the
     * list of listeners.
     * @since 1.0
     */
    LEAP_EXPORT bool removeListener(Listener& listener);

    /**
     * Returns a frame of tracking data from the Leap Motion software. Use the optional
     * history parameter to specify which frame to retrieve. Call frame() or
     * frame(0) to access the most recent frame; call frame(1) to access the
     * previous frame, and so on. If you use a history value greater than the
     * number of stored frames, then the controller returns an invalid frame.
     *
     * \include Controller_Frame_1.txt
     *
     * You can call this function in your Listener implementation to get frames at the
     * Leap Motion frame rate:
     *
     * \include Controller_Listener_onFrame.txt

     * @param history The age of the frame to return, counting backwards from
     * the most recent frame (0) into the past and up to the maximum age (59).
     * @returns The specified frame; or, if no history parameter is specified,
     * the newest frame. If a frame is not available at the specified history
     * position, an invalid Frame is returned.
     * @since 1.0
     */
    LEAP_EXPORT Frame frame(int history = 0) const;

    LEAP_EXPORT HeadPose headPose(int64_t timestamp) const;

    /**
     * The most recent set of images from the Leap Motion cameras.
     *
     * \include Controller_images.txt
     *
     * Depending on timing and the current processing frame rate, the images
     * obtained with this function can be newer than images obtained from
     * the current frame of tracking data.
     *
     * @return An ImageList object containing the most recent camera images.
     * @since 2.2.1
     */
    LEAP_EXPORT ImageList images() const;

    LEAP_EXPORT ImageList rawImages() const;

    /**
     * A structure used to query and modify the leap service settings
     *
     * @since 4.0
     */

    LEAP_EXPORT Config config() const;

    /**
     * The list of currently attached and recognized Leap Motion controller devices.
     *
     * The Device objects in the list describe information such as the range and
     * tracking volume.
     *
     * \include Controller_devices.txt
     *
     * Currently, the Leap Motion Controller only allows a single active device at a time,
     * however there may be multiple devices physically attached and listed here.  Any active
     * device(s) are guaranteed to be listed first, however order is not determined beyond that.
     *
     * @returns The list of Leap Motion controllers.
     * @since 1.0
     */
    LEAP_EXPORT DeviceList devices() const;

    /**
    * A list of any Leap Motion hardware devices that are physically connected to
    * the client computer, but are not functioning correctly. The list contains
    * FailedDevice objects containing the pnpID and the reason for failure. No
    * other device information is available.
    *
    * \include Controller_failedDevices.txt
    *
    * @since 3.0
    */
    LEAP_EXPORT FailedDeviceList failedDevices() const;

     /**
     * Pauses or resumes the Leap Motion service.
     *
     * When the service is paused no applications receive tracking data and the
     * service itself uses minimal CPU time.
     *
     * Before changing the state of the service, you must set the
     * POLICY_ALLOW_PAUSE_RESUME using the Controller::setPolicy() function.
     * Policies must be set every time the application is run.
     *
     * \include Controller_setPaused.txt
     *
     * @param pause Set true to pause the service; false to resume.
     * @since 3.0
     */
    LEAP_EXPORT void setPaused(bool pause);
     /**
     * Reports whether the Leap Motion service is currently paused.
     *
     * \include Controller_isPaused.txt
     *
     * @returns True, if the service is paused; false, otherwise.
     * @since 3.0
     */
    LEAP_EXPORT bool isPaused() const;

    /**
     * Returns a timestamp value as close as possible to the current time.
     * Values are in microseconds, as with all the other timestamp values.
     *
     * @since 2.2.7
     **/
    LEAP_EXPORT int64_t now() const;
  };

  /**
   * Reports whether the message is for
   * a severe failure, a recoverable warning, or a status change.
   * @since 3.0
   */
  enum MessageSeverity {
    MESSAGE_UNKNOWN = 0,        /**< Unknown severity, indicates an error. The rest of the data may be invalid. */
    MESSAGE_CRITICAL = 1,       /**< A problem severe enough to stop tracking */
    MESSAGE_WARNING = 2,        /**< A correctable issue or status that can impact tracking */
    MESSAGE_INFORMATION = 3     /**< A verbose, informational message */
  };

  /**
   * The Listener class defines a set of callback functions that you can
   * override in a subclass to respond to events dispatched by the Controller object.
   *
   * To handle Leap Motion events, create an instance of a Listener subclass and assign
   * it to the Controller instance. The Controller calls the relevant Listener
   * callback function when an event occurs, passing in a reference to itself.
   * You do not have to implement callbacks for events you do not want to handle.
   *
   * The Controller object calls these Listener functions from a thread created
   * by the Leap Motion library, not the thread used to create or set the Listener instance.
   * @since 1.0
   */
  class LEAP_EXPORT_CLASS Listener {
  public:
    /**
     * Constructs a Listener object.
     * @since 1.0
     */
    LEAP_EXPORT Listener() {}

    /**
     * Destroys this Listener object.
     */
    LEAP_EXPORT virtual ~Listener() {}

    /**
     * Called once, when this Listener object is newly added to a Controller.
     *
     * \include Listener_onInit.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onInit(const Controller&) {}

    /**
     * Called when the Controller object connects to the Leap Motion software and
     * the Leap Motion hardware device is plugged in,
     * or when this Listener object is added to a Controller that is already connected.
     *
     * When this callback is invoked, Controller::isServiceConnected is true,
     * Controller::devices() is not empty.
     *
     * \include Listener_onConnect.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onConnect(const Controller&) {}

    /**
     * Called when the Controller object disconnects from the Leap Motion software or
     * the Leap Motion hardware is unplugged.
     * The controller can disconnect when the Leap Motion device is unplugged, the
     * user shuts the Leap Motion software down, the user pauses the service,
     * or the Leap Motion software encounters an unrecoverable error.
     *
     * \include Listener_onDisconnect.txt
     *
     * Note: When you launch a Leap-enabled application in a debugger, the
     * Leap Motion library does not disconnect from the application. This is to allow
     * you to step through code without losing the connection because of time outs.
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onDisconnect(const Controller&) {}

    /**
     * Called when this Listener object is removed from the Controller
     * or the Controller instance is destroyed.
     *
     * \include Listener_onExit.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onExit(const Controller&) {}

    /**
     * Called when a new frame of hand and finger tracking data is available.
     * Access the new frame data using the Controller::frame() function.
     *
     * \include Listener_onFrame.txt
     *
     * Note, the Controller skips any pending onFrame events while your
     * onFrame handler executes. If your implementation takes too long to return,
     * one or more frames can be skipped. The Controller still inserts the skipped
     * frames into the frame history. You can access recent frames by setting
     * the history parameter when calling the Controller::frame() function.
     * You can determine if any pending onFrame events were skipped by comparing
     * the ID of the most recent frame with the ID of the last received frame.
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.0
     */
    LEAP_EXPORT virtual void onFrame(const Controller&) {}

    // onServiceConnect/onServiceDisconnect are for connection established/lost.
    // in normal course of events onServiceConnect will get called once after onInit
    // and onServiceDisconnect will not get called. disconnect notification only happens
    // if service stops running or something else bad happens to disconnect controller from service.
    /**
     * Called when the Leap Motion daemon/service connects to your application Controller.
     *
     * \include Listener_onServiceConnect.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.2
     */
    LEAP_EXPORT virtual void onServiceConnect(const Controller&) {}
    /**
     * Called if the Leap Motion daemon/service disconnects from your application Controller.
     *
     * Normally, this callback is not invoked. It is only called if some external event
     * or problem shuts down the service or otherwise interrupts the connection.
     *
     * \include Listener_onServiceDisconnect.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.2
     */
    LEAP_EXPORT virtual void onServiceDisconnect(const Controller&) {}

    /**
     * Called when a Leap Motion controller is plugged in, unplugged, or the device changes state.
     *
     * State changes include entering or leaving robust mode and low resource mode.
     * Note that there is no direct way to query whether the device is in these modes,
     * although you can use Controller::isLightingBad() to check if there are environmental
     * IR lighting problems.
     *
     * \include Listener_onDeviceChange.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 1.2
     */
    LEAP_EXPORT virtual void onDeviceChange(const Controller&) {}

    /**
     * Called when new images are available.
     * Access the new frame data using the Controller::images() function.
     *
     * \include Listener_onImages.txt
     *
     * @param controller The Controller object invoking this callback function.
     * @since 2.2.1
     */
    LEAP_EXPORT virtual void onImages(const Controller&) {}

    /**
    * Called when the Leap Motion service is paused or resumed or when a
    * controller policy is changed.
    *
    * The service can change states because the computer user changes settings
    * in the Leap Motion Control Panel application or because an application
    * connected to the service triggers a change. Any application can pause or
    * unpause the service, but only runtime policy changes you make apply to your
    * own application.
    *
    * \include Listener_onServiceChange.txt
    *
    * You can query the pause state of the controller with Controller::isPaused().
    * You can check the state of those policies you are interested in with
    * Controller::isPolicySet().
    *
    * @param controller The Controller object invoking this callback function.
    * @since 3.0
    */
    LEAP_EXPORT virtual void onServiceChange(const Controller&) {}

    /**
    * Called when a Leap Motion controller device is plugged into the client
    * computer, but fails to operate properly.
    *
    * Get the list containing all failed devices using Controller::failedDevices().
    * The members of this list provide the device pnpID and reason for failure.
    *
    * \include Listener_onDeviceFailure.txt
    *
    * @param controller The Controller object invoking this callback function.
    * @since 3.0
    */
    LEAP_EXPORT virtual void onDeviceFailure(const Controller&) {}

    /**
    * Called when the service emits a log message to report an error, warning, or
    * status change.
    *
    * Log message text is provided as ASCII-encoded english.
    *
    * @param controller The Controller object invoking this callback function.
    * @param severity The severity of the error, if known.
    * @param timestamp The timestamp of the error in microseconds.
    * (Use Controller::now() - timestamp to compute the age of the message.)
    * @param msg The log message.
    * @since 3.0
    */
    LEAP_EXPORT virtual void onLogMessage(const Controller&, MessageSeverity severity, int64_t timestamp, const char* msg) {}

    /**
    * Called when the service emits new head pose data.
    *
    * @param controller The Controller object invoking this callback function.
    * @param position A quaternion representing the head orientation.
    * @param orientation The orthonormal basis vectors for the head position as a matrix.
    * @since 4.0
    */
    LEAP_EXPORT virtual void onHeadPose(const Controller&, int64_t timestamp) {}
  };
}

#endif // __Leap_h__
