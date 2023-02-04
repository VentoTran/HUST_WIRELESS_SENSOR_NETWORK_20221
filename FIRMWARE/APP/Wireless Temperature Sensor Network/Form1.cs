using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.IO;
using System.Runtime.InteropServices;
using System.Data.SqlClient;

namespace Wireless_Temperature_Sensor_Network
{
    public partial class Form1 : Form
    {

        public int numberOfNode = 10;

        ImageList imgListLarge = new ImageList() {ImageSize = new Size(120, 120)};
        

        public Form1()
        {
            InitializeComponent();

        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // TODO: This line of code loads data into the 'testDataSet.NODE' table. You can move, or remove it, as needed.
            //this.nODETableAdapter.Fill(this.testDataSet.NODE);

            string[] Ports = SerialPort.GetPortNames();
            COMBO_BOX.Items.AddRange(Ports);

            NoD.Text = numberOfNode.ToString();

            toolStripStatusLabel1.Text = "NOT READY";
            toolStripProgressBar1.ForeColor = Color.Red;
            toolStripStatusLabel2.Text = "GATEWAY DISCONNECTED";
            toolStripProgressBar2.ForeColor = Color.Red;

            lstvw.View = View.Details;
            toolStripComboBox1.Text = "DETAILS";

            LoadListView();



            imgListLarge.Images.Add(new Bitmap("D:\\GitHub\\HUST_WIRELESS_SENSOR_NETWORK_20221\\FIRMWARE\\APP\\Wireless Temperature Sensor Network\\Image\\1.png"));
            lstvw.LargeImageList = imgListLarge;

        }


        private void CONNECT_BUTTON_Click(object sender, EventArgs e)
        {
            if (!serialPort1.IsOpen)
            {
                
                try
                {
                    serialPort1.PortName = COMBO_BOX.Text;
                    serialPort1.BaudRate = 115200;
                    serialPort1.DataBits = 8;
                    serialPort1.StopBits = (StopBits)Enum.Parse(typeof(StopBits), "One");
                    serialPort1.Parity = (Parity)Enum.Parse(typeof(Parity), "Even");
                    serialPort1.Open();
                    CONNECT_BUTTON.Text = "DISCONNECT";
                    toolStripStatusLabel2.Text = "GATEWAY CONNECTED";
                }
                catch (Exception err)
                {
                    MessageBox.Show(err.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                serialPort1.Close();
                CONNECT_BUTTON.Text = "CONNECT";
                toolStripStatusLabel2.Text = "GATEWAY DISCONNECTED";
            }
        }

        private void Serial_Port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {

        }

        private void SCAN_BUTTON_Click(object sender, EventArgs e)
        {
            //COM COMMAND FOR SCAN, RECEIVE NUMBER OF NODE AND ITS PROPERTIES
            SqlConnection con = new SqlConnection();
            con.ConnectionString = "Data Source=VENTO;Initial Catalog=Test;Integrated Security=True";
            con.Open();
            String query0 = "DELETE NODE_DETAILS WHERE ID=1";
            String query1 = "INSERT INTO NODE_DETAILS (ID, Address, BatteryVoltage, Temperature, SamplingPeriod, Resolution, Threshold, Status) VALUES (1, 20, 3.6, 18.6, 3.2, 12, 16.0, 'ONLINE')";
            SqlCommand cmd = new SqlCommand(query0, con);
            if (cmd.ExecuteNonQuery() < 0)
            {
                MessageBox.Show("FAIL TO DELETE DATA!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            cmd = new SqlCommand(query1, con);
            if (cmd.ExecuteNonQuery() < 0)
            {
                MessageBox.Show("FAIL TO SAVE DATA!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }


            cmd = new SqlCommand("SELECT * FROM NODE_DETAILS", con);
            SqlDataAdapter da = new SqlDataAdapter(cmd);
            DataSet ds = new DataSet();
            da.Fill(ds, "DataTable");

            con.Close();

            DataTable dt = new DataTable();
            dt = ds.Tables["DataTable"];
            int i;
            for (i = 0; i <= dt.Rows.Count - 1; i++)
            {
                lstvw.Items.Add("NODE " + dt.Rows[i].ItemArray[0].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[1].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[2].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[3].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[4].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[5].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[6].ToString());
                lstvw.Items[i].SubItems.Add(dt.Rows[i].ItemArray[7].ToString());
                lstvw.Items[i].ImageIndex = 0;
            }
        }


        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            Form2 frm = new Form2();
            frm.Show();
        }


        private void COMBO_BOX_Click(object sender, EventArgs e)
        {
            string[] Ports = SerialPort.GetPortNames();
            COMBO_BOX.Items.Clear();
            COMBO_BOX.Items.AddRange(Ports);
        }


        private void toolStripComboBox1_TextChanged(object sender, EventArgs e)
        {
            if (String.Compare(toolStripComboBox1.Text, "DETAILS") == 0)
            {
                lstvw.View = View.Details;
            }
            else
            {
                lstvw.View = View.LargeIcon;
            }
        }

        void LoadListView()
        {
            SqlConnection con = new SqlConnection();
            con.ConnectionString = "Data Source=VENTO;Integrated Security=True";
            con.Open();
            if (con.State == ConnectionState.Closed)
            {
                toolStripStatusLabel3.Text = "DATABASE DISCONNECTED";
                toolStripProgressBar3.ForeColor = Color.Red;
                MessageBox.Show("Can not connect to database\nConnection Failed!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                toolStripStatusLabel3.Text = "DATABASE CONNECTED";
                toolStripProgressBar3.ForeColor = Color.Lime;
                con.Close();
            }


            if (numberOfNode == 0)
            {

            }
            else if (numberOfNode == 1)
            {

            }
            //ListViewItem lstItem = new ListViewItem();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                serialPort1.Close();
            }
        }
    }

}
