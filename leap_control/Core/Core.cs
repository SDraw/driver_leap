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

        System.Threading.Thread m_trayThread = null;
        System.Windows.Forms.NotifyIcon m_trayIcon = null;

        bool Initialize()
        {
            if(!m_initialized)
            {
                m_sfmlManager = new SfmlManager();
                m_vrManager = new VRManager(this);
                m_leapManager = new LeapManager(this);
                m_controlManager = new ControlManager(this);

                m_initialized = (m_vrManager.Initialize() && m_sfmlManager.Initialize() && m_leapManager.Initialize() && m_controlManager.Initialize());

                if(m_initialized)
                {
                    m_trayThread = new System.Threading.Thread(() =>
                    {
                        try
                        {
                            m_trayIcon = new System.Windows.Forms.NotifyIcon();
                            m_trayIcon.Icon = new System.Drawing.Icon("icon.ico");
                            m_trayIcon.Text = "Driver Leap Control";
                            m_trayIcon.Visible = true;

                            m_trayIcon.ContextMenu = new System.Windows.Forms.ContextMenu();
                            var l_reloadItem = new System.Windows.Forms.MenuItem();
                            l_reloadItem.Text = "Reload settings";
                            l_reloadItem.Click += new EventHandler((o, e) =>
                            {
                                // Safe to call from another thread
                                this.GetVRManager().SendMessage("reload");
                                this.GetVRManager().ShowNotification("Settings reloaded");
                            });
                            m_trayIcon.ContextMenu.MenuItems.Add(l_reloadItem);

                            System.Windows.Forms.Application.Run();
                        }
                        catch(System.Threading.ThreadAbortException)
                        {
                            System.Windows.Forms.Application.Exit();
                        }
                        catch(Exception) { }
                    });
                    m_trayThread.Start();
                }

                m_active = m_initialized;
            }

            return m_initialized;
        }

        void Terminate()
        {
            m_controlManager?.Terminate();
            m_controlManager = null;

            m_leapManager?.Terminate();
            m_leapManager = null;

            m_sfmlManager?.Terminate();
            m_sfmlManager = null;

            m_vrManager?.Terminate();
            m_vrManager = null;

            m_trayThread?.Abort();
            m_trayThread = null;

            m_trayIcon?.Dispose();
            m_trayIcon = null;

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

        public VRManager GetVRManager() => m_vrManager;
        public ControlManager GetControlManager() => m_controlManager;

        static void Main(string[] args)
        {
            Core l_core = new Core();
            if(l_core.Initialize())
            {
                while(l_core.DoPulse())
                { }
                l_core.Terminate();
            }
        }
    }
}
