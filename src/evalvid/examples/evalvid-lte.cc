/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Billy Pinheiro <haquiticos@gmail.com>
 */

#include <fstream>
#include <string.h>

#include "ns3/csma-helper.h"
#include "ns3/evalvid-client-server-helper.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
 
NS_LOG_COMPONENT_DEFINE ("EvalvidLTEExample");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
  LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);


  uint16_t numberOfeNodeBNodes = 3;
  uint16_t numberOfUENodes = 6;
  // double simTime = 5.0;
  double distance = 60.0;
  // Inter packet interval in ms
  // double interPacketInterval = 1;
  // double interPacketInterval = 100;
   uint16_t port = 8000;


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);  //Evolved Packet Core (EPC)
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler"); // Scheduler es para asignar los recursos un UE va a tener  (cuales UE deben tener recursos y cuanto)
  //PfFfMacScheduler --> es un proportional fair scheduler
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));


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

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfeNodeBNodes);
  ueNodes.Create(numberOfUENodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> (); //Modified
  for (uint16_t i = 0; i < numberOfeNodeBNodes; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  

  //for (uint16_t i = 0; i < numberOfUENodes; i++)  //Modified
  //  {
  //    positionAlloc2->Add (Vector(distance * i, 0, 0));
 //   }
  

  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);

  MobilityHelper mobility2; //Modified
  //obility2.SetMobilityModel("ns3::RandomWaypointMobilityModel"); //Modified
  //mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  //                           "Mode", StringValue ("Time"),
  //                           "Time", StringValue ("2s"),
  //                           "Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
  //                             "Bounds", StringValue ("0|200|0|200"));
  //mobility2.SetPositionAllocator(positionAlloc2); //Modified
  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("2s"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                             "Bounds", StringValue ("0|100|0|100"));

  
  mobility.Install(enbNodes);
  mobility2.Install(ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

 
  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (int i = 0; i < ueNodes.GetN(); i++) {
    Ptr<Node> ueNode = ueNodes.Get (i);
     // Set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  // Attach one UE per eNodeB // ahora no es un UE por eNodeB, es cualquier UE a cualquier eNodeB
  lteHelper->Attach (ueLteDevs);

  //lteHelper->ActivateEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
  
  NS_LOG_INFO ("Create Applications.");
  
  EvalvidServerHelper server(port);
  server.SetAttribute ("SenderTraceFilename", StringValue("src/evalvid/st_highway_cif.st"));
  server.SetAttribute ("SenderDumpFilename", StringValue("src/evalvid/sd_a01_lte"));
  ApplicationContainer apps = server.Install(remoteHostContainer.Get(0));
  apps.Start (Seconds (9.0));
  apps.Stop (Seconds (101.0));
  for (int i = 0; i < ueNodes.GetN(); i++) {
  EvalvidClientHelper client (internetIpIfaces.GetAddress (1),port);
  
  stringstream s;
  s << "rd_a" << i << "_lte";

  client.SetAttribute ("ReceiverDumpFilename", StringValue(s.str()));
  apps = client.Install (ueNodes.Get(i));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (90.0));
}

  AnimationInterface anim ("UOSLTEtest.xml"); // Mandatory

  

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(90));
  Simulator::Run ();         
  Simulator::Destroy ();
  
  
   NS_LOG_INFO ("Done.");
  return 0;

}
