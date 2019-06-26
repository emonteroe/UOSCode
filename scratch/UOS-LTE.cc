/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*      Copyright (c) 2019
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Emanuel Montero Espaillat <emanuel.montero.e@gmail.com>
 */

#include <fstream>
#include <string.h>
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <memory>
#include "ns3/double.h"
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include "ns3/gnuplot.h" //gnuplot

#include "ns3/csma-helper.h"
#include "ns3/evalvid-client-server-helper.h"
#include "ns3/netanim-module.h"

#include "ns3/lte-module.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"

//#include "ns3/gtk-config-store.h"
#include "ns3/onoff-application.h"
#include "ns3/on-off-helper.h"
 #include <ns3/lte-ue-net-device.h>
 #include <ns3/lte-ue-phy.h>
 #include <ns3/lte-ue-rrc.h>
// #include <lte-test-ue-measurements.h>
#include "ns3/flow-monitor-module.h"

#include <math.h>

using namespace ns3;

const uint16_t numberOfeNodeBNodes = 10;
const uint16_t numberOfUENodes = 150;
const uint16_t numberOfUABS = 0;
double simTime = 140;
const double m_distance = 2000; //m_distance between enBs towers.
// Inter packet interval in ms
// double interPacketInterval = 1;
// double interPacketInterval = 100;
// uint16_t port = 8000;
int evalvidId = 0;      
int eNodeBTxPower = 43; //Set enodeB Power
int UABSTxPower =0; // 33;   //Set UABS Power
uint8_t bandwidth = 100; // 100 RB --> 20MHz  |  25 RB --> 5MHz
double speedUABS = 0;//10;
double ue_info[numberOfeNodeBNodes + numberOfUABS][numberOfUENodes]; //UE Connection Status Register Matrix
double ue_imsi_sinr[numberOfUENodes]; //UE Connection Status Register Matrix
int minSINR = 0;
string GetClusterCoordinates;
std::stringstream cmd;
double Throughput=0.0;


	/**
	 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
	 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
	 * It also  starts yet another flow between each UE pair.
	 */
	 
	NS_LOG_COMPONENT_DEFINE ("UOSLTE");



	void NotifyMeasureMentReport (string context, uint64_t imsi, uint16_t cellid, uint16_t rnti, LteRrcSap::MeasurementReport msg)
	{

	//std::cout<< Simulator::Now().GetSeconds() <<" User: "<< imsi << " CellId=" << cellid << " RSRQ=" << ns3::EutranMeasurementMapping::RsrqRange2Db((uint16_t) msg.measResults.rsrqResult)<<" RSRP=" << ns3::EutranMeasurementMapping::RsrpRange2Dbm( (uint16_t) msg.measResults.rsrpResult)<< " RNTI: "<< rnti << " Neighbor Cells: " << msg.measResults.haveMeasResultNeighCells <<endl;

	//double test = -90;
	//double compare = ns3::EutranMeasurementMapping::RsrpRange2Dbm( (uint16_t) msg.measResults.rsrpResult);
	//if (compare  <= test){

	//NS_LOG_UNCOND("User can do handover to UABS");


	//}

	}

	/* void ns3::PhyStatsCalculator::ReportCurrentCellRsrpSinrCallback(Ptr< PhyStatsCalculator > phyStats, std::string path, uint16_t cellId, uint16_t rnti, double rsrp, double sinr,uint8_t componentCarrierId )
	{

	std::cout<< "Time: "<< Simulator::Now().GetSeconds() << " SINR: " << 10*log(sinr) << " Cell ID:"<< cellId <<" RSRP: "<< 10*log(1000*rsrp) << endl;


	}
	*/
	void ns3::PhyStatsCalculator::ReportUeSinr(uint16_t cellId, uint64_t imsi, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
	{
	double sinrdB = 10 * log(sinrLinear); 
	//feed UE_info with actual SINR in dB.
	ue_info[cellId-1][imsi-1] = sinrdB;
	ue_imsi_sinr[imsi-1]=sinrdB; 
	//std::cout << "Sinr: " << ue_imsi[imsi-1] <<" Sinr Linear: "<< sinrLinear << " Imsi: "<< imsi << " CellId: " << cellId << " rnti: "<< rnti << endl;

	}


	void GetPositionUEandenB(NodeContainer ueNodes, NodeContainer enbNodes, NodeContainer UABSNodes, NetDeviceContainer enbLteDevs)
	{
	 // iterate our nodes and print their position.
		
				std::stringstream enodeB;
				enodeB << "enBs"; 
				std::stringstream uenodes;
				uenodes << "LTEUEs";
				std::stringstream UABSnod;
				UABSnod << "UABSs";
				std::ofstream enB;
				enB.open(enodeB.str());    
				std::ofstream UE;
				UE.open(uenodes.str());     
				std::ofstream UABS;
				UABS.open(UABSnod.str());   
				uint16_t enBCellId;
				int i=0; 

	 
		for (NodeContainer::Iterator j = enbNodes.Begin ();j != enbNodes.End (); ++j)
			{
				/*  std::stringstream enodeB;
				enodeB << "enB_" << *j;
	 
				std::ofstream enB;
				enB.open(enodeB.str());*/
				enBCellId = enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetCellId();     
				Ptr<Node> object = *j;
				Ptr<MobilityModel> enBposition = object->GetObject<MobilityModel> ();
				//Ptr<LteEnbNetDevice> enBCellID = object->GetObject<LteEnbNetDevice> ();
				////enBCellId = enBCellID->GetCellId();
				NS_ASSERT (enBposition != 0);
				Vector pos = enBposition->GetPosition ();
				enB  << pos.x << "," << pos.y << "," << pos.z <<"," << enBCellId <<std::endl;
				//enB.close();
				i++;
			}
				enB.close();


		for (NodeContainer::Iterator j = ueNodes.Begin ();j != ueNodes.End (); ++j)
			{
				/*  std::stringstream uenodes;
				uenodes << "UE_" << *j;
	 
				std::ofstream UE;
				UE.open(uenodes.str());*/     
				Ptr<Node> object = *j;
				Ptr<MobilityModel> UEposition = object->GetObject<MobilityModel> ();
				NS_ASSERT (UEposition != 0);
				Vector pos = UEposition->GetPosition ();
				UE << pos.x << "," << pos.y << "," << pos.z << std::endl;
				//UE.close();
			}
				UE.close();


		for (NodeContainer::Iterator j = UABSNodes.Begin ();j != UABSNodes.End (); ++j)
			{
				/*  std::stringstream enodeB;
				enodeB << "enB_" << *j;
	 
				std::ofstream enB;
				enB.open(enodeB.str());*/     
				Ptr<Node> object = *j;
				Ptr<MobilityModel> UABSposition = object->GetObject<MobilityModel> ();
				NS_ASSERT (UABSposition != 0);
				Vector pos = UABSposition->GetPosition ();
				UABS << pos.x << "," << pos.y << "," << pos.z << std::endl;
				//enB.close();
			}

				UABS.close();


	 Simulator::Schedule(Seconds(5), &GetPositionUEandenB,ueNodes, enbNodes, UABSNodes, enbLteDevs);
	}

	  //Get SINR of UEs and Positions
	void GetSinrUE (NetDeviceContainer ueLteDevs, NodeContainer ueNodes)
	{  
				uint64_t UEPhy;
				std::stringstream uenodes;
				uenodes << "UEsLowSinr";    
				std::ofstream UE;
				UE.open(uenodes.str());
				NodeContainer::Iterator j = ueNodes.Begin();

		for(uint16_t i = 0; i < ueLteDevs.GetN(); i++){
			UEPhy = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
				if (ue_imsi_sinr[UEPhy] < minSINR) // revisar aqui si tengo que poner uephy-1 (imsi-1)
					{	
						//NS_LOG_UNCOND("Sinr: "<< ue_imsi_sinr[UEPhy] << " Imsi: " << UEPhy );
						//UE << "Sinr: "<< ue_imsi_sinr[UEPhy] << " Imsi: " << UEPhy << std::endl;
						
						Ptr<Node> object = *j;
						Ptr<MobilityModel> UEposition = object->GetObject<MobilityModel> ();
						NS_ASSERT (UEposition != 0);
						Vector pos = UEposition->GetPosition ();
						UE << pos.x << "," << pos.y << "," << pos.z << "," << ue_imsi_sinr[UEPhy] << ","<< UEPhy<< std::endl;
						++j;
						
					}
			}
		UE.close();
		Simulator::Schedule(Seconds(5), &GetSinrUE,ueLteDevs,ueNodes);
		}


	void SetTXPowerPositionAndVelocityUABS(NodeContainer UABSNodes, double speedUABS, NetDeviceContainer UABSLteDevs)
	{
		Ptr<LteEnbPhy> UABSPhy;
	 
		//Turn on or off UABS TX Power:
		for( uint16_t i = 0; i < UABSLteDevs.GetN(); i++) {
			UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
			UABSPhy->SetTxPower(UABSTxPower);
		}

		//Set Position of UABS / or trajectory to go to assist a low SINR Area:




		//Set Velocity of UABS to start moving:
		 for (uint16_t n=0 ; n < UABSNodes.GetN(); n++)
			{
				Ptr<ConstantVelocityMobilityModel> VelUABS = UABSNodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
				VelUABS->SetVelocity(Vector(speedUABS, speedUABS, 0));
				//NS_LOG_UNCOND (VelUABS->GetVelocity());
			}

	}


		std::string exec(const char* cmd)
		{
    		std::array<char, 128> buffer;
    		std::string result;
    		std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    		if (!pipe)
        		throw std::runtime_error("popen() failed!");
    		while (!feof(pipe.get())) {
        		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            		result += buffer.data();
    		}
    		return result;
		}


	void GetPrioritizedClusters()
	{
		cmd << "python3 UOS-PythonCode.py " << " 2>/dev/null";
		//GetClusterCoordinates =  stod(exec(cmd.str().c_str()));
		GetClusterCoordinates =  exec(cmd.str().c_str());
		NS_LOG_UNCOND("Coordinates of prioritized Clusters: " + GetClusterCoordinates);
	}

	void ThroughputCalc(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier,Gnuplot2dDataset datasetThroughput)
	{

		monitor->CheckForLostPackets ();
		//Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon->GetClassifier ());
		std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

	  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
		{
		  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

		  //if ((t.sourceAddress == Ipv4Address("10.1.1.1") && t.destinationAddress == Ipv4Address("10.1.1.2"))
		   // || (t.sourceAddress == Ipv4Address("10.1.1.3") && t.destinationAddress == Ipv4Address("10.1.1.2")))
		   // {
			 /* NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
			  NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
			  NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
			  NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");*/

			std::cout<<"Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress<<"\n";
			std::cout<<"Tx Packets = " << iter->second.txPackets<<"\n";
			std::cout<<"Rx Packets = " << iter->second.rxPackets<<"\n";
			std::cout<<"Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) /1024  << " Mbps\n";
			Throughput=iter->second.rxBytes * 8.0 /(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/ 1024;
			datasetThroughput.Add((double)iter->first,(double) Throughput);
			//}
		}

		monitor->SerializeToXmlFile("UOSLTE-FlowMonitor.flowmon",true,true);


	}



	// MAIN FUNCTION

	int
	main (int argc, char *argv[])
	{
	 // LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
	 // LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);



	  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
	  //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
	  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
	  lteHelper->SetEpcHelper (epcHelper);  //Evolved Packet Core (EPC)
	  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler"); // Scheduler es para asignar los recursos un UE va a tener  (cuales UE deben tener recursos y cuanto)
	  //PfFfMacScheduler --> es un proportional fair scheduler
	  Ptr<Node> pgw = epcHelper->GetPgwNode ();
	  
	  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
	  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));
	  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth)); //Set Download BandWidth
	  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth)); //Set Upload Bandwidth

	  
	  //Set Handover algorithm 
	  //lteHelper->SetHandoverAlgorithmType("ns3::AhpHandoverAlgorithm");
	  lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
	  lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis",
											  DoubleValue (3.0));
	  lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger",
											  TimeValue (MilliSeconds (256)));
	 
	  // Create a single RemoteHost
	  NodeContainer remoteHostContainer;
	  remoteHostContainer.Create (1);
	  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
	  InternetStackHelper internet;
	  internet.Install (remoteHost);
	  

	  // Create the Internet
	  PointToPointHelper p2ph;
	  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
	  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
	  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));   //0.010
	  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);


	  Ipv4AddressHelper ipv4h;
	  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
	  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
	  // interface 0 is localhost, 1 is the p2p device
	  // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

	  Ipv4StaticRoutingHelper ipv4RoutingHelper;
	  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
	  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
	  
	  // Create node containers: UE, eNodeBs, UABSs.
	  NodeContainer ueNodes;
	  NodeContainer enbNodes;
	  NodeContainer UABSNodes;
	  enbNodes.Create(numberOfeNodeBNodes);
	  ueNodes.Create(numberOfUENodes);
	  UABSNodes.Create(numberOfUABS);

	NS_LOG_UNCOND("Installing Mobility Model in enBs...");

	  // Install Mobility Model eNodeBs
	  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
		
	  for (uint16_t i = 0; i < numberOfeNodeBNodes; i++)
		{
		  //positionAlloc->Add (Vector(m_distance * i, 0, 0));
		  positionAlloc->Add (Vector(m_distance * i, m_distance, 30));
		}  

	  MobilityHelper mobility;
	  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	 // mobility.SetPositionAllocator(positionAlloc);
	  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
									   "MinX", DoubleValue (0.0),
									   "MinY", DoubleValue (0.0),
									   "DeltaX", DoubleValue (m_distance),
									   "DeltaY", DoubleValue (m_distance),
									   "GridWidth", UintegerValue (3),
									   "LayoutType", StringValue ("RowFirst"));

	  mobility.Install(enbNodes);


	   
	NS_LOG_UNCOND("Installing Mobility Model in UEs...");
	  // Install Mobility Model User Equipments
	  MobilityHelper mobility2;
	  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
								 "Mode", StringValue ("Time"),
								 "Time", StringValue ("1s"),
								 //"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=4.0]"),
								 "Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
						 "Bounds", StringValue ("0|6000|0|6000"));
	  mobility2.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
			  "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"),
			  "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"));
	  mobility2.Install(ueNodes);
	  
	  
	NS_LOG_UNCOND("Installing Mobility Model in UABSs...");
	  // Install Mobility Model UABS
	  MobilityHelper mobility3;
	  mobility3.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
	  //mobility3.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
		//                         "Mode", StringValue ("Time"),
		  //                       "Time", StringValue ("5s"),
								 //"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
			//                     "Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
			//		     "Bounds", StringValue ("0|2000|0|2000"));
		mobility3.SetPositionAllocator ("ns3::GridPositionAllocator",
										"MinX", DoubleValue (0.0),
										"MinY", DoubleValue (0.0),
										"DeltaX", DoubleValue (m_distance),
										"DeltaY", DoubleValue (m_distance),
										"GridWidth", UintegerValue (3),
										"LayoutType", StringValue ("RowFirst"));

		mobility3.Install(UABSNodes);

	  

	  // Install LTE Devices to the nodes
	  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
	  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
	  NetDeviceContainer UABSLteDevs = lteHelper->InstallEnbDevice (UABSNodes);

	  // Get position of enBs, UABSs and UEs.
	  Simulator::Schedule(Seconds(5), &GetPositionUEandenB,ueNodes, enbNodes,UABSNodes,enbLteDevs);
	  

		//Set Power of eNodeBs  
		Ptr<LteEnbPhy> enodeBPhy; 

		for( uint16_t i = 0; i < enbLteDevs.GetN(); i++) {
			enodeBPhy = enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
			enodeBPhy->SetTxPower(eNodeBTxPower);
		}

		//Set TXPower, Velocity and position to UABSs needed to enhance the throughput
		//if ( an UABS is needed == 1) {
		SetTXPowerPositionAndVelocityUABS(UABSNodes, speedUABS, UABSLteDevs); 
		//}

	  //Get Power of eNodeBs and UABSs
	/*  	Ptr<LteUePhy> UEPhy;
		for(uint16_t i = 0; i < ueLteDevs.GetN(); i++)
		{
			UEPhy = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetPhy();
			//UEPhy->GetTxPower();
			//UEPhy->GenerateCqiRsrpRsrq();
			//NS_LOG_UNCOND(UEPhy->GetNoiseFigure());        
			//NS_LOG_UNCOND (UEPhy->GetPhy());
			
			 if (UEPhy->GetTxPower() == 10)
			{
				NS_LOG_UNCOND("Test " << i);
			}
		}
	*/

	 

	 
	  // Install the IP stack on the UEs
	  internet.Install (ueNodes);
	  Ipv4InterfaceContainer ueIpIface;
	  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
	  
	  // Assign IP address to UEs, and install applications
	  for (uint16_t i = 0; i < ueNodes.GetN(); i++) {
		Ptr<Node> ueNode = ueNodes.Get (i);
			// Set the default gateway for the UE
			Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
			ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
	  }


	NS_LOG_UNCOND("Attaching Ues in enBs/UABSs...");
	  // Attach one UE per eNodeB // ahora no es un UE por eNodeB, es cualquier UE a cualquier eNodeB
	  lteHelper->AttachToClosestEnb (ueLteDevs, enbLteDevs);
	  //lteHelper->Attach (ueLteDevs);

	  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
	  
	  
	//get Sinr
	 Simulator::Schedule(Seconds(5), &GetSinrUE,ueLteDevs,ueNodes);


	//Run Python Command to get centroids
	GetPrioritizedClusters();

	NS_LOG_UNCOND("Resquesting-sending Video...");
	  NS_LOG_INFO ("Create Applications.");
	   for (uint16_t i = 0; i < ueNodes.GetN(); i++) {
		evalvidId++;
		uint16_t  port = 8000 * evalvidId + 8000; //to use a different port in every iterac...


		//Video Server
		EvalvidServerHelper server(port);
		server.SetAttribute ("SenderTraceFilename", StringValue("src/evalvid/st_highway_cif.st"));
		server.SetAttribute ("SenderDumpFilename", StringValue("src/evalvid/sd_a01_lte"));
		ApplicationContainer apps = server.Install(remoteHost);//Container.Get(0)); //verificar esto
		apps.Start (Seconds (9.0));
		apps.Stop (Seconds (simTime));

		// Clients
		EvalvidClientHelper client (internetIpIfaces.GetAddress (1),port);
	  
		stringstream s;
		s << "rd_a" << i << "_lte";

		client.SetAttribute ("ReceiverDumpFilename", StringValue(s.str()));
		apps = client.Install (ueNodes.Get(i));{
		
		 }
		
		apps.Start (Seconds (10.0));
		apps.Stop (Seconds (simTime));

		Ptr<Ipv4> ipv4 = ueNodes.Get(i)->GetObject<Ipv4>();
	  }	

	  AnimationInterface anim ("UOSLTE.xml"); // Mandatory
	  anim.SetMaxPktsPerTraceFile(500000); // Set animation interface max packets. (TO CHECK: how many packets i will be sending?) 
	  // Cor e Descrição para eNb
		for (uint32_t i = 0; i < enbNodes.GetN(); ++i) {
			anim.UpdateNodeDescription(enbNodes.Get(i), "eNb");
			anim.UpdateNodeColor(enbNodes.Get(i), 0, 255, 0);
		anim.UpdateNodeSize(i,100,100); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < ueNodes.GetN(); ++i) {
			anim.UpdateNodeDescription(ueNodes.Get(i), "UEs");
			anim.UpdateNodeColor(ueNodes.Get(i),  255, 0, 0);
		anim.UpdateNodeSize(i,50,50); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < UABSNodes.GetN(); ++i) {
			anim.UpdateNodeDescription(UABSNodes.Get(i), "UABS");
			anim.UpdateNodeColor(UABSNodes.Get(i), 0, 0, 255);
		anim.UpdateNodeSize(i,100,100); // to change the node size in the animation.
		}
			anim.UpdateNodeDescription(remoteHost, "RH");
			anim.UpdateNodeColor(remoteHost, 0, 255, 255);
		//anim.UpdateNodeSize(remoteHost,100,100); // to change the node size in the animation.
	 



	 
	  //lteHelper->EnableTraces (); Set the traces on or off. 
	  
	NS_LOG_UNCOND("Running simulation...");
	NS_LOG_INFO ("Run Simulation.");
	  
		lteHelper->EnablePhyTraces();
		lteHelper->EnableUlPhyTraces();
		lteHelper->EnableMacTraces();
		lteHelper->EnableRlcTraces();
		lteHelper->EnablePdcpTraces();
		  /*--------------NOTIFICAÇÕES DE UE Mesasurements-------------------------*/
	   Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/RecvMeasurementReport", MakeCallback (&NotifyMeasureMentReport)); 
	  // Config::Connect ("/NodeList/*/DeviceList/*/LteUePhy/ReportCurrentCellRsrpSinr",MakeCallback (&ns3::PhyStatsCalculator::ReportCurrentCellRsrpSinrCallback));
	Config::Connect ("/NodeList/*/DeviceList/*/LteUePhy/ReportUeSinr",MakeCallback (&ns3::PhyStatsCalculator::ReportUeSinr));



		//Gnuplot parameters
		string fileNameWithNoExtension = "FlowVSThroughput";
		string graphicsFileName        = fileNameWithNoExtension + ".png";
		string plotFileName            = fileNameWithNoExtension + ".plt";
		string plotTitle               = "Flow vs Throughput";
		string dataTitle               = "Throughput";

		// Instantiate the plot and set its title.
		Gnuplot gnuplot (graphicsFileName);
		gnuplot.SetTitle (plotTitle);

		// Make the graphics file, which the plot file will be when it is used with Gnuplot, be a PNG file.
		gnuplot.SetTerminal ("png");

		// Set the labels for each axis.
		gnuplot.SetLegend ("Flow", "Throughput");

		Gnuplot2dDataset datasetThroughput;
		datasetThroughput.SetTitle (dataTitle);
		datasetThroughput.SetStyle (Gnuplot2dDataset::LINES_POINTS);

		//Flow Monitor Setup
		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll();
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
		
		Simulator::Stop(Seconds(simTime));
		Simulator::Run ();

		// Print per flow statistics
		ThroughputCalc(monitor,classifier,datasetThroughput);

		//Gnuplot ...continued
 
		gnuplot.AddDataset (datasetThroughput);

		// Open the plot file.
		ofstream plotFile (plotFileName.c_str());

		// Write the plot file.
		gnuplot.GenerateOutput (plotFile);

		// Close the plot file.
		plotFile.close ();

		
		Simulator::Destroy ();
	  
		   NS_LOG_INFO ("Done.");
	  return 0;

	}
