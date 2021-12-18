using System;

namespace leap_control
{
    class Core
    {
        bool m_initialized = false;
        bool m_active = false;

        SfmlManager m_sfmlManager = null;
        VRManager m_vrManager = null;
        LeapManager m_leapManager = null;
        ControlManager m_controlManager = null;

        System.Windows.Forms.NotifyIcon m_trayIcon = null;

        public ControlManager ControlManager
        {
            get => m_controlManager;
        }

        public VRManager VRManager
        {
            get => m_vrManager;
        }

        bool Initialize()
        {
            if(!m_initialized)
            {
                m_sfmlManager = new SfmlManager();
                m_vrManager = new VRManager(this);
                m_leapManager = new LeapManager(this);
                m_controlManager = new ControlManager(this);

                m_trayIcon = new System.Windows.Forms.NotifyIcon();
                try
                {
                    m_trayIcon.Icon = new System.Drawing.Icon("icon.ico");
                }
                catch(Exception) { }
                m_trayIcon.Text = "Driver Leap Control";
                m_trayIcon.Visible = true;

                m_initialized = (m_vrManager.Initialize() && m_sfmlManager.Initialize() && m_leapManager.Initialize() && m_controlManager.Initialize());
                m_active = m_initialized;
            }

            return m_initialized;
        }

        void Terminate()
        {
            if(m_controlManager != null)
            {
                m_controlManager.Terminate();
                m_controlManager = null;
            }

            if(m_leapManager != null)
            {
                m_leapManager.Terminate();
                m_leapManager = null;
            }

            if(m_sfmlManager != null)
            {
                m_sfmlManager.Terminate();
                m_sfmlManager = null;
            }

            if(m_vrManager != null)
            {
                m_vrManager.Terminate();
                m_vrManager = null;
            }

            if(m_trayIcon != null)
            {
                m_trayIcon.Visible = false;
                m_trayIcon.Dispose();
                m_trayIcon = null;
            }

            m_initialized = false;
        }

        bool DoPulse()
        {
            m_active |= m_vrManager.DoPulse();
            m_leapManager.DoPulse();
            m_controlManager.DoPulse();

            m_sfmlManager.DoPulse();
            return m_active;
        }

        static void Main(string[] args)
        {
            Core l_core = new Core();
            if(l_core.Initialize())
            {
                while(l_core.DoPulse()) ;
                l_core.Terminate();
            }

        }
    }
}
