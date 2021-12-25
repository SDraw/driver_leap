using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace leap_control
{
    static class Utils
    {
        public static void Convert(this GlmSharp.mat4 p_src, ref Valve.VR.HmdMatrix34_t p_dst)
        {
            p_dst.m0 = p_src.m00;
            p_dst.m1 = p_src.m10;
            p_dst.m2 = p_src.m20;
            p_dst.m3 = p_src.m30;

            p_dst.m4 = p_src.m01;
            p_dst.m5 = p_src.m11;
            p_dst.m6 = p_src.m21;
            p_dst.m7 = p_src.m31;

            p_dst.m8 = p_src.m02;
            p_dst.m9 = p_src.m12;
            p_dst.m10 = p_src.m22;
            p_dst.m11 = p_src.m32;
        }

        public static void Convert(this Valve.VR.HmdMatrix34_t p_src, ref GlmSharp.mat4 p_dst)
        {
            p_dst.m00 = p_src.m0;
            p_dst.m01 = p_src.m4;
            p_dst.m02 = p_src.m8;
            p_dst.m03 = 0f;

            p_dst.m10 = p_src.m1;
            p_dst.m11 = p_src.m5;
            p_dst.m12 = p_src.m9;
            p_dst.m13 = 0f;

            p_dst.m20 = p_src.m2;
            p_dst.m21 = p_src.m6;
            p_dst.m22 = p_src.m10;
            p_dst.m23 = 0f;

            p_dst.m30 = p_src.m3;
            p_dst.m31 = p_src.m7;
            p_dst.m32 = p_src.m11;
            p_dst.m33 = 1f;
        }

        public static void Convert(this Leap.Vector p_src, ref GlmSharp.vec3 p_dst)
        {
            p_dst.x = p_src.x;
            p_dst.y = p_src.y;
            p_dst.z = p_src.z;
        }

        public static bool InRange(float p_value, float p_min, float p_max)
        {
            return ((p_value >= p_min) && (p_value <= p_max));
        }
        public static bool IsInRange(this GlmSharp.vec3 p_vec, float p_min, float p_max)
        {
            return (InRange(p_vec.x, p_min, p_max) && InRange(p_vec.y, p_min, p_max) && InRange(p_vec.z, p_min, p_max));
        }
    }
}
