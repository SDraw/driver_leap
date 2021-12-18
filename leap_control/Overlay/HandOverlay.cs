using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
        static readonly GlmSharp.vec3 ms_forwardDirection = new GlmSharp.vec3(0f, 0f, -1f);
        //static readonly GlmSharp.vec3 ms_backwardDirection = new GlmSharp.vec3(0f, 0f, 1f);
        static readonly GlmSharp.vec4 ms_pointMultiplier = new GlmSharp.vec4(0f, 0f, 0f, 1f);
        static readonly float ms_overlayWidth = 0.125f;
        static readonly float ms_overlayWidthHalf = ms_overlayWidth * 0.5f;
        static readonly float ms_touchDistance = ms_overlayWidth * 0.5f * 0.5f;
        static readonly float ms_clickDistance = ms_overlayWidth * 0.5f * 0.333333f;
        static readonly float ms_radius = (float)Math.Sqrt(2f * Math.Pow(ms_overlayWidth * 0.5f, 2f));
        static readonly GlmSharp.quat ms_rotation = new GlmSharp.quat(new GlmSharp.vec3(-(float)Math.PI / 2f, 0f, 0f));
        static readonly GlmSharp.vec3 ms_displacement = new GlmSharp.vec3(0f, 0f, 0.0625f);

        static Dictionary<string, SFML.Graphics.Texture> ms_resources = new Dictionary<string, SFML.Graphics.Texture>();

        ulong m_overlay = 0;
        Valve.VR.Texture_t m_overlayTexture;

        SFML.Graphics.RenderTexture m_renderTexture = null;
        SFML.Graphics.Sprite m_backgroundSprite = null;
        SFML.Graphics.Sprite m_thumbstickShape = null;
        SFML.Graphics.Sprite m_touchpadShape = null;
        SFML.Graphics.Sprite m_buttonA = null;
        SFML.Graphics.Sprite m_buttonB = null;
        SFML.Graphics.Sprite m_buttonSystem = null;
        SFML.Graphics.CircleShape m_cursorShape = null;
        SFML.Graphics.CircleShape m_thumbstickAxisShape = null;
        SFML.Graphics.CircleShape m_touchpadAxisShape = null;
        SFML.Graphics.RectangleShape m_presureRectangle = null;
        SFML.Graphics.RectangleShape m_presureFillRectangle = null;

        GlmSharp.vec3 m_position;
        GlmSharp.quat m_rotation;
        GlmSharp.vec3 m_direction;
        GlmSharp.mat4 m_matrix;
        Valve.VR.HmdMatrix34_t m_vrMatrix;

        GlmSharp.vec3 m_tipPositionLocal;
        GlmSharp.vec3 m_tipPositionGlobal;
        bool m_handPresence = false;

        bool m_inRange = false;
        GlmSharp.vec3 m_cursorPosition;
        GlmSharp.vec2 m_cursorPlanePosition;

        List<ControlButton> m_controlButtons = null;

        public HandOverlay(Hand f_hand)
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
            Valve.VR.OpenVR.Overlay.CreateOverlay(ms_overlayNames[(int)f_hand], "Ultraleap hand overlay", ref m_overlay);
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

        public void SetWorldTransform(GlmSharp.mat4 f_mat)
        {
            m_position = (f_mat * ms_pointMultiplier).xyz;
            m_rotation = f_mat.ToQuaternion * ms_rotation;
            m_direction = m_rotation * ms_forwardDirection;
            m_position += m_rotation * ms_displacement;
        }

        public void SetHandPresence(bool f_presence, GlmSharp.vec3 f_tipPos)
        {
            m_handPresence = f_presence;
            m_tipPositionLocal = f_tipPos;
        }

        public List<ControlButton> GetControlButtons() => m_controlButtons;

        public void Update(GlmSharp.mat4 f_headTransform)
        {
            // Reset controls
            foreach(ControlButton l_controlButton in m_controlButtons)
                l_controlButton.ResetUpdate();

            m_tipPositionGlobal = ((f_headTransform * GlmSharp.mat4.Translate(m_tipPositionLocal)) * ms_pointMultiplier).xyz;

            // Update overlays transform
            m_matrix = (GlmSharp.mat4.Translate(m_position) * m_rotation.ToMat4);
            m_matrix.Convert(ref m_vrMatrix);
            Valve.VR.OpenVR.Overlay.SetOverlayTransformAbsolute(m_overlay, Valve.VR.ETrackingUniverseOrigin.TrackingUniverseRawAndUncalibrated, ref m_vrMatrix);

            m_cursorPosition = ((m_matrix.Inverse * GlmSharp.mat4.Translate(m_tipPositionGlobal)) * ms_pointMultiplier).xyz;
            if(m_handPresence && m_cursorPosition.IsInRange(-ms_overlayWidthHalf, ms_overlayWidthHalf) && (m_cursorPosition.z > -0.025f))
            {
                m_cursorShape.FillColor = ((m_cursorPosition.z <= ms_touchDistance) ? ms_touchColor : ms_activeColor);
                m_cursorPlanePosition.x = ((m_cursorPosition.x + ms_overlayWidthHalf) / ms_overlayWidth) * 512f;
                m_cursorPlanePosition.y = ((-m_cursorPosition.y + ms_overlayWidthHalf) / ms_overlayWidth) * 512f;
                m_cursorShape.Position = new SFML.System.Vector2f(m_cursorPlanePosition.x - 5f, m_cursorPlanePosition.y - 5f);

                if(m_cursorPosition.z <= ms_touchDistance)
                {
                    // Check for axes
                    if(m_thumbstickShape.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_thumbstickAxisShape.FillColor = (m_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_thumbstickAxisShape.Position = new SFML.System.Vector2f(m_cursorPlanePosition.x - 7.5f, m_cursorPlanePosition.y - 7.5f);

                        GlmSharp.vec2 l_axes = (m_cursorPlanePosition.xy - 145f) / 105f;
                        l_axes.y *= -1f;

                        m_controlButtons[(int)ButtonIndex.Thumbstick].SetState(m_cursorPosition.z <= ms_clickDistance ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                        m_controlButtons[(int)ButtonIndex.Thumbstick].SetAxes(l_axes);
                    }

                    if(m_touchpadShape.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_touchpadAxisShape.FillColor = (m_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_touchpadAxisShape.Position = new SFML.System.Vector2f(m_cursorPlanePosition.x - 7.5f, m_cursorPlanePosition.y - 7.5f);

                        GlmSharp.vec2 l_axes = (m_cursorPlanePosition.xy - new GlmSharp.vec2(145f, 375f)) / 105f;
                        l_axes.y *= -1f;
                        m_controlButtons[(int)ButtonIndex.Touchpad].SetState(m_cursorPosition.z <= ms_clickDistance ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                        m_controlButtons[(int)ButtonIndex.Touchpad].SetAxes(l_axes);
                    }

                    // Check for buttons
                    if(m_buttonA.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_buttonA.Color = (m_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_controlButtons[(int)ButtonIndex.A].SetState((m_cursorPosition.z <= ms_clickDistance) ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                    }
                    else
                    {
                        m_buttonA.Color = ms_inactiveColor;
                        m_controlButtons[(int)ButtonIndex.A].SetState(ControlButton.ButtonState.None);
                    }

                    if(m_buttonB.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {
                        m_buttonB.Color = (m_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_controlButtons[(int)ButtonIndex.B].SetState((m_cursorPosition.z <= ms_clickDistance) ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
                    }
                    else
                    {
                        m_buttonB.Color = ms_inactiveColor;
                        m_controlButtons[(int)ButtonIndex.B].SetState(ControlButton.ButtonState.None);
                    }

                    if(m_buttonSystem.GetGlobalBounds().Contains(m_cursorPlanePosition.x, m_cursorPlanePosition.y))
                    {

                        m_buttonSystem.Color = (m_cursorPosition.z <= ms_clickDistance ? ms_axisColorClick : ms_axisColorTouch);
                        m_controlButtons[(int)ButtonIndex.System].SetState((m_cursorPosition.z <= ms_clickDistance) ? ControlButton.ButtonState.Clicked : ControlButton.ButtonState.Touched);
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
                    m_controlButtons[(int)ButtonIndex.Touchpad].SetState(ControlButton.ButtonState.None);

                    m_buttonA.Color = ms_inactiveColor;
                    m_controlButtons[(int)ButtonIndex.A].SetState(ControlButton.ButtonState.None);

                    m_buttonB.Color = ms_inactiveColor;
                    m_controlButtons[(int)ButtonIndex.B].SetState(ControlButton.ButtonState.None);

                    m_buttonSystem.Color = ms_inactiveColor;
                    m_controlButtons[(int)ButtonIndex.System].SetState(ControlButton.ButtonState.None);
                }

                // Presure indicator
                float l_presure = 1f - Utils.Clamp(Utils.Clamp(m_cursorPosition.z, 0f, float.MaxValue) / ms_overlayWidthHalf, 0f, 1f);
                m_presureFillRectangle.Size = new SFML.System.Vector2f(m_presureFillRectangle.Size.X, -320f * l_presure);
                m_presureFillRectangle.FillColor = ((l_presure >= 0.5f) ? ((l_presure >= 0.666667f) ? ms_axisColorClick : ms_axisColorTouch) : ms_activeColor);

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
            Valve.VR.OpenVR.Overlay.SetOverlayTexture(m_overlay, ref m_overlayTexture);
        }

        public static void LoadResources()
        {
            ms_resources.Add("background", new SFML.Graphics.Texture(new SFML.Graphics.Image("resources/tx_background.png")));
            ms_resources.Add("circle", new SFML.Graphics.Texture(new SFML.Graphics.Image("resources/tx_circle.png")));
            ms_resources.Add("buttonA", new SFML.Graphics.Texture(new SFML.Graphics.Image("resources/tx_buttonA.png")));
            ms_resources.Add("buttonB", new SFML.Graphics.Texture(new SFML.Graphics.Image("resources/tx_buttonB.png")));
            ms_resources.Add("buttonS", new SFML.Graphics.Texture(new SFML.Graphics.Image("resources/tx_buttonS.png")));
        }
    }
}
