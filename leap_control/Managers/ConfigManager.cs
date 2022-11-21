using System;
using System.Linq;
using System.Xml;

namespace leap_control
{
    class ConfigManager
    {
        GlmSharp.mat4 m_rootTransform;

        public ConfigManager()
        {
            m_rootTransform = GlmSharp.mat4.Identity;
        }

        public void Load()
        {
            GlmSharp.vec3 l_point = GlmSharp.vec3.Zero;
            float l_angle = 0f;

            try
            {
                XmlDocument l_xml = new XmlDocument();
                l_xml.Load("../../resources/settings.xml");
                XmlElement l_rootNode = l_xml["settings"];
                if(l_rootNode != null)
                {
                    for(XmlNode l_node = l_rootNode["setting"]; l_node != null; l_node = l_node.NextSibling)
                    {
                        XmlAttribute l_attribName = l_node.Attributes?["name"];
                        XmlAttribute l_attribValue = l_node.Attributes?["value"];
                        if((l_attribName != null) && (l_attribValue != null))
                        {
                            switch(l_attribName.Value)
                            {
                                case "rootOffset":
                                {
                                    float[] l_values = l_attribValue.Value.Split().Select(x => float.Parse(x, System.Globalization.CultureInfo.InvariantCulture)).ToArray();
                                    if(l_values.Count() >= 3)
                                    {
                                        for(int i = 0; i < 3; i++)
                                            l_point[i] = l_values[i];
                                    }
                                }
                                break;

                                case "rootAngle":
                                {
                                    l_angle = float.Parse(l_attribValue.Value, System.Globalization.CultureInfo.InvariantCulture);
                                }
                                break;
                            }
                        }
                    }
                }
            }
            catch(Exception) { }

            m_rootTransform = GlmSharp.mat4.Translate(l_point) * GlmSharp.mat4.RotateX(l_angle);
        }

        public GlmSharp.mat4 GetRootTransform() => m_rootTransform;
    }
}
