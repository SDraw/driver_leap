namespace leap_control
{
    class SfmlManager
    {
        bool m_initialized = false;

        SFML.Window.Context m_context = null;

        public bool Initialize()
        {
            if(!m_initialized)
            {
                m_context = new SFML.Window.Context();
                m_initialized = m_context.SetActive(true);
            }

            return m_initialized;
        }

        public void Terminate()
        {
            if(m_initialized && (m_context != null))
            {
                m_context.SetActive(false);

                m_context = null;
                m_initialized = false;
            }
        }

        public void DoPulse()
        {
            System.Threading.Thread.Sleep(16);
        }
    }
}
