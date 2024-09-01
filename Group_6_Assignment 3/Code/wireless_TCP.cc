#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include <fstream>
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Wireless_Topology");
Ptr<PacketSink> sink;                         /*packet sink application's pointer */



int main (int argc, char *argv[])
{

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVeno::GetTypeId ())); // Select TCP type {TCPVegas, TCPWestwoodPlus, TCPVeno}

  std::vector<uint32_t> packets = {50, 54, 58, 62, 70, 652, 676, 728, 1520,1600};


  for(auto it : packets)
  {
    std::cout << "Packet Size : " << it << std::endl;

    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (it));

    WifiMacHelper apWifiMac,staWifiMac;
    WifiHelper wifiHelper;
    wifiHelper.SetStandard (WIFI_STANDARD_80211n);


    YansWifiChannelHelper wifiChannel1,wifiChannel2;
    wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel1.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
    wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel2.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));


    YansWifiPhyHelper wifiAP1 = YansWifiPhyHelper();
    wifiAP1.SetChannel (wifiChannel1.Create ());
    wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));

    YansWifiPhyHelper wifiAP2 = YansWifiPhyHelper();
    wifiAP2.SetChannel (wifiChannel2.Create ());
    wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));


    NS_LOG_INFO("Creating Nodes");
    Ptr<Node> n0 = CreateObject<Node> ();
    Ptr<Node> bs1 = CreateObject<Node> ();
    Ptr<Node> bs2 = CreateObject<Node> ();
    Ptr<Node> n1 = CreateObject<Node> ();


    NodeContainer baseStationNode = NodeContainer(bs1,bs2);
    NodeContainer staWifiNode = NodeContainer(n0,n1);
    NodeContainer all = NodeContainer(n0,n1,bs1,bs2);


    PointToPointHelper p2p;
    p2p.SetChannelAttribute ("Delay", StringValue ("100ms"));
    p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));

    NetDeviceContainer basedevice = p2p.Install (baseStationNode);

    NetDeviceContainer apDevice1,apDevice2,staDevices1,staDevices2;

    Ssid ssid = Ssid ("wireless-network");
    apWifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));

    apDevice1 = wifiHelper.Install (wifiAP1, apWifiMac, bs1);
    apDevice2 = wifiHelper.Install (wifiAP2, apWifiMac, bs2);


    staWifiMac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid));

    staDevices1 = wifiHelper.Install (wifiAP1, staWifiMac, n0);
    staDevices2 = wifiHelper.Install (wifiAP2, staWifiMac, n1);


    MobilityHelper mobility;

    Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
    positionAlloc1->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc1->Add (Vector (5.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc1);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (n0);
    mobility.Install (bs1);


    Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
    positionAlloc2->Add (Vector (10.0, 0.0, 0.0));
    positionAlloc2->Add (Vector (15.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc2);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (bs2);
    mobility.Install (n1);


    InternetStackHelper stack;
    stack.Install (all);


    Ipv4AddressHelper ipv4;
    Ipv4InterfaceContainer apInterface1,apInterface2,staInterface1,staInterface2;

    ipv4.SetBase ("192.168.1.0", "255.255.255.0");
    staInterface1 = ipv4.Assign (staDevices1);
    apInterface1 = ipv4.Assign (apDevice1);

    ipv4.SetBase ("192.168.2.0", "255.255.255.0");
    apInterface2 = ipv4.Assign (apDevice2);
    staInterface2 = ipv4.Assign (staDevices2);

    ipv4.SetBase ("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer baseinterface = ipv4.Assign (basedevice);
    std::cout <<"Interface Addresses"  <<"\n";
    std::cout <<"Node0 : " << staInterface1.GetAddress(0) << "\t";
    std::cout <<"BaseStation1 : " << apInterface1.GetAddress(0) << "\t";
    std::cout <<"BaseStation2 : " << apInterface2.GetAddress(0) << "\t";
    std::cout <<"Node1 : " << staInterface2.GetAddress(0) << std::endl;
    std::cout << std::endl;


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
    ApplicationContainer sinkApp = sinkHelper.Install (n1);
    sink = StaticCast<PacketSink> (sinkApp.Get(0));


    OnOffHelper serverHelper ("ns3::TcpSocketFactory", (InetSocketAddress (staInterface2.GetAddress (0), 9)));
    serverHelper.SetAttribute ("PacketSize", UintegerValue (it));
    serverHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    serverHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    serverHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
    ApplicationContainer serverApp = serverHelper.Install (n0);

    /* Start Applications */
    sinkApp.Start (Seconds (0.0));
    serverApp.Start (Seconds (10.0));

    FlowMonitorHelper flowmonitor;
    Ptr<FlowMonitor> monitor = flowmonitor.InstallAll();
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds(11.0));
    Simulator::Run ();

    //Fairness Index and Throughput calculation
    double Sumx = 0, SumSqx = 0;
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonitor.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {

      double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();

      double Throughput = ((i->second.rxBytes * 8.0) / time)/1024;
      std::cout << "  Throughput observed for flow "<<i->first<<" : " << Throughput  << " Kbps\n\n";

      Sumx += Throughput;
      SumSqx += Throughput * Throughput ;
    }

    double FairnessIndex = (Sumx * Sumx)/ (2 * SumSqx) ;
    std :: cout << "Average Throughput: \t" << Sumx/2 << " Kbps" << std::endl;
    std :: cout << "FairnessIndex: \t" << FairnessIndex << std :: endl << std::endl;

    Simulator::Destroy ();
    NS_LOG_INFO ("Complete");
  }


  return 0;
}
