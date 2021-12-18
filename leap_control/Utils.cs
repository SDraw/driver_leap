using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace leap_control
{
    static class Utils
    {
        public static void Convert(this GlmSharp.mat4 f_src, ref Valve.VR.HmdMatrix34_t f_dst)
        {
            f_dst.m0 = f_src.m00;
            f_dst.m1 = f_src.m10;
            f_dst.m2 = f_src.m20;
            f_dst.m3 = f_src.m30;

            f_dst.m4 = f_src.m01;
            f_dst.m5 = f_src.m11;
            f_dst.m6 = f_src.m21;
            f_dst.m7 = f_src.m31;

            f_dst.m8 = f_src.m02;
            f_dst.m9 = f_src.m12;
            f_dst.m10 = f_src.m22;
            f_dst.m11 = f_src.m32;
        }

        public static void Convert(this Valve.VR.HmdMatrix34_t f_src, ref GlmSharp.mat4 f_dst)
        {
            f_dst.m00 = f_src.m0;
            f_dst.m01 = f_src.m4;
            f_dst.m02 = f_src.m8;
            f_dst.m03 = 0f;

            f_dst.m10 = f_src.m1;
            f_dst.m11 = f_src.m5;
            f_dst.m12 = f_src.m9;
            f_dst.m13 = 0f;

            f_dst.m20 = f_src.m2;
            f_dst.m21 = f_src.m6;
            f_dst.m22 = f_src.m10;
            f_dst.m23 = 0f;

            f_dst.m30 = f_src.m3;
            f_dst.m31 = f_src.m7;
            f_dst.m32 = f_src.m11;
            f_dst.m33 = 1f;
        }

        public static void Convert(this Leap.Vector f_src, ref GlmSharp.vec3 f_dst)
        {
            f_dst.x = f_src.x;
            f_dst.y = f_src.y;
            f_dst.z = f_src.z;
        }

        public static float Clamp(float f_value, float f_min, float f_max) => Math.Min(Math.Max(f_value, f_min), f_max);

        public static bool InRange(float f_value, float f_min, float f_max)
        {
            return ((f_value >= f_min) && (f_value <= f_max));
        }
        public static bool IsInRange(this GlmSharp.vec3 f_vec, float f_min, float f_max)
        {
            return (InRange(f_vec.x, f_min, f_max) && InRange(f_vec.y, f_min, f_max) && InRange(f_vec.z, f_min, f_max));
        }
    }
}
