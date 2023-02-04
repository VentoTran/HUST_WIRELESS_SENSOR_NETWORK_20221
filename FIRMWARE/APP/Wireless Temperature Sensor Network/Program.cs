using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace Wireless_Temperature_Sensor_Network
{
    static class Program
    {

        public struct COM_param
        {
            int baudrate;
            int databits;
            StopBits stopbits;
            Parity parity;
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]


        static void Main()
        {
            //Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
