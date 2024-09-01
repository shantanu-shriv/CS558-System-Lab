#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "ns3/tcp-westwood-plus.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Wired_Topology");


class WiredTCP : public Application {
public:
	WiredTCP ();
	virtual ~WiredTCP();

	void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate datarate);

private:
	virtual void StartApplication (void);
  	virtual void StopApplication (void);
  	
  	void ScheduleTx (void);
  	void SendPacket (void);

  	Ptr<Socket> socket;
  	Address peer;
  	uint32_t packetSize;
  	uint32_t nPackets;
  	DataRate dataRate;
  	EventId sendEvent;
  	bool isRunning;
  	uint32_t packetsSent;
};



int main(int argc, char *argv[]) {
	bool flag = 1;

	std::vector<uint32_t> packetSizes = {50, 54, 58, 62, 70, 652, 676, 728, 1520,1600};
	uint32_t nPackets = 100;

	for(auto it : packetSizes) {
		
		uint32_t packetSize = it;

		Ptr<Node> n2 = CreateObject<Node> ();
		Ptr<Node> r1 = CreateObject<Node> ();
		Ptr<Node> r2 = CreateObject<Node> ();
		Ptr<Node> n3 = CreateObject<Node> ();

		NodeContainer n2r1 = NodeContainer(n2,r1);
		NodeContainer r1r2 = NodeContainer(r1,r2);
		NodeContainer r2n3 = NodeContainer(r2,n3);
		NodeContainer all = NodeContainer(n2,r1,r2,n3);


		PointToPointHelper p2p_n2r1;
		p2p_n2r1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		p2p_n2r1.SetChannelAttribute("Delay", StringValue("20ms"));
		NetDeviceContainer device_n2r1;
		device_n2r1 = p2p_n2r1.Install(n2r1);

		PointToPointHelper p2p_r1r2;

		p2p_r1r2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
		p2p_r1r2.SetChannelAttribute("Delay", StringValue("50ms"));
		NetDeviceContainer device_r1r2;
		device_r1r2 = p2p_r1r2.Install(r1r2);

		PointToPointHelper p2p_r2n3;
		p2p_r2n3.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		p2p_r2n3.SetChannelAttribute("Delay", StringValue("20ms"));
		NetDeviceContainer device_r2n3;
		device_r2n3 = p2p_r2n3.Install(r2n3);


		InternetStackHelper stack;
		stack.Install(all);

		Ipv4AddressHelper ipaddress;
		ipaddress.SetBase("192.168.1.0", "255.255.255.0");
		Ipv4InterfaceContainer Ipinterface_n2r1 = ipaddress.Assign(device_n2r1);
		
		ipaddress.SetBase("192.168.2.0", "255.255.255.0");
		Ipv4InterfaceContainer Ipinterface_r1r2 = ipaddress.Assign(device_r1r2);
		
		ipaddress.SetBase("192.168.3.0", "255.255.255.0");
		Ipv4InterfaceContainer Ipinterface_r2n3 = ipaddress.Assign(device_r2n3);

		if(flag)
		{
			std::cout << "r1 ipaddr: " << Ipinterface_r1r2.GetAddress(0) << "  r2 ipaddr: " << Ipinterface_r1r2.GetAddress(1) << std::endl;
			std::cout << "n2 ipaddr: " << Ipinterface_n2r1.GetAddress(0) << "  r1 ipaddr: " << Ipinterface_n2r1.GetAddress(1) << std::endl;
			std::cout << "n3 ipaddr: " << Ipinterface_r2n3.GetAddress(0) << "  n3 ipaddr: " << Ipinterface_r2n3.GetAddress(1) << std::endl;
			flag = 0;
		}
		std:: cout<<"Packet size: " <<it<<std::endl;


		Ipv4GlobalRoutingHelper routingHelper;
		routingHelper.PopulateRoutingTables();

		uint16_t port = 4200;
		Address sinkAddress(InetSocketAddress(Ipinterface_r2n3.GetAddress(1),port));
		PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);

		ApplicationContainer sinkApp = sinkHelper.Install(n3);
		sinkApp.Start(Seconds(1.0));
		sinkApp.Stop(Seconds(20.0));

		std::vector<std::string> tcptypes = {"ns3::TcpVegas", "ns3::TcpVeno", "ns3::TcpWestwoodPlus"};
			Ptr<WiredTCP> app = CreateObject<WiredTCP> ();
			TypeId tid = TypeId::LookupByName(tcptypes[1]); // Select TCP type
			std::stringstream nodeId;
			nodeId << n2r1.Get(0)->GetId();
			std::string specificNode = "/NodeList/"+nodeId.str()+"/$ns3::TcpL4Protocol/SocketType";
			Config::Set (specificNode, TypeIdValue (tid));
			Ptr<Socket> tcpSocket = Socket::CreateSocket(n2r1.Get(0),TcpSocketFactory::GetTypeId());
			app->Setup(tcpSocket,sinkAddress,packetSize,nPackets,DataRate("100Mbps"));
			n2r1.Get (0)->AddApplication (app);
			app->SetStartTime(Seconds(1.0));
			app->SetStopTime(Seconds(20.0));
	



	// Flow monitor to collect stats
		FlowMonitorHelper flowmonitor;
	    Ptr<FlowMonitor> monitor = flowmonitor.InstallAll();
	    

	//Simulation
	    NS_LOG_INFO("Run Simulation");
	    Simulator::Stop (Seconds(25.0));
	    Simulator::Run ();
		
	    monitor->CheckForLostPackets();
	    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonitor.GetClassifier ());
	    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    	double Sumx = 0, SumSqx = 0;
    	int numberofflows=0;

    	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
	      	numberofflows++;
	      
	      	double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();

	      	double throughput = ((i->second.rxBytes * 8.0) / time)/1024;
	      	std::cout << "  Throughput observed for flow "<<i->first<<" : " << throughput  << " Kbps\n\n";

	      	Sumx += throughput;
	      	SumSqx += throughput * throughput ;
    	}

    	double FairnessIndex = (Sumx * Sumx)/ (numberofflows * SumSqx) ;
    	std :: cout << "Average Throughput: " << Sumx/numberofflows << " Kbps" << std::endl;
		
    	std :: cout << "FairnessIndex:	" << FairnessIndex << std :: endl << std::endl;
    	Simulator::Destroy ();
    	NS_LOG_INFO ("Complete");
	}

	return 0;
}
// constructor
WiredTCP::WiredTCP ()
 : socket (0), 
    peer (), 
    packetSize (0), 
    nPackets (0), 
    dataRate (0), 
    sendEvent (), 
    isRunning (false), 
    packetsSent (0)
{
}

WiredTCP::~WiredTCP()
{
  socket = 0;
}

void WiredTCP::Setup(Ptr<Socket> m_socket, Address m_address, uint32_t m_packetSize, uint32_t m_nPackets, DataRate m_datarate){
	socket = m_socket;
	peer = m_address;
	packetSize = m_packetSize;
	nPackets = m_nPackets;
	dataRate = m_datarate;
}

void WiredTCP::StartApplication() {
	isRunning = true;
	packetsSent = 0;
	socket->Bind();
	socket->Connect(peer);
	SendPacket();
}

void WiredTCP::StopApplication() {
	isRunning = false;

	if(sendEvent.IsRunning()){
		Simulator::Cancel(sendEvent);
	}
	if(socket){
		socket->Close();
	}
}

void WiredTCP::SendPacket() {
	Ptr<Packet> packet = Create<Packet>(packetSize);
	socket->Send(packet);

	if(++packetsSent < nPackets){
		ScheduleTx();
	}
}

void 
WiredTCP::ScheduleTx (void)
{
  if (isRunning)
    {
      Time tNext (Seconds (packetSize * 8 / static_cast<double> (dataRate.GetBitRate ())));
      sendEvent = Simulator::Schedule (tNext, &WiredTCP::SendPacket, this);
    }
}
