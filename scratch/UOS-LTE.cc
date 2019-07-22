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

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split

using namespace ns3;

const uint16_t numberOfeNodeBNodes = 4;
const uint16_t numberOfUENodes = 80;
const uint16_t numberOfUABS = 6;
double simTime = 300;
const int m_distance = 2000; //m_distance between enBs towers.
// Inter packet interval in ms
// double interPacketInterval = 1;
// double interPacketInterval = 100;
// uint16_t port = 8000;
int evalvidId = 0;      
int eNodeBTxPower = 20; //Set enodeB Power
int UABSTxPower = 0;//33;   //Set UABS Power
uint8_t bandwidth = 100; // 100 RB --> 20MHz  |  25 RB --> 5MHz
//uint8_t bandwidth = 25; // To use with UABS --> tengo que ver si no necesito crear otro LTEhelper solo para los UABS.
double speedUABS = 10;
double ue_info[numberOfeNodeBNodes + numberOfUABS][numberOfUENodes]; //UE Connection Status Register Matrix
double ue_imsi_sinr[numberOfUENodes]; //UE Connection Status Register Matrix
double ue_imsi_sinr_linear[numberOfUENodes];
double ue_info_cellid[numberOfUENodes];
int minSINR = 0;
string GetClusterCoordinates;
double Throughput=0.0;
bool UABSFlag;
bool UABS_On_Flag = false;
uint32_t txPacketsum = 0;
uint32_t rxPacketsum = 0;
uint32_t DropPacketsum = 0;
uint32_t LostPacketsum = 0;
double Delaysum = 0;
std::stringstream cmd;
double UABSHeight = 30.0;
double enBHeight = 30.0;

	 
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

		/*void ns3::PhyStatsCalculator::ReportCurrentCellRsrpSinrCallback(Ptr< PhyStatsCalculator > phyStats, std::string path, uint16_t cellId, uint16_t rnti, double rsrp, double sinr,uint8_t componentCarrierId )
		{
			std::cout<< "Time: "<< Simulator::Now().GetSeconds() << " SINR: " << 10*log(sinr) << " Cell ID:"<< cellId <<" RSRP: "<< 10*log(1000*rsrp) << endl;
		}
		*/

		void ns3::PhyStatsCalculator::ReportUeSinr(uint16_t cellId, uint64_t imsi, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
		{
			double sinrdB = 10 * log(sinrLinear); 
			//feed UE_info with actual SINR in dB.
			//ue_info[cellId-1][imsi-1] = sinrdB;
			ue_info_cellid[imsi-1] = cellId;
			ue_imsi_sinr[imsi-1]=sinrdB; 
			ue_imsi_sinr_linear[imsi-1]=sinrLinear; //To pass SIRN Linear to python code to do the linear sum
			//std::cout << "Sinr: " << ue_imsi_sinr[imsi-1] <<" Sinr Linear: "<< sinrLinear << " Imsi: "<< imsi << " CellId: " << cellId << " rnti: "<< rnti << endl;

		}


		void GetPositionUEandenB(NodeContainer ueNodes, NodeContainer enbNodes, NodeContainer UABSNodes, NetDeviceContainer enbLteDevs,NetDeviceContainer UABSLteDevs, NetDeviceContainer ueLteDevs)
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
			uint16_t UABSCellId;
			Ptr<LteEnbPhy> UABSPhy;
			int i=0; 
			//int j=0;
			int k=0;

	 
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
				UABSCellId = UABSLteDevs.Get(k)->GetObject<LteEnbNetDevice>()->GetCellId(); 
				
				UABSPhy = UABSLteDevs.Get(k)->GetObject<LteEnbNetDevice>()->GetPhy();
				NS_LOG_UNCOND("UABS " << std::to_string(k) << " TX Power: ");
				NS_LOG_UNCOND(UABSPhy->GetTxPower());
				
				Ptr<Node> object = *j;
				Ptr<MobilityModel> UABSposition = object->GetObject<MobilityModel> ();
				NS_ASSERT (UABSposition != 0);
				Vector pos = UABSposition->GetPosition ();
				UABS << pos.x << "," << pos.y << "," << pos.z <<"," << UABSCellId << std::endl;
				//enB.close();
				k++;
			}

			UABS.close();

			Simulator::Schedule(Seconds(5), &GetPositionUEandenB,ueNodes, enbNodes, UABSNodes, enbLteDevs, UABSLteDevs, ueLteDevs );
		}

	  //Get SINR of UEs and Positions
		void GetSinrUE (NetDeviceContainer ueLteDevs, NodeContainer ueNodes)
		{  
			uint64_t UEImsi;
			std::stringstream uenodes;
			uenodes << "UEsLowSinr";    
			std::ofstream UE;
			UE.open(uenodes.str());
			NodeContainer::Iterator j = ueNodes.Begin();
			int k =0;

			for(uint16_t i = 0; i < ueLteDevs.GetN(); i++)
			{
				UEImsi = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
					if (ue_imsi_sinr[UEImsi-1] < minSINR) // revisar aqui si tengo que poner uephy-1 (imsi-1)
					{	
						//NS_LOG_UNCOND("Sinr: "<< ue_imsi_sinr[UEImsi] << " Imsi: " << UEImsi );
						//UE << "Sinr: "<< ue_imsi_sinr[UEImsi] << " Imsi: " << UEImsi << std::endl;
						
						Ptr<Node> object = *j;
						Ptr<MobilityModel> UEposition = object->GetObject<MobilityModel> ();
						NS_ASSERT (UEposition != 0);
						Vector pos = UEposition->GetPosition ();
						UE << pos.x << "," << pos.y << "," << pos.z << "," << ue_imsi_sinr_linear[UEImsi-1] << ","<< UEImsi<< "," << ue_info_cellid[UEImsi-1]<< std::endl;
						++j;
						++k;
						
					}
			}
			NS_LOG_UNCOND("Users with low sinr: "); //To know if after an UABS is functioning this number decreases.
			NS_LOG_UNCOND(k);

			UE.close();
			Simulator::Schedule(Seconds(5), &GetSinrUE,ueLteDevs,ueNodes);
		}


		void SetTXPowerPositionAndVelocityUABS(NodeContainer UABSNodes, double speedUABS, NetDeviceContainer UABSLteDevs, std::vector<ns3::Vector3D> CoorPriorities_Vector)
		{
			Ptr<LteEnbPhy> UABSPhy;
			// uint16_t UABSCellId;

			//Necesito hacer un filtro de que cuando CoorPriorities_Vector venga con las coordenadas y cual UABS va pa X coordenada buscar cual UABS tiene cual CellID y entonces colocar ese UABS en la coordenada.

			//UABSCellId = UABSLteDevs.Get(k)->GetObject<LteEnbNetDevice>()->GetCellId();
	 
			//---------Turn on or off UABS TX Power:
			//If el UABS_On_Flag o el UABSFlag esta True, set la potencia, de lo contrario mantenla en 0 (apagado).
			if (UABSFlag == true && UABS_On_Flag == false) // revisar esta logica
			{
				UABSTxPower = 10;
				//---------Turn On UABS Power
				for( uint16_t i = 0; i < UABSLteDevs.GetN(); i++) 
				{
					UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
					UABSPhy->SetTxPower(UABSTxPower);
					// NS_LOG_UNCOND("UABS TX Power: ");
					// NS_LOG_UNCOND(UABSPhy->GetTxPower());
					//lteHelper->AttachToClosestEnb (ueLteDevs, UABSLteDevs);
					
				}

				//-----------Set Velocity of UABS to start moving:
				//If  si el UABSFlag esta True, setea la velocidad, de lo contrario si UABS_On_Flag esta True mantenla en 0 (apagado).
				for (uint16_t n=0 ; n < UABSNodes.GetN(); n++)
				{
					Ptr<ConstantVelocityMobilityModel> VelUABS = UABSNodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
					VelUABS->SetVelocity(Vector(speedUABS, 0,0));//speedUABS, 0));
					//NS_LOG_UNCOND (VelUABS->GetVelocity());
				}
				UABS_On_Flag = true;
			}
			else if (UABSFlag == false && UABS_On_Flag == false) 
			{

				UABSTxPower = 0;
				//Turn off UABS Power
				for( uint16_t i = 0; i < UABSLteDevs.GetN(); i++) 
				{
					UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
					UABSPhy->SetTxPower(UABSTxPower);
					// NS_LOG_UNCOND("UABS TX Power: ");
					// NS_LOG_UNCOND(UABSPhy->GetTxPower());
					//lteHelper->AttachToClosestEnb (ueLteDevs, UABSLteDevs);
				}

				//Set Velocity of UABS to stop moving:
				
				for (uint16_t n=0 ; n < UABSNodes.GetN(); n++)
				{
					Ptr<ConstantVelocityMobilityModel> VelUABS = UABSNodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
					VelUABS->SetVelocity(Vector(0, 0,0));
					//NS_LOG_UNCOND (VelUABS->GetVelocity());
				}

			}

		//Set Position of UABS / or trajectory to go to assist a low SINR Area:
		//If la posicion cambio y el UABS_On_Flag esta True, setea la nueva posicion.

			//To do: here i have to receive a variable saying which UABS is going to X position.
		for (uint16_t k=0 ; k < UABSNodes.GetN(); k++)
		{
			Ptr<ConstantVelocityMobilityModel> PosUABS = UABSNodes.Get(k)->GetObject<ConstantVelocityMobilityModel>();
			//PosUABS->SetPosition(Vector(10, 10, 40)); // aqui tengo que poner las coordenadas obtenidas por el algoritmo de clusterizacion.
			PosUABS->SetPosition(CoorPriorities_Vector.at(k));
			//NS_LOG_UNCOND("Temp: ");
			//NS_LOG_UNCOND(CoorPriorities_Vector.at(k));
			//NS_LOG_UNCOND("GetPosition: ");
			NS_LOG_UNCOND (PosUABS->GetPosition());
		}

		}

		// ---------------Function to run the Python Command in order to receive the prioritized locations -------------//
		std::string exec(const char* cmd)
		{
			std::array<char, 128> buffer;
			std::string result;
			std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
			if (!pipe)
				throw std::runtime_error("popen() failed!");
			while (!feof(pipe.get())) 
			{
				if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
				result += buffer.data();
			}
			return result;
		}

		// ---------------Function to receive the prioritized locations into an Vector 3D in order to later call the SetTXPowerPositionAndVelocityUABS function. -----------//
		void GetPrioritizedClusters(NodeContainer UABSNodes, double speedUABS, NetDeviceContainer UABSLteDevs)
		{
			//std::stringstream cmd;
			std::vector<std::string> Split_coord_Prior;
			
			cmd << "python3 UOS-PythonCode.py " << " 2>/dev/null ";
			//GetClusterCoordinates =  stod(exec(cmd.str().c_str()));
			GetClusterCoordinates =  exec(cmd.str().c_str());
			NS_LOG_UNCOND("Coordinates of prioritized Clusters: " + GetClusterCoordinates);
			ns3::Vector3D CoorPriorities;
			std::vector<ns3::Vector3D>  CoorPriorities_Vector;
			
			int j=0;
			if (!GetClusterCoordinates.empty())
			{
				UABSFlag = true;
							
				boost::split(Split_coord_Prior, GetClusterCoordinates, boost::is_any_of(" "), boost::token_compress_on);
				
				for (uint16_t i = 0; i < Split_coord_Prior.size()-1; i+=2)
				{
					//NS_LOG_UNCOND(Split_coord_Prior[i]);
					//NS_LOG_UNCOND(Split_coord_Prior[i] << "," << Split_coord_Prior[i+1]<< ", 40"<<std::endl);
					CoorPriorities = Vector(std::stod(Split_coord_Prior[i]),std::stod(Split_coord_Prior[i+1]),UABSHeight);
					//NS_LOG_UNCOND("Funct GetPrioritizedClusters: ");
					//NS_LOG_UNCOND(CoorPriorities);
					CoorPriorities_Vector.push_back(CoorPriorities); 
					//NS_LOG_UNCOND("Ahi e: ");
					//NS_LOG_UNCOND(CoorPriorities_Vector.at(j));
					j++;
				}
			}
			else 
			{
				UABSFlag = false;
				UABS_On_Flag = false;
			}

			if (UABSFlag == true)
			{
				
				NS_LOG_UNCOND(std::to_string(j) <<" UABS needed: Setting TXPower, Velocity and position");
				SetTXPowerPositionAndVelocityUABS(UABSNodes, speedUABS, UABSLteDevs, CoorPriorities_Vector); 
			}
			
			Simulator::Schedule(Seconds(6), &GetPrioritizedClusters,UABSNodes,  speedUABS,  UABSLteDevs);
		}




		// ------------ Function to calculate the Throughput ---------------//
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
			txPacketsum += iter->second.txPackets;
			rxPacketsum += iter->second.rxPackets;
			LostPacketsum = txPacketsum-rxPacketsum;
			DropPacketsum += iter->second.packetsDropped.size();
			Delaysum += iter->second.delaySum.GetSeconds();

			std::cout<<"Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress<<"\n";
			std::cout<<"Tx Packets = " << iter->second.txPackets<<"\n";
			std::cout<<"Rx Packets = " << iter->second.rxPackets<<"\n";
			//std::cout << "  All Tx Packets: " << txPacketsum << "\n";
			//std::cout << "  All Rx Packets: " << rxPacketsum << "\n";
			//std::cout << "  All Delay: " << Delaysum / txPacketsum << "\n";
			//std::cout << "  All Lost Packets: " << LostPacketsum << "\n";
			//std::cout << "  All Drop Packets: " << DropPacketsum << "\n";
			std::cout<<"Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) /1024  << " Mbps\n";
			std::cout << "Packets Delivery Ratio: " << ((rxPacketsum * 100) / txPacketsum) << "%" << "\n";
			std::cout << "Packets Lost Ratio: " << ((LostPacketsum * 100) / txPacketsum) << "%" << "\n";
			Throughput=iter->second.rxBytes * 8.0 /(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/ 1024;
			datasetThroughput.Add((double)iter->first,(double) Throughput);
			//}
		}

		monitor->SerializeToXmlFile("UOSLTE-FlowMonitor.flowmon",true,true);


		}



		//----------------------MAIN FUNCTION---------------------------//
		int main (int argc, char *argv[])
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

		// set frequency. This is important because it changes the behavior of the path loss model
   		lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(100));
    	lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(18100)); //investigar cual es la frecuencia que voy a utilizar.
   		// lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (200));

		 Config::SetDefault( "ns3::LteUePhy::TxPower", DoubleValue(10) );         // Transmission power in dBm
		 Config::SetDefault( "ns3::LteUePhy::NoiseFigure", DoubleValue(10) );     // Default 5
		// Config::SetDefault( "ns3::LteEnbPhy::TxPower", DoubleValue(40) );        // Transmission power in dBm
		// Config::SetDefault( "ns3::LteEnbPhy::NoiseFigure", DoubleValue(6) );    // Default 5

		lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth)); //Set Download BandWidth
		lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth)); //Set Upload Bandwidth

	  
		//Set Handover algorithm 

		// lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm"); // Handover algorithm implementation based on RSRQ measurements, Event A2 and Event A4.
		// lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
		// lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (2));                                      
		lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm"); // Handover by Reference Signal Reference Power (RSRP)
		lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (256))); //default: 256
		lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue (1.0)); //default: 3.0
		


		 //Antenna parameters  (cuando activo la antena da error de " what():  vector::_M_range_check: __n (which is 4) >= this->size() (which is 4)" )

		// lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");
		// lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue(0));
		// lteHelper->SetEnbAntennaModelAttribute("Beamwidth", DoubleValue(60));
		// lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue(0.0));

		//Pathlossmodel
		//lteHelper->SetAttribute("PathlossModel",StringValue("ns3::NakagamiPropagationLossModel"));
		
		//lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
		
		lteHelper->SetAttribute("PathlossModel",StringValue("ns3::OkumuraHataPropagationLossModel"));
    	lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
    	Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue (Seconds(1.00)));
		
		//lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel")); // it has to be used with building model
		// use always LOS model ( for the HybridBuildingsPropagationModel )
  		//lteHelper->SetPathlossModelAttribute ("Los2NlosThr", DoubleValue (1e6));


	 
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
		// Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
		// int boundX = 0;
		// int boundY = 0;
		// for (uint16_t i = 0; i < numberOfeNodeBNodes; i++)  // Pendiente: enhance this function in order to position the enbs better.
		// {
		// 	for (uint16_t j = 0; j < numberOfeNodeBNodes; j++)  // Pendiente: enhance this function in order to position the enbs better.
		// 	{
		// 	//positionAlloc->Add (Vector(m_distance * i, 0, 0));
		// 	 boundX = m_distance *j;
		// 	 boundY= m_distance *i;
		// 	 if(boundX <= 6000 && boundY <= 6000 )
		// 	{
		// 		positionAlloc->Add (Vector( boundX, boundY , enBHeight));
		// 	}
		// }  }

		Ptr<ListPositionAllocator> positionAllocenB = CreateObject<ListPositionAllocator> ();
		positionAllocenB->Add (Vector( 1500, 1500 , enBHeight));
		positionAllocenB->Add (Vector( 4500, 1500 , enBHeight));
		positionAllocenB->Add (Vector( 1500, 4500 , enBHeight));
		positionAllocenB->Add (Vector( 4500, 4500 , enBHeight));

		MobilityHelper mobilityenB;
		//mobilityenB.SetMobilityModel("ns3::ConstantPositionMobilityModel");
		mobilityenB.SetPositionAllocator(positionAllocenB);
		// mobilityenB.SetPositionAllocator ("ns3::GridPositionAllocator",
		// 							   "MinX", DoubleValue (0.0),
		// 							   "MinY", DoubleValue (0.0),
		// 							   //"Z", DoubleValue (30.0),
		// 							   "DeltaX", DoubleValue (m_distance),
		// 							   "DeltaY", DoubleValue (m_distance),
		// 							   "GridWidth", UintegerValue (3),
		// 							   "LayoutType", StringValue ("RowFirst"));

		mobilityenB.Install(enbNodes);


		NS_LOG_UNCOND("Installing Mobility Model in UEs...");

		// Install Mobility Model User Equipments

		// Ptr<ListPositionAllocator> positionAllocUEs = CreateObject<ListPositionAllocator> ();
		// int boundX = 0;
		// int boundY = 0;
		// for (uint16_t i = 0; i < numberOfUENodes; i++)  // Pendiente: enhance this function in order to position the enbs better.
		// {
		// 	for (uint16_t j = 0; j < numberOfUENodes; j++)  // Pendiente: enhance this function in order to position the enbs better.
		// 	{
		// 	//positionAlloc->Add (Vector(m_distance * i, 0, 0));
		// 	 boundX = rand() % m_distance *j;
		// 	 boundY= rand() % m_distance *i;
		// 	 if(boundX <= 6000 && boundY <= 6000 )
		// 	{
				
		// 		positionAllocUEs->Add (Vector( boundX, boundY , 1.5));
		// 	}
		// }  }

		MobilityHelper mobilityUEs;
		mobilityUEs.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
									 "Mode", StringValue ("Time"),
									 "Time", StringValue ("0.5s"),//("1s"),
									 //"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=4.0]"),
									 //"Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
									 "Speed", StringValue ("ns3::UniformRandomVariable[Min=4.0|Max=8.0]"),
									 "Bounds", StringValue ("0|6000|0|6000"));
		// mobilityUEs.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
		// 	 							 "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"),
		// 								 "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"));
		mobilityUEs.SetPositionAllocator("ns3::RandomBoxPositionAllocator",  // to use OkumuraHataPropagationLossModel needs to be in a height greater then 0.
			 							 "X", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=6000.0]"),
										 "Y", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=6000.0]"),
										 "Z", StringValue ("ns3::UniformRandomVariable[Min=2|Max=5]"));
		//mobilityUEs.SetPositionAllocator(positionAllocUEs);

		mobilityUEs.Install(ueNodes);
	  
	  
		NS_LOG_UNCOND("Installing Mobility Model in UABSs...");
		// Install Mobility Model UABS

		Ptr<ListPositionAllocator> positionAllocUABS = CreateObject<ListPositionAllocator> ();
		positionAllocUABS->Add (Vector( 1500, 1500 , enBHeight)); //1
		positionAllocUABS->Add (Vector( 4500, 1500 , enBHeight)); //2
		positionAllocUABS->Add (Vector( 1500, 4500 , enBHeight)); //3
		positionAllocUABS->Add (Vector( 4500, 4500 , enBHeight)); //4
		positionAllocUABS->Add (Vector( 4500, 4500 , enBHeight)); //5
		positionAllocUABS->Add (Vector( 4500, 4500 , enBHeight)); //6

		MobilityHelper mobilityUABS;
		mobilityUABS.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
		//mobilityUABS.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
		//                         "Mode", StringValue ("Time"),
		  //                       "Time", StringValue ("5s"),
								 //"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
			//                     "Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
			//		     "Bounds", StringValue ("0|2000|0|2000"));
		mobilityenB.SetPositionAllocator(positionAllocUABS);
		// mobilityUABS.SetPositionAllocator ("ns3::GridPositionAllocator",
		// 								"MinX", DoubleValue (0.0),
		// 								"MinY", DoubleValue (0.0),
		// 								"DeltaX", DoubleValue (m_distance),
		// 								"DeltaY", DoubleValue (m_distance),
		// 								"GridWidth", UintegerValue (3),
		// 								"LayoutType", StringValue ("RowFirst"));

		mobilityUABS.Install(UABSNodes);

			  

		// Install LTE Devices to the nodes
		NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
		NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
		NetDeviceContainer UABSLteDevs = lteHelper->InstallEnbDevice (UABSNodes);

		


		// Get position of enBs, UABSs and UEs.
		Simulator::Schedule(Seconds(5), &GetPositionUEandenB,ueNodes, enbNodes,UABSNodes,enbLteDevs,UABSLteDevs,ueLteDevs);
	  

		//Set Power of eNodeBs  
		Ptr<LteEnbPhy> enodeBPhy; 

		for( uint16_t i = 0; i < enbLteDevs.GetN(); i++) 
		{
			enodeBPhy = enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
			enodeBPhy->SetTxPower(eNodeBTxPower);
			
		}
		// //Set Power of UABS, initially 0 to simulate a turned off UABS.
		Ptr<LteEnbPhy> UABSPhy;
	 
			for( uint16_t i = 0; i < UABSLteDevs.GetN(); i++) 
			{
				UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
				UABSPhy->SetTxPower(UABSTxPower);
				// NS_LOG_UNCOND("UABS TX Power: ");
				// NS_LOG_UNCOND(UABSPhy->GetTxPower());
			}


	  //Get Power of eNodeBs and UABSs
	 // 	Ptr<LteUePhy> UEPhy;
		// for(uint16_t i = 0; i < ueLteDevs.GetN(); i++)
		// {
		// 	UEPhy = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetPhy();
		// 	UEPhy->GetTxPower();
		// 	//UEPhy->GenerateCqiRsrpRsrq();
		// 	NS_LOG_UNCOND("Noise Figure: ");
		// 	NS_LOG_UNCOND(UEPhy->GetNoiseFigure());  
		// 	NS_LOG_UNCOND(" UE Tx Power: ");   
		// 	NS_LOG_UNCOND (UEPhy->GetTxPower());
			
	
	
	 
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
		//lteHelper->AttachToClosestEnb (ueLteDevs, enbLteDevs);
		lteHelper->Attach (ueLteDevs);
		
		// this enables handover for macro eNBs
		lteHelper->AddX2Interface (enbNodes); // X2 interface for macrocells
		lteHelper->AddX2Interface (UABSNodes); // X2 interface for UABSs

		//Set a X2 interface between UABS and all enBs to enable handover.
		for (uint16_t i = 0; i < UABSNodes.GetN(); i++) 
		{
			Ptr<Node> PosUABS = UABSNodes.Get(i)->GetObject<Node>();
			for (uint16_t j = 0; j < enbNodes.GetN(); j++) 
			{
				Ptr<Node> PosUABS = enbNodes.Get(j)->GetObject<Node>();
				//Set a X2 interface between UABS and all enBs	
				lteHelper->AddX2Interface(UABSNodes.Get(i), enbNodes.Get(j));
				NS_LOG_UNCOND("Creating X2 Interface between UABS " << UABSNodes.Get(i) << " and enB " << enbNodes.Get(j));
			}
		}
		
		
		// activate EPSBEARER
		lteHelper->ActivateDedicatedEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
	  
	  
		//get Sinr
		Simulator::Schedule(Seconds(5), &GetSinrUE,ueLteDevs,ueNodes);


		//Run Python Command to get centroids
		//GetPrioritizedClusters();
		Simulator::Schedule(Seconds(6), &GetPrioritizedClusters, UABSNodes,  speedUABS,  UABSLteDevs);

	
		NS_LOG_UNCOND("Resquesting-sending Video...");
	  	NS_LOG_INFO ("Create Applications.");
	   	for (uint16_t i = 0; i < ueNodes.GetN(); i++) 
		{
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
			apps = client.Install (ueNodes.Get(i));
		
		 
		
			apps.Start (Seconds (10.0));
			apps.Stop (Seconds (simTime));

			Ptr<Ipv4> ipv4 = ueNodes.Get(i)->GetObject<Ipv4>();
	  	}

		AnimationInterface anim ("UOSLTE.xml"); // Mandatory
		anim.SetMaxPktsPerTraceFile(500000); // Set animation interface max packets. (TO CHECK: how many packets i will be sending?) 
		// Cor e Descrição para eNb
		for (uint32_t i = 0; i < enbNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(enbNodes.Get(i), "eNb");
			anim.UpdateNodeColor(enbNodes.Get(i), 0, 255, 0);
			anim.UpdateNodeSize(i,100,100); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < ueNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(ueNodes.Get(i), "UEs");
			anim.UpdateNodeColor(ueNodes.Get(i),  255, 0, 0);
			anim.UpdateNodeSize(i,50,50); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < UABSNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(UABSNodes.Get(i), "UABS");
			anim.UpdateNodeColor(UABSNodes.Get(i), 0, 0, 255);
			anim.UpdateNodeSize(i,100,100); // to change the node size in the animation.
		}
			anim.UpdateNodeDescription(remoteHost, "RH");
			anim.UpdateNodeColor(remoteHost, 0, 255, 255);
		//anim.UpdateNodeSize(remoteHost,100,100); // to change the node size in the animation.
	 



	 
		//lteHelper->EnableTraces (); Set the traces on or off. 
	  

		//Enabling Traces
		lteHelper->EnablePhyTraces();
		lteHelper->EnableUlPhyTraces();
		lteHelper->EnableMacTraces();
		lteHelper->EnableRlcTraces();
		lteHelper->EnablePdcpTraces();
		
		/*--------------NOTIFICAÇÕES DE UE Mesasurements-------------------------*/
		Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/RecvMeasurementReport", MakeCallback (&NotifyMeasureMentReport)); 
		//Config::Connect ("/NodeList/*/DeviceList/*/LteUePhy/ReportCurrentCellRsrpSinr",MakeCallback (&ns3::PhyStatsCalculator::ReportCurrentCellRsrpSinrCallback));
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
		
		NS_LOG_UNCOND("Running simulation...");
		NS_LOG_INFO ("Run Simulation.");
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
