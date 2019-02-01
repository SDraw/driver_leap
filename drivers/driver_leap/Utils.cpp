#include "stdafx.h"
#include "Utils.h"

void GenerateSerialNumber(char* p, int psize, int controller)
{
    char tmp[32];
    _snprintf(tmp, 32, "controller%d", controller);
    _snprintf(p, psize, "leap0_%s", (controller == LEFT_CONTROLLER) ? "lefthand" : (controller == RIGHT_CONTROLLER) ? "righthand" : tmp);
}

// convert a 3x3 rotation matrix into a rotation quaternion
void CalculateRotation(float(* a)[3], vr::HmdQuaternion_t &f_result)
{
    float trace = a[0][0] + a[1][1] + a[2][2];
    if(trace > 0)
    {
        float s = 0.5f / sqrtf(trace + 1.0f);
        f_result.w = 0.25f / s;
        f_result.x = (a[2][1] - a[1][2]) * s;
        f_result.y = (a[0][2] - a[2][0]) * s;
        f_result.z = (a[1][0] - a[0][1]) * s;
    }
    else
    {
        if(a[0][0] > a[1][1] && a[0][0] > a[2][2])
        {
            float s = 2.0f * sqrtf(1.0f + a[0][0] - a[1][1] - a[2][2]);
            f_result.w = (a[2][1] - a[1][2]) / s;
            f_result.x = 0.25f * s;
            f_result.y = (a[0][1] + a[1][0]) / s;
            f_result.z = (a[0][2] + a[2][0]) / s;
        }
        else if(a[1][1] > a[2][2])
        {
            float s = 2.0f * sqrtf(1.0f + a[1][1] - a[0][0] - a[2][2]);
            f_result.w = (a[0][2] - a[2][0]) / s;
            f_result.x = (a[0][1] + a[1][0]) / s;
            f_result.y = 0.25f * s;
            f_result.z = (a[1][2] + a[2][1]) / s;
        }
        else
        {
            float s = 2.0f * sqrtf(1.0f + a[2][2] - a[0][0] - a[1][1]);
            f_result.w = (a[1][0] - a[0][1]) / s;
            f_result.x = (a[0][2] + a[2][0]) / s;
            f_result.y = (a[1][2] + a[2][1]) / s;
            f_result.z = 0.25f * s;
        }
    }
    f_result.x = -f_result.x;
    f_result.y = -f_result.y;
    f_result.z = -f_result.z;
}

// generate a rotation quaternion around an arbitrary axis
vr::HmdQuaternion_t rotate_around_axis(const Leap::Vector& v, const float& a)
{
    // Here we calculate the sin( a / 2) once for optimization
    float factor = sinf(a* 0.01745329 / 2.0);

    // Calculate the x, y and z of the quaternion
    float x = v.x * factor;
    float y = v.y * factor;
    float z = v.z * factor;

    // Calcualte the w value by cos( a / 2 )
    float w = cosf(a* 0.01745329 / 2.0);

    float mag = sqrtf(w*w + x*x + y*y + z*z);

    vr::HmdQuaternion_t result = { w / mag, x / mag, y / mag, z / mag };
    return result;
}

// multiplication of quaternions
vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& a, const vr::HmdQuaternion_t& b)
{
    vr::HmdQuaternion_t tmp;

    tmp.w = (b.w * a.w) - (b.x * a.x) - (b.y * a.y) - (b.z * a.z);
    tmp.x = (b.w * a.x) + (b.x * a.w) + (b.y * a.z) - (b.z * a.y);
    tmp.y = (b.w * a.y) + (b.y * a.w) + (b.z * a.x) - (b.x * a.z);
    tmp.z = (b.w * a.z) + (b.z * a.w) + (b.x * a.y) - (b.y * a.x);

    return tmp;
}

int ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec)
{
    int l_result = -1;
    for(auto iter = f_vec.begin(), iterEnd = f_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(f_val))
        {
            l_result = std::distance(f_vec.begin(),iter);
            break;
        }
    }
    return l_result;
}
