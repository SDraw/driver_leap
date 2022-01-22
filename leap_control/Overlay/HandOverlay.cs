using System;
using System.Collections.Generic;

namespace leap_control
{
    class HandOverlay
    {
        public enum Hand
        {
            Left = 0,
            Right
        };

        public enum ButtonIndex
        {
            A = 0,
            B,
            System,
            Thumbstick,
            Touchpad
        };

        static readonly string[] ms_overlayNames = { "leap.overlay.handLeft", "leap.overlay.handRight" };
        static readonly SFML.Graphics.Color ms_emptyColor = new SFML.Graphics.Color(0x0, 0x0, 0x0, 0x0);
        static readonly SFML.Graphics.Color ms_backgroundColor = new SFML.Graphics.Color(0x4D, 0x4D, 0x4D);
        static readonly SFML.Graphics.Color ms_inactiveColor = new SFML.Graphics.Color(0x33, 0x33, 0x33);
        static readonly SFML.Graphics.Color ms_activeColor = new SFML.Graphics.Color(0x33, 0x99, 0xFF);
        static readonly SFML.Graphics.Color ms_touchColor = new SFML.Graphics.Color(0xFF, 0x00, 0x00);
        static readonly SFML.Graphics.Color ms_axisColorTouch = new SFML.Graphics.Color(0x00, 0xFF, 0x00);
        static readonly SFML.Graphics.Color ms_axisColorClick = new SFML.Graphics.Color(0xFF, 0xA5, 0x00);
        static readonly float ms_overlayWidth = 0.125f;
        static readonly float ms_overlayWidthHalf = ms_overlayWidth * 0.5f;
        static readonly float ms_touchDistance = ms_overlayWidth * 0.5f * 0.5f;
        static readonly float ms_clickDistance = ms_overlayWidth * 0.5f * 0.25f;
        static readonly GlmSharp.quat ms_rotationOffset = new GlmSharp.quat(new GlmSharp.vec3(-(float)Math.PI / 2f, 0f, 0f));
        static readonly GlmSharp.vec3 ms_positionOffset = new GlmSharp.vec3(0f, 0f, 0.0625f);

        static Dictionary<string, SFML.Graphics.Texture> ms_resources = new Dictionary<string, SFML.Graphics.Texture>();

        ulong m_overlay = Valve.VR.OpenVR.k_ulOverlayHandleInvalid;
        Valve.VR.Texture_t m_overlayTexture;

        readonly SFML.Graphics.RenderTexture m_renderTexture = null;
        readonly SFML.Graphics.Sprite m_backgroundSprite = null;
        readonly SFML.Graphics.Sprite m_thumbstickShape = null;
        readonly SFML.Graphics.Sprite m_touchpadShape = null;
        readonly SFML.Graphics.Sprite m_buttonA = null;
        readonly SFML.Graphics.Sprite m_buttonB = null;
        readonly SFML.Graphics.Sprite m_buttonSystem = null;
        readonly SFML.Graphics.CircleShape m_cursorShape = null;
        readonly SFML.Graphics.CircleShape m_thumbstickAxisShape = null;
        readonly SFML.Graphics.CircleShape m_touchpadAxisShape = null;
        readonly SFML.Graphics.RectangleShape m_presureRectangle = null;
        readonly SFML.Graphics.RectangleShape m_presureFillRectangle = null;

        GlmSharp.vec3 m_position;
        GlmSharp.quat m_rotation;
        GlmSharp.vec3 m_direction;
        Valve.VR.HmdMatrix34_t m_vrMatrix;

        GlmSharp.vec3 m_tipPositionLocal;
        bool m_handPresence = false;

        bool m_inRange = false;
        GlmSharp.vec2 m_cursorPlanePosition;

        readonly List<ControlButton> m_controlButtons = null;

        float m_opacity = 1f;
        float m_rangeOpacity = 0.5f;

        bool m_locked = false;

        public HandOverlay(Hand p_hand)
        {
            m_renderTexture = new SFML.Graphics.RenderTexture(512, 512);

            m_backgroundSprite = new SFML.Graphics.Sprite(ms_resources["background"]);
            m_backgroundSprite.Color = ms_backgroundColor;

            m_thumbstickShape = new SFML.Graphics.Sprite(ms_resources["circle"]);
            m_thumbstickShape.Color = ms_inactiveColor;
            m_thumbstickShape.Position = new SFML.System.Vector2f(40f, 40f);

            m_touchpadShape = new SFML.Graphics.Sprite(ms_resources["circle"]);
            m_touchpadShape.Color = ms_inactiveColor;
            m_touchpadShape.Position = new SFML.System.Vector2f(40f, 270f);

            m_buttonB = new SFML.Graphics.Sprite(ms_resources["buttonB"]);
            m_buttonB.Color = ms_inactiveColor;
            m_buttonB.Position = new SFML.System.Vector2f(310f, 95f);

            m_buttonA = new SFML.Graphics.Sprite(ms_resources["buttonA"]);
            m_buttonA.Color = ms_inactiveColor;
            m_buttonA.Position = new SFML.System.Vector2f(310f, 210f);

            m_buttonSystem = new SFML.Graphics.Sprite(ms_resources["buttonS"]);
            m_buttonSystem.Color = ms_inactiveColor;
            m_buttonSystem.Position = new SFML.System.Vector2f(310f, 325f);

            m_cursorShape = new SFML.Graphics.CircleShape(5f);
            m_cursorShape.FillColor = ms_activeColor;

            m_thumbstickAxisShape = new SFML.Graphics.CircleShape(7.5f);
            m_thumbstickAxisShape.FillColor = ms_axisColorTouch;

            m_touchpadAxisShape = new SFML.Graphics.CircleShape(7.5f);
            m_touchpadAxisShape.FillColor = ms_axisColorTouch;

            m_presureRectangle = new SFML.Graphics.RectangleShape(new SFML.System.Vector2f(10f, 320f));
            m_presureRectangle.Position = new SFML.System.Vector2f(480f, 100f);
            m_presureRectangle.FillColor = ms_inactiveColor;

            m_presureFillRectangle = new SFML.Graphics.RectangleShape(new SFML.System.Vector2f(10f, -320f));
            m_presureFillRectangle.Position = new SFML.System.Vector2f(480f, 420f);
            m_presureFillRectangle.FillColor = ms_inactiveColor;

            // Create overlay
            Valve.VR.OpenVR.Overlay.CreateOverlay(ms_overlayNames[(int)p_hand], "Ultraleap hand overlay", ref m_overlay);
            Valve.VR.OpenVR.Overlay.SetOverlayWidthInMeters(m_overlay, ms_overlayWidth);
            Valve.VR.OpenVR.Overlay.ShowOverlay(m_overlay);
            m_overlayTexture.eColorSpace = Valve.VR.EColorSpace.Gamma;
            m_overlayTexture.eType = Valve.VR.ETextureType.OpenGL;
            m_overlayTexture.handle = (IntPtr)m_renderTexture.Texture.NativeHandle;

            // Controls
            m_controlButtons = new List<ControlButton>();
            m_controlButtons.Add(new ControlButton(ControlButton.ButtonType.Button, "a"));
            m_controlButtons.Add(new ControlButton(ControlButton.ButtonType.Button, "b"));
            m_controlButtons.Add(new ControlButton(ControlButton.ButtonType.Button, "system"));
            m_controlButtons.Add(new ControlButton(ControlButton.ButtonType.Axis, "thumbstick"));
            m_controlButtons.Add(new ControlButton(ControlButton.ButtonType.Axis, "touchpad"));
        }

        public void SetWorldTransform(GlmSharp.mat4 p_mat)
        {
            m_position = (p_mat * GlmSharp.vec4.UnitW).xyz;
            m_rotation = p_mat.ToQuaternion * ms_rotationOffset;
            m_direction = m_rotation * -GlmSharp.vec3.UnitZ;
            m_position += m_rotation * ms_positionOffset;
        }

        public void SetHandPresence(bool p_presence, GlmSharp.vec3 p_tipPos)
        {
            m_handPresence = p_presence;
            m_tipPositionLocal = p_tipPos;
        }

        public List<ControlButton> GetControlButtons() => m_controlButtons;

        public void Update(GlmSharp.mat4 p_headTransform)
        {
            // Reset controls
            foreach(ControlButton l_controlButton in m_controlButtons)
                l_controlButton.ResetUpdate();

            GlmSharp.vec3 l_tipPositionGlobal = ((p_headTransform * GlmSharp.mat4.Translate(m_tipPositionLocal)) * GlmSharp.vec4.UnitW).xyz;

            // Update overlays transform
            GlmSharp.mat4 l_matrix = (GlmSharp.mat4.Translate(m_position) * m_rotation.ToMat4);
            l_matrix.Convert(ref m_vrMatrix);
            Valve.VR.OpenVR.Overlay.SetOverlayTransformAbsolute(m_overlay, Valve.VR.ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, ref m_vrMatrix);

            GlmSharp.vec3 l_cursorPosition = ((l_matrix.Inverse * GlmSharp.mat4.Translate(l_tipPositionGlobal)) * GlmSharp.vec4.UnitW).xyz;
            if(m_handPresence && !m_locked && l_cursorPosition.IsInRange(-ms_overlayWidthHalf, ms_overlayWidthHalf) && (l_cursorPosition.z > -0.025f))
            {
                m_cursorShape.FillColor = ((l_cursorPosition.z <= ms_touchDistance) ? ms_touchColor : ms_activeColor);
                m_cursorPlanePosition.x = ((l_cursorPosition.x + ms_overlayWidthHalf) / ms_overlayWidth) * 512f;
                m_cursorPlanePosition.y = ((-l_cursorPosition.y + ms_overlayWidthHalf) / ms_overlayWidth) * 512f;
                m_cursorShape.Position = new SFML.System.Vector2f(m_cursorPlanePosition.x - 5f, m_cursorPlanePosition.y - 5f);

                if(l_cursorPosition.z <= ms_touchDistance)
                {
                    // Check for axes
                    if(m_thumbstickShape.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_thumbstickAxisShape.FillColor = (l_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_thumbstickAxisShape.Position = new SFML.System.Vector2f(m_cursorPlanePosition.x - 7.5f, m_cursorPlanePosition.y - 7.5f);

                        GlmSharp.vec2 l_axes = (m_cursorPlanePosition.xy - 145f) / 105f;
                        l_axes.y *= -1f;

                        m_controlButtons[(int)ButtonIndex.Thumbstick].SetState(l_cursorPosition.z <= ms_clickDistance ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                        m_controlButtons[(int)ButtonIndex.Thumbstick].SetAxes(l_axes);
                    }
                    else
                    {
                        m_controlButtons[(int)ButtonIndex.Thumbstick].SetState(ControlButton.ButtonState.None);
                        m_controlButtons[(int)ButtonIndex.Thumbstick].SetAxes(GlmSharp.vec2.Zero);
                    }

                    if(m_touchpadShape.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_touchpadAxisShape.FillColor = (l_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_touchpadAxisShape.Position = new SFML.System.Vector2f(m_cursorPlanePosition.x - 7.5f, m_cursorPlanePosition.y - 7.5f);

                        GlmSharp.vec2 l_axes = (m_cursorPlanePosition.xy - new GlmSharp.vec2(145f, 375f)) / 105f;
                        l_axes.y *= -1f;
                        m_controlButtons[(int)ButtonIndex.Touchpad].SetState(l_cursorPosition.z <= ms_clickDistance ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                        m_controlButtons[(int)ButtonIndex.Touchpad].SetAxes(l_axes);
                    }
                    else
                    {
                        m_controlButtons[(int)ButtonIndex.Touchpad].SetState(ControlButton.ButtonState.None);
                        m_controlButtons[(int)ButtonIndex.Touchpad].SetAxes(GlmSharp.vec2.Zero);
                    }

                    // Check for buttons
                    if(m_buttonA.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_buttonA.Color = (l_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_controlButtons[(int)ButtonIndex.A].SetState((l_cursorPosition.z <= ms_clickDistance) ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                    }
                    else
                    {
                        m_buttonA.Color = ms_inactiveColor;
                        m_controlButtons[(int)ButtonIndex.A].SetState(ControlButton.ButtonState.None);
                    }

                    if(m_buttonB.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_buttonB.Color = (l_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_controlButtons[(int)ButtonIndex.B].SetState((l_cursorPosition.z <= ms_clickDistance) ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                    }
                    else
                    {
                        m_buttonB.Color = ms_inactiveColor;
                        m_controlButtons[(int)ButtonIndex.B].SetState(ControlButton.ButtonState.None);
                    }

                    if(m_buttonSystem.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {

                        m_buttonSystem.Color = (l_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_controlButtons[(int)ButtonIndex.System].SetState((l_cursorPosition.z <= ms_clickDistance) ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                    }
                    else
                    {
                        m_buttonSystem.Color = ms_inactiveColor;
                        m_controlButtons[(int)ButtonIndex.System].SetState(ControlButton.ButtonState.None);
                    }
                }
                else
                {
                    m_controlButtons[(int)ButtonIndex.Thumbstick].SetState(ControlButton.ButtonState.None);
                    m_controlButtons[(int)ButtonIndex.Thumbstick].SetAxes(GlmSharp.vec2.Zero);
                    m_controlButtons[(int)ButtonIndex.Touchpad].SetState(ControlButton.ButtonState.None);
                    m_controlButtons[(int)ButtonIndex.Touchpad].SetAxes(GlmSharp.vec2.Zero);

                    m_buttonA.Color = ms_inactiveColor;
                    m_controlButtons[(int)ButtonIndex.A].SetState(ControlButton.ButtonState.None);

                    m_buttonB.Color = ms_inactiveColor;
                    m_controlButtons[(int)ButtonIndex.B].SetState(ControlButton.ButtonState.None);

                    m_buttonSystem.Color = ms_inactiveColor;
                    m_controlButtons[(int)ButtonIndex.System].SetState(ControlButton.ButtonState.None);
                }

                // Presure indicator
                float l_presure = 1f - GlmSharp.glm.Clamp(GlmSharp.glm.Clamp(l_cursorPosition.z, 0f, float.MaxValue) / ms_overlayWidthHalf, 0f, 1f);
                m_presureFillRectangle.Size = new SFML.System.Vector2f(m_presureFillRectangle.Size.X, -320f * l_presure);
                m_presureFillRectangle.FillColor = ((l_presure >= 0.5f) ? ((l_presure >= 0.75f) ? ms_axisColorClick : ms_axisColorTouch) : ms_activeColor);

                m_inRange = true;
            }
            else
                m_inRange = false;

            // Draw
            if(m_renderTexture.SetActive(true))
            {
                m_renderTexture.Clear(ms_emptyColor);
                m_renderTexture.Draw(m_backgroundSprite);
                m_renderTexture.Draw(m_thumbstickShape);
                m_renderTexture.Draw(m_touchpadShape);
                m_renderTexture.Draw(m_buttonA);
                m_renderTexture.Draw(m_buttonB);
                m_renderTexture.Draw(m_buttonSystem);
                m_renderTexture.Draw(m_presureRectangle);

                if(m_inRange)
                {
                    m_renderTexture.Draw(m_presureFillRectangle);
                    m_renderTexture.Draw(m_thumbstickAxisShape);
                    m_renderTexture.Draw(m_touchpadAxisShape);
                    m_renderTexture.Draw(m_cursorShape);
                }

                m_renderTexture.Display();
                m_renderTexture.SetActive(false);
            }

            // Update overlay
            m_rangeOpacity = GlmSharp.glm.Lerp(m_rangeOpacity, m_locked ? 0f : 0.5f, 0.25f);
            m_opacity = GlmSharp.glm.Lerp(m_opacity, m_inRange ? 1f : m_rangeOpacity, 0.25f);
            Valve.VR.OpenVR.Overlay.SetOverlayTexture(m_overlay, ref m_overlayTexture);
            Valve.VR.OpenVR.Overlay.SetOverlayAlpha(m_overlay, m_opacity);
        }

        public bool GetLocked() => m_locked;
        public bool SetLocked(bool p_state) => m_locked = p_state;

        public bool IsInputActive() => m_inRange;

        public static void LoadResources()
        {
            ms_resources.Add("background", new SFML.Graphics.Texture(new SFML.Graphics.Image("../../resources/textures/tx_background.png")));
            ms_resources.Add("circle", new SFML.Graphics.Texture(new SFML.Graphics.Image("../../resources/textures/tx_circle.png")));
            ms_resources.Add("buttonA", new SFML.Graphics.Texture(new SFML.Graphics.Image("../../resources/textures/tx_buttonA.png")));
            ms_resources.Add("buttonB", new SFML.Graphics.Texture(new SFML.Graphics.Image("../../resources/textures/tx_buttonB.png")));
            ms_resources.Add("buttonS", new SFML.Graphics.Texture(new SFML.Graphics.Image("../../resources/textures/tx_buttonS.png")));
        }
    }
}
