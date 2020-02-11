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




    LTE UABS Offloading Scheme
    Configuration:
        No.of Cells     : 6
        Cell Radius     : -
        No.Of users     : 100 moving user 
        User Speed      : 1 - 4 m/s
        Fading Model    :
        Path Loss Model :
        eNodeB Configuration:
            Tx Power        : 46dBm
        UE Configuration :
            AMC Selection Scheme    : ?
*/
 

#include <fstream>
#include <string.h>
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <memory>
#include <unordered_set>
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
#include <ns3/buildings-module.h>
#include "ns3/propagation-loss-model.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/cost231-propagation-loss-model.h"
#include "ns3/itu-r-1411-los-propagation-loss-model.h"
#include "ns3/itu-r-1411-nlos-over-rooftop-propagation-loss-model.h"

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
const uint16_t numberOfUENodes = 100; //Number of user to test: 245, 392, 490 (The number of users and their traffic model follow the parameters recommended by the 3GPP)
const uint16_t numberOfOverloadUENodes = 0; // user that will be connected to an specific enB. 
const uint16_t numberOfUABS = 6;
double simTime = 100; // 120 secs ||100 secs || 300 secs
const int m_distance = 2000; //m_distance between enBs towers.
// Inter packet interval in ms
// double interPacketInterval = 1;
// double interPacketInterval = 100;
// uint16_t port = 8000;
int evalvidId = 0;      
int eNodeBTxPower = 46; //Set enodeB Power dBm 46dBm --> 20MHz  |  43dBm --> 5MHz
int UABSTxPower = 0;//23;   //Set UABS Power
uint8_t bandwidth_enb = 100; // 100 RB --> 20MHz  |  25 RB --> 5MHz
uint8_t bandwidth_UABS = 100; // 100 RB --> 20MHz  |  25 RB --> 5MHz
//uint8_t bandwidth = 25; // To use with UABS --> tengo que ver si no necesito crear otro LTEhelper solo para los UABS.
double speedUABS = 0;
double ue_info[numberOfeNodeBNodes + numberOfUABS][numberOfUENodes]; //UE Connection Status Register Matrix
double ue_imsi_sinr[numberOfUENodes]; //UE Connection Status Register Matrix
double ue_imsi_sinr_linear[numberOfUENodes];
double ue_info_cellid[numberOfUENodes];
int minSINR = 0; //  minimum SINR to be considered to clusterization
string GetClusterCoordinates;
double PDR=0.0; //Packets Delay Rate
double PLR=0.0; //Packets Lost Rate
double APD=0.0;	//Average Packet Delay
double Avg_Jitter=0.0;	//Average Packet Jitter
bool UABSFlag;
bool UABS_On_Flag = false;
std::stringstream cmd;
double UABSHeight = 80;
double enBHeight = 30;
uint32_t nRuns = 1;
uint32_t randomSeed = 1234;
int scen = 4; 
// [Scenarios --> Scen[0]: General Scenario, with no UABS support, network ok;  Scen[1]: one enB damaged (off) and no UABS;
// Scen[2]: one enB damaged (off) with supporting UABS; Scen[3]:Overloaded enB(s) with no UABS support; Scen[4]:Overloaded enB(s) with UABS support; ]
int enBpowerFailure=0;
int transmissionStart = 0;
//double UABSPriority[20];
bool graphType = false; // If "true" generates all the graphs based in FlowsVSThroughput, if "else" generates all the graphs based in TimeVSThroughput
int remMode = 0; // [0]: REM disabled; [1]: generate REM at 1 second of simulation;
//[2]: generate REM at simTime/2 seconds of simulation
bool disableUl = true;
std::stringstream Users_UABS; // To UEs cell id in every second of the simulation
std::stringstream Qty_UABS; //To get the quantity of UABS used per RUNS
std::stringstream uenodes_log;
std::stringstream Qty_UE_SINR;
std::ofstream UE_UABS; // To UEs cell id in every second of the simulation
std::ofstream UABS_Qty; //To get the quantity of UABS used per RUNS
Gnuplot2dDataset datasetAvg_Jitter;
NodeContainer ueNodes;

	 
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

		

		void ns3::PhyStatsCalculator::ReportUeSinr(uint16_t cellId, uint64_t imsi, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
		{
			double sinrdB = 10 * log(sinrLinear); 
			//feed UE_info with actual SINR in dB.
			//ue_info[cellId-1][imsi-1] = sinrdB;
			ue_info_cellid[imsi-1] = cellId;
			ue_imsi_sinr[imsi-1]=sinrdB; 
			ue_imsi_sinr_linear[imsi-1]=sinrLinear; //To pass SIRN Linear to python code to do the linear sum
			//std::cout << "Sinr: " << ue_imsi_sinr[imsi-1] <<" Sinr Linear: "<< sinrLinear << " Imsi: "<< imsi << " CellId: " << cellId << " rnti: "<< rnti << endl;
			
			

			UE_UABS << (double)Simulator::Now().GetSeconds() << "," << imsi << "," << cellId <<"," << sinrdB <<std::endl;



		}


		void GetPositionUEandenB(NodeContainer enbNodes, NodeContainer UABSNodes, NetDeviceContainer enbLteDevs,NetDeviceContainer UABSLteDevs, NodeContainer ueOverloadNodes , NetDeviceContainer ueLteDevs)
		{
		// iterate our nodes and print their position.
			std::stringstream enodeB;
			enodeB << "enBs"; 
			std::stringstream uenodes;
			uenodes << "LTEUEs";
			std::stringstream OverloadingUenodes;
			OverloadingUenodes << "LTE_Overloading_UEs";
			std::stringstream UABSnod;
			UABSnod << "UABSs";
			std::ofstream enB;
			enB.open(enodeB.str());    
			std::ofstream UE;
			UE.open(uenodes.str());
			std::ofstream UE_Log;
			UE_Log.open(uenodes_log.str(),std::ios_base::app);   
			std::ofstream OverloadingUE;
			OverloadingUE.open(OverloadingUenodes.str());    
			std::ofstream UABS;
			UABS.open(UABSnod.str());   
			uint16_t enBCellId;
			uint16_t UABSCellId;
			//uint16_t UEConnectedCellId;
			Ptr<LteEnbPhy> UABSPhy;
			Time now = Simulator::Now (); 
			uint64_t UEImsi;
			
			int i=0; 
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
				i++;
			}
			enB.close();

			i=0;
			for (NodeContainer::Iterator j = ueNodes.Begin ();j != ueNodes.End (); ++j)
			{
				Ptr<Node> object = *j;
				Ptr<MobilityModel> UEposition = object->GetObject<MobilityModel> ();
				NS_ASSERT (UEposition != 0);
				Vector pos = UEposition->GetPosition ();
				UE << pos.x << "," << pos.y << "," << pos.z << std::endl;
				UEImsi = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
				UE_Log << now.GetSeconds() << ","  << pos.x << "," << pos.y << "," << pos.z << "," << UEImsi << "," << ue_info_cellid[UEImsi-1] << "," << ue_imsi_sinr_linear[UEImsi-1] << std::endl;
				i++;
				
			}
			UE.close();

			for (NodeContainer::Iterator j = ueOverloadNodes.Begin ();j != ueOverloadNodes.End (); ++j)
			{

				Ptr<Node> object = *j;
				Ptr<MobilityModel> OverloadingUEposition = object->GetObject<MobilityModel> ();
				NS_ASSERT (OverloadingUEposition != 0);
				Vector pos = OverloadingUEposition->GetPosition ();
				OverloadingUE << pos.x << "," << pos.y << "," << pos.z << std::endl;
				
			}
			OverloadingUE.close();


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
				
				k++;
			}

			UABS.close();

			Simulator::Schedule(Seconds(5), &GetPositionUEandenB,enbNodes,UABSNodes,enbLteDevs,UABSLteDevs,ueOverloadNodes,ueLteDevs);
			
		}

	  //---------------Get SINR of UEs and Positions--------------------------//
		void GetSinrUE (NetDeviceContainer ueLteDevs, NodeContainer ueNodes, NodeContainer ueOverloadNodes, NetDeviceContainer OverloadingUeLteDevs)
		{  
			uint64_t UEImsi;
			uint64_t UEOverloadImsi;
			std::stringstream uenodes;
			uenodes << "UEsLowSinr";    
			std::ofstream UE;
			UE.open(uenodes.str());
			std::ofstream Qty_UE_SINR_file;
			Qty_UE_SINR_file.open(Qty_UE_SINR.str(),std::ios_base::app);
			// NodeContainer::Iterator j = ueNodes.Begin();
			// NodeContainer::Iterator q = ueOverloadNodes.Begin();
			int k =0;
			int z =0;
			int i =0;
			int q =0;

			//for(uint16_t i = 0; i < ueLteDevs.GetN(); i++)
			//{
				for (NodeContainer::Iterator j = ueNodes.Begin ();j != ueNodes.End (); ++j)
				{
					UEImsi = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();
					if (ue_imsi_sinr[UEImsi-1] < minSINR) 
					{	
						//NS_LOG_UNCOND("Sinr: "<< ue_imsi_sinr[UEImsi] << " Imsi: " << UEImsi );
						//UE << "Sinr: "<< ue_imsi_sinr[UEImsi] << " Imsi: " << UEImsi << std::endl;
						
						Ptr<Node> object = *j;
						Ptr<MobilityModel> UEposition = object->GetObject<MobilityModel> ();
						NS_ASSERT (UEposition != 0);
						Vector pos = UEposition->GetPosition ();
						UE << pos.x << "," << pos.y << "," << pos.z << "," << ue_imsi_sinr_linear[UEImsi-1] << ","<< UEImsi<< "," << ue_info_cellid[UEImsi-1]<< std::endl;
						++i;
						++k;
						
					}
				}
			NS_LOG_UNCOND("Users with low sinr: "); //To know if after an UABS is functioning this number decreases.
			NS_LOG_UNCOND(k);
			Qty_UE_SINR_file << Simulator::Now().GetSeconds() << "," << k << std::endl;

			// for(uint16_t i = 0; i < OverloadingUeLteDevs.GetN(); i++)
			// {
				for (NodeContainer::Iterator j = ueOverloadNodes.Begin ();j != ueOverloadNodes.End (); ++j)
				{
					UEOverloadImsi = OverloadingUeLteDevs.Get(q)->GetObject<LteUeNetDevice>()->GetImsi();
					if (ue_imsi_sinr[UEOverloadImsi-1] < minSINR) // revisar aqui si tengo que poner uephy-1 (imsi-1)
					{	
						//NS_LOG_UNCOND("Sinr: "<< ue_imsi_sinr[UEImsi] << " Imsi: " << UEImsi );
						//UE << "Sinr: "<< ue_imsi_sinr[UEImsi] << " Imsi: " << UEImsi << std::endl;
						
						Ptr<Node> object = *j;
						Ptr<MobilityModel> UEposition = object->GetObject<MobilityModel> ();
						NS_ASSERT (UEposition != 0);
						Vector pos = UEposition->GetPosition ();
						UE << pos.x << "," << pos.y << "," << pos.z << "," << ue_imsi_sinr_linear[UEOverloadImsi-1] << ","<< UEOverloadImsi<< "," << ue_info_cellid[UEOverloadImsi-1]<< std::endl;
						++q;
						++z;
						
					}
			}
			NS_LOG_UNCOND("Overloading Users with low sinr: "); //To know if after an UABS is functioning this number decreases.
			NS_LOG_UNCOND(z);

			UE.close();
			Simulator::Schedule(Seconds(5), &GetSinrUE,ueLteDevs,ueNodes, ueOverloadNodes, OverloadingUeLteDevs);
		}


		void SetTXPowerPositionAndVelocityUABS(NodeContainer UABSNodes, double speedUABS, NetDeviceContainer UABSLteDevs, std::vector<ns3::Vector3D> CoorPriorities_Vector, double UABSPriority[])
		{
			Ptr<LteEnbPhy> UABSPhy;
			uint16_t UABSCellId;
			
			if (UABSFlag == true )//&& UABS_On_Flag == false) 
			{
				UABSTxPower = 27;
				speedUABS = 0.1;

				if (CoorPriorities_Vector.size() <= UABSNodes.GetN())
				{
				
					for (uint16_t k=0 ; k < CoorPriorities_Vector.size(); k++)
					{	
						
						for (uint16_t i=0 ; i < UABSLteDevs.GetN(); i++)
						{
							UABSCellId = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetCellId();
							if (UABSPriority[k] == UABSCellId)
							{
								//--------------------Set Position of UABS / or trajectory to go to assist a low SINR Area:--------------------//
								// NS_LOG_UNCOND("UABSCellId:");
								// NS_LOG_UNCOND(UABSCellId);
								// NS_LOG_UNCOND("UABS_Prior_CellID:");
								// NS_LOG_UNCOND(UABSPriority[k]);
								Ptr<ConstantVelocityMobilityModel> PosUABS = UABSNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
								PosUABS->SetPosition(CoorPriorities_Vector.at(k));
								NS_LOG_UNCOND (PosUABS->GetPosition());

								//----------------------Turn on UABS TX Power-----------------------//
								UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
								UABSPhy->SetTxPower(UABSTxPower);

								//-------------------Set Velocity of UABS to start moving:----------------------//
								Ptr<ConstantVelocityMobilityModel> VelUABS = UABSNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
								VelUABS->SetVelocity(Vector(speedUABS, 0,0));
								//NS_LOG_UNCOND (VelUABS->GetVelocity());
							}
				 		}

					}	
				}
				else
				{
					for (uint16_t i=0 ; i < UABSNodes.GetN(); i++)
					{	
						UABSCellId = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetCellId();
						for (uint16_t k=0 ; k < CoorPriorities_Vector.size(); k++)
						{
							if (UABSCellId == UABSPriority[k])
							{
								//--------------------Set Position of UABS / or trajectory to go to assist a low SINR Area:--------------------//
								Ptr<ConstantVelocityMobilityModel> PosUABS = UABSNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
								PosUABS->SetPosition(CoorPriorities_Vector.at(k));
								//NS_LOG_UNCOND("Temp: ");
								//NS_LOG_UNCOND(CoorPriorities_Vector.at(k));
								//NS_LOG_UNCOND("GetPosition: ");
								NS_LOG_UNCOND (PosUABS->GetPosition());

								//---------------------Turn on UABS TX Power-------------------------------//
								UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
								UABSPhy->SetTxPower(UABSTxPower);

								//-------------------Set Velocity of UABS to start moving:----------------------//
								Ptr<ConstantVelocityMobilityModel> VelUABS = UABSNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
								VelUABS->SetVelocity(Vector(speedUABS, 0,0));//speedUABS, 0));
								//NS_LOG_UNCOND (VelUABS->GetVelocity());	
							}
						}	
					}
				}
			//UABS_On_Flag = true;
			}
			else if (UABSFlag == false )//&& UABS_On_Flag == false) 
			{

				UABSTxPower = 0;
				speedUABS= 0;
				for( uint16_t i = 0; i < UABSLteDevs.GetN(); i++) 
				{
					//-------------------Turn Off UABS Power-----------------//
					UABSPhy = UABSLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
					UABSPhy->SetTxPower(UABSTxPower);
					
					//---------------Set UABS velocity to 0.-------------------//
					Ptr<ConstantVelocityMobilityModel> VelUABS = UABSNodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
					VelUABS->SetVelocity(Vector(speedUABS, 0,0));//speedUABS, 0));
					//NS_LOG_UNCOND (VelUABS->GetVelocity());
					
				}

			}

		}

		//--------------------Function to execute Python in console--------------//
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


		void GetPrioritizedClusters(NodeContainer UABSNodes, double speedUABS, NetDeviceContainer UABSLteDevs)
		{
			std::vector<std::string> Split_coord_Prior;
			ns3::Vector3D CoorPriorities;
			std::vector<ns3::Vector3D>  CoorPriorities_Vector;
			std::unordered_set<double> used_UABs;
			int j=0;
			double priority;
			double UABSPriority[20];

			// Call Python code to get string with clusters prioritized and trajectory optimized (Which UABS will serve which cluster).
			cmd << "python3 UOS-PythonCode.py " << " 2>/dev/null ";
			GetClusterCoordinates =  exec(cmd.str().c_str());
			

			if (!GetClusterCoordinates.empty())
			{
				UABSFlag = true;
				NS_LOG_UNCOND("Coordinates of prioritized Clusters: " + GetClusterCoordinates);

				boost::split(Split_coord_Prior, GetClusterCoordinates, boost::is_any_of(" "), boost::token_compress_on);
				UABSPriority [Split_coord_Prior.size()];

				for (uint16_t i = 0; i < Split_coord_Prior.size()-2 && used_UABs.size() < numberOfUABS; i+=3)
				{
					priority = std::stod(Split_coord_Prior[i+2]); //Save priority into a double array. // cambie UABSPriority [i] por UABSPriority [j] porque i incrementa de 3 en 3. 
					if(used_UABs.count(priority)==0){
						used_UABs.insert(priority);
						UABSPriority [j] = priority;
						CoorPriorities = Vector(std::stod(Split_coord_Prior[i]),std::stod(Split_coord_Prior[i+1]),UABSHeight); //Vector containing: [X,Y,FixedHeight]
						CoorPriorities_Vector.push_back(CoorPriorities); 
						j++;
					}
				}
			}
			else 
			{
				UABSFlag = false;
				UABS_On_Flag = false;
				NS_LOG_UNCOND("No prioritized cluster needed, users to far from each other.");
			}

			if (UABSFlag == true)
			{
				
				NS_LOG_UNCOND(std::to_string(j) <<" UABS needed: Setting TXPower, Velocity and position");
				SetTXPowerPositionAndVelocityUABS(UABSNodes, speedUABS, UABSLteDevs, CoorPriorities_Vector, UABSPriority); 
				UABS_Qty << (double)Simulator::Now().GetSeconds() << "," <<  std::to_string(j) << std::endl; 
			}
			
			Simulator::Schedule(Seconds(6), &GetPrioritizedClusters,UABSNodes,  speedUABS,  UABSLteDevs);
		}



		void ThroughputCalc(Ptr<FlowMonitor> monitor, Ptr<Ipv4FlowClassifier> classifier,Gnuplot2dDataset datasetThroughput,Gnuplot2dDataset datasetPDR,Gnuplot2dDataset datasetPLR, Gnuplot2dDataset datasetAPD)
		{
		uint32_t txPacketsum = 0;
		uint32_t rxPacketsum = 0;
		uint32_t DropPacketsum = 0;
		uint32_t LostPacketsum = 0;
		double Delaysum = 0;
		double Jittersum = 0;
		double Throughput=0.0;

		monitor->CheckForLostPackets ();
		//Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon->GetClassifier ());
		std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
		{
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

			txPacketsum += iter->second.txPackets;
			rxPacketsum += iter->second.rxPackets;
			LostPacketsum = txPacketsum-rxPacketsum;
			DropPacketsum += iter->second.packetsDropped.size();
			Delaysum += iter->second.delaySum.GetSeconds();
			Jittersum += iter->second.jitterSum.GetSeconds(); //Jitter is the amount of variation in latency/response time, in milliseconds. Reliable connections consistently report back the same latency over and over again. Lots of variation (or 'jitter') is an indication of problems.

			// std::cout<<"Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress<<"\n";
			// std::cout<<"Tx Packets = " << iter->second.txPackets<<"\n";
			// std::cout<<"Rx Packets = " << iter->second.rxPackets<<"\n";

			// //std::cout << "  All Tx Packets: " << txPacketsum << "\n";
			// //std::cout << "  All Rx Packets: " << rxPacketsum << "\n";
			// //std::cout << "  All Delay/Average Packet Delay (APD): " << Delaysum / txPacketsum << "\n"; //APD = Average Packet Delay : to do !
			// //std::cout << "  All Lost Packets: " << LostPacketsum << "\n";
			// //std::cout << "  All Drop Packets: " << DropPacketsum << "\n";

			// std::cout<<"Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) /1024 << " Kbps\n";///1024 << " Mbps\n";
			// std::cout << "Packets Delivery Ratio: " << ((rxPacketsum * 100) / txPacketsum) << "%" << "\n";
			// std::cout << "Packets Loss Ratio: " << ((LostPacketsum * 100) / txPacketsum) << "%" << "\n";
			// std::cout << "Average Packet Delay: " << Delaysum / txPacketsum << "\n"; 
			
			PDR = ((rxPacketsum * 100) / txPacketsum);
			PLR = ((LostPacketsum * 100) / txPacketsum);
			APD = (Delaysum / rxPacketsum);
			Avg_Jitter = (Jittersum / rxPacketsum);
			
			// Save in datasets to later plot the results.
			if (graphType == true){
			Throughput = iter->second.rxBytes * 8.0 /(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/ 1024;// / 1024;
			datasetThroughput.Add((double)iter->first,(double) Throughput);
			datasetPDR.Add((double)iter->first,(double) PDR);
			datasetPLR.Add((double)iter->first,(double) PLR);
			datasetAPD.Add((double)iter->first,(double) APD);
			datasetAvg_Jitter.Add((double)iter->first,(double) Avg_Jitter);
			} else {
			Throughput += iter->second.rxBytes * 8.0 /(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/ 1024;// / 1024;
			}
		}
		if(!graphType){
		datasetThroughput.Add((double)Simulator::Now().GetSeconds(),(double) Throughput);
		datasetPDR.Add((double)Simulator::Now().GetSeconds(),(double) PDR);
		datasetPLR.Add((double)Simulator::Now().GetSeconds(),(double) PLR);
		datasetAPD.Add((double)Simulator::Now().GetSeconds(),(double) APD);
		datasetAvg_Jitter.Add((double)Simulator::Now().GetSeconds(),(double) Avg_Jitter);
		}

			//}

		//monitor->SerializeToXmlFile("UOSLTE-FlowMonitor.xml",true,true);
		//monitor->SerializeToXmlFile("UOSLTE-FlowMonitor_run_"+std::to_string(z)+".xml",true,true);
		Simulator::Schedule(Seconds(1),&ThroughputCalc, monitor,classifier,datasetThroughput,datasetPDR,datasetPLR,datasetAPD);


		}

		void UDPApp (Ptr<Node> remoteHost, NodeContainer ueNodes, Ipv4Address remoteHostAddr, Ipv4InterfaceContainer ueIpIface) {
			// Install and start applications on UEs and remote host
		  
			ApplicationContainer serverApps;
			ApplicationContainer clientApps;
			Time interPacketInterval = MilliSeconds (10);
			uint16_t dlPort = 1100;
			uint16_t ulPort = 2000;
			  
			// Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
	  //  		startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
	  //  		startTimeSeconds->SetAttribute ("Max", DoubleValue (interPacketInterval/1000.0));


		  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
		  {
		  	
		  	
	    	// int startTime = rand() % (int)simTime + 2;
			int startTime = (rand() % (int)10)/10.0 + 2;
			ulPort++;
			dlPort++;
			
		       //if (!disableDl)
		         //{
		          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
		          serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));
		          
		          UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
		          dlClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
		          dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
		          dlClient.SetAttribute ("PacketSize", UintegerValue (1024));
		          clientApps.Add (dlClient.Install (remoteHost));
		         //}

		       if (!disableUl)
		         {
		          //++ulPort;
		          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
		          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

		          UdpClientHelper ulClient (remoteHostAddr, ulPort);
		          ulClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
		          ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
		          //ulClient.SetAttribute ("PacketSize", UintegerValue (1024));
		          clientApps.Add (ulClient.Install (ueNodes.Get(u)));
		         }
				serverApps.Start (Seconds(1));
				clientApps.Start (Seconds(startTime));
				//clientApps.Start (Seconds(2));
				clientApps.Stop (Seconds(simTime-2));
			}

		  		
		  		//Simulator::Schedule (Seconds (1), &CalculateThroughput,ueNodes,clientApps);

		}
		void requestVideoStream(Ptr<Node> remoteHost, NodeContainer ueNodes, Ipv4Address remoteHostAddr, double simTime)//, double start)
		{
			for (uint16_t i = 0; i < ueNodes.GetN(); i++) 
			{
				evalvidId++;
				//int startTime = rand() % (int)simTime + 2; // a random number between 2 - simtime (actual 100 segs)
				//int startTime = rand() % 40 + 20;
				//NS_LOG_UNCOND("Node " << i << " requesting video at " << startTime << "\n");
				uint16_t  port = 8000 * evalvidId + 8000; //to use a different port in every iterac...
				std::stringstream sdTrace;
        		std::stringstream rdTrace;
        		sdTrace << "UOS_vidslogs/sd_a01_" << evalvidId; //here there is a problem when is called for the users that are overloading the enb, it overwrites. To fix.
        		rdTrace << "UOS_vidslogs/rd_a01_" << evalvidId;

			//Video Server
				EvalvidServerHelper server(port);
				server.SetAttribute ("SenderTraceFilename", StringValue("evalvid_videos/st_highway_600_cif")); //Old: src/evalvid/st_highway_cif.st
				server.SetAttribute ("SenderDumpFilename", StringValue(sdTrace.str()));
				server.SetAttribute("PacketPayload", UintegerValue(512));
				ApplicationContainer apps = server.Install(remoteHost);
				apps.Start (Seconds (1.0));
				apps.Stop (Seconds (simTime));

			// Clients
				EvalvidClientHelper client (remoteHostAddr,port);
				client.SetAttribute ("ReceiverDumpFilename", StringValue(rdTrace.str()));
				apps = client.Install (ueNodes.Get(i));
			
			 
				apps.Start (Seconds (2.0)); //starttime
				apps.Stop (Seconds (simTime));

				Ptr<Ipv4> ipv4 = ueNodes.Get(i)->GetObject<Ipv4>();
	  		}
		}


		void enB_Failure (NetDeviceContainer enbLteDevs,NetDeviceContainer ueLteDevs, Ptr<LteHelper> lteHelper,int enBpowerFailure )
		{
			//Set Power of eNodeBs  
			Ptr<LteEnbPhy> enodeBPhy;
			uint16_t enBCellId; 
			uint64_t UeHandoverImsi;
			Ptr<LteUeNetDevice> UeToHandoff;
			
			for (uint16_t i = 0; i < ueLteDevs.GetN(); i++)
			{	
				UeToHandoff = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>();
				//UeHandoverImsi = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetImsi();//GetTargetEnb()->GetCellId();
				UeHandoverImsi = ueLteDevs.Get(i)->GetObject<LteUeNetDevice>()->GetTargetEnb()->GetCellId();
				
				//if (ue_info_cellid[UeHandoverImsi-1] == 1)
				if (UeHandoverImsi == 1)
				{
					Ptr<LteEnbNetDevice> Source_enB = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>();
					Ptr<LteEnbNetDevice> Target_enB = enbLteDevs.Get(1)->GetObject<LteEnbNetDevice>();
					lteHelper->HandoverRequest(Seconds(0.1),UeToHandoff,Source_enB,Target_enB);
					NS_LOG_UNCOND("Ue " << std::to_string(UeHandoverImsi) <<" handover triggered from enB " << std::to_string(Source_enB->GetCellId()) << " to enB " << std::to_string(Target_enB->GetCellId()));
				}
			}	
			

			if  (enBpowerFailure == 0)
			{
			//for( uint16_t i = 0; i < enbLteDevs.GetN(); i++) 
			//{
				// To simulate a failure we set the Tx Power to 0w in the enb[0].
				enBCellId = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetCellId();
				enodeBPhy = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetPhy();
				//enodeBPhy->SetTxPower(20);
				//NS_LOG_UNCOND("enB " << std::to_string(enBCellId) << " is presenting power fault! [enbpower: 20]");
				enBpowerFailure = 2;
				Simulator::Schedule(Seconds(5), &enB_Failure,enbLteDevs,ueLteDevs,lteHelper,enBpowerFailure);
			
			//}
			}
			// else if (enBpowerFailure == 1)
			// {
			// 	enBCellId = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetCellId();
			// 	enodeBPhy = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetPhy();
			// 	enodeBPhy->SetTxPower(10);
			// 	NS_LOG_UNCOND("enB " << std::to_string(enBCellId) << " is presenting power fault! [enbpower: 10]");
			// 	enBpowerFailure = 2;
			// 	Simulator::Schedule(Seconds(30), &enB_Failure,enbLteDevs,ueLteDevs,lteHelper,enBpowerFailure);

			// }
			else if (enBpowerFailure == 2)
			{
				enBCellId = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetCellId();
				enodeBPhy = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>()->GetPhy();
				enodeBPhy->SetTxPower(0);
				NS_LOG_UNCOND("enB " << std::to_string(enBCellId) << " out of service: Power Failure!");

			}


		}

		void enB_Overload ( Ptr<LteHelper> lteHelper, NetDeviceContainer OverloadingUeLteDevs, NetDeviceContainer enbLteDevs)
		{
			NS_LOG_UNCOND("Attaching Overloading Ues in enB 1...");
			//Ptr<LteEnbNetDevice> Target_enB = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>();
			//lteHelper->Attach(OverloadingUeLteDevs, Target_enB);
			lteHelper->Attach(OverloadingUeLteDevs);
			//Simulator::Schedule(Seconds(10),&enB_Overload, lteHelper, OverloadingUeLteDevs,enbLteDevs);

		}

		void NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellId,
                       uint16_t rnti,
                       uint16_t targetCellId)
		{
  			std::cout << Simulator::Now ().GetSeconds () << " " << context
			          << " UE IMSI " << imsi
			          << ": previously connected to CellId " << cellId
			          << " with RNTI " << rnti
			          << ", doing handover to CellId " << targetCellId
			          << std::endl;
		}

		void NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellId,
                       uint16_t rnti)
		{
		  	std::cout << Simulator::Now ().GetSeconds () << " " << context
		              << " UE IMSI " << imsi
		              << ": successful handover to CellId " << cellId
		              << " with RNTI " << rnti
		             << std::endl;
		}

		void NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellId,
                        uint16_t rnti,
                        uint16_t targetCellId)
		{
			std::cout << Simulator::Now ().GetSeconds () << " " << context
			          << " eNB CellId " << cellId
			          << ": start handover of UE with IMSI " << imsi
			          << " RNTI " << rnti
			          << " to CellId " << targetCellId
			          << std::endl;
		}

		void NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellId,
                        uint16_t rnti)
		{
			 std::cout << Simulator::Now ().GetSeconds () << " " << context
			            << " eNB CellId " << cellId
			            << ": completed handover of UE with IMSI " << imsi
			            << " RNTI " << rnti
			            << std::endl;
		}


		// MAIN FUNCTION
		int main (int argc, char *argv[])
		{
		//LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
		//LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);

		// File to Log all Users that will be connected to UABS and how many UABS will be activated.
		

		CommandLine cmm;
    	cmm.AddValue("randomSeed", "value of seed for random", randomSeed);
    	cmm.AddValue("scen", "scenario to run", scen);
    	cmm.AddValue("nRuns", "Number of runs", nRuns);
    	cmm.AddValue("graphType","Type of graphs", graphType); 
    	//cmm.AddValue("numberOfUABS", "Number of UABS", numberOfUABS);
    	//cmm.AddValue("numberOfeNodeBNodes", "Number of enBs", numberOfeNodeBNodes);
    	cmm.AddValue("remMode","Radio environment map mode",remMode);
    	cmm.Parse(argc, argv);

		for (uint32_t z = 0; z < nRuns; z++){
				uint32_t seed = randomSeed + z;
				SeedManager::SetSeed (seed);
				NS_LOG_UNCOND("Run # " << std::to_string(z));
				
				Users_UABS.str(""); //To clean these variables in every run because they are global.
				Qty_UABS.str("");	//To clean these variables in every run because they are global.
				uenodes_log.str("");
				Qty_UE_SINR.str("");

				Users_UABS << "UE_info_UABS_RUN#" << z;
				Qty_UABS << "Quantity_UABS_per_RUN#" << z;
				uenodes_log << "LTEUEs_Log_RUN#" << z;
				Qty_UE_SINR << "Qty_UE_SINR_RUN#" << z;

			
				UE_UABS.open(Users_UABS.str(),std::ios_base::app);
				UABS_Qty.open(Qty_UABS.str(),std::ios_base::app);


		Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
		//Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
		Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
		lteHelper->SetEpcHelper (epcHelper);  //Evolved Packet Core (EPC)
		
		//----------------------Proportional Fair Scheduler-----------------------//
		lteHelper->SetSchedulerType("ns3::PfFfMacScheduler"); // Scheduler es para asignar los recursos un UE va a tener  (cuales UE deben tener recursos y cuanto)
		//PfFfMacScheduler --> es un proportional fair scheduler
		//----------------------PSS Scheduler-----------------------//
		// lteHelper->SetSchedulerType("ns3::PssFfMacScheduler");  //Priority Set scheduler.
		// lteHelper->SetSchedulerAttribute("nMux",UintegerValue(1)); // the maximum number of UE selected by TD scheduler
  		//lteHelper->SetSchedulerAttribute("PssFdSchedulerType", StringValue("CoItA")); // PF scheduler type in PSS
		
		// Modo de transmissão (SISO [0], MIMO [1])
    	Config::SetDefault("ns3::LteEnbRrc::DefaultTransmissionMode",UintegerValue(0));

		Ptr<Node> pgw = epcHelper->GetPgwNode ();
	  
		Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (true));
		Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));


		 Config::SetDefault( "ns3::LteUePhy::TxPower", DoubleValue(12) );         // Transmission power in dBm
		 Config::SetDefault( "ns3::LteUePhy::NoiseFigure", DoubleValue(9) );     // Default 5
		

	  
		//Set Handover algorithm 

		lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm"); // Handover algorithm implementation based on RSRQ measurements, Event A2 and Event A4.
		lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (30));
		lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (3));                                      
		//lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm"); // Handover by Reference Signal Reference Power (RSRP)
		//lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (256))); //default: 256
		//lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue (3.0)); //default: 3.0
		//Config::SetDefault ("ns3::LteEnbRrc::HandoverJoiningTimeoutDuration", TimeValue (Seconds (1)));
		//Config::SetDefault ("ns3::LteEnbRrc::HandoverLeavingTimeout", TimeValue (Seconds (3)));

		//Pathlossmodel
		if (scen == 0 || scen == 1 || scen == 3)
		{
			NS_LOG_UNCOND("Pathloss model: Nakagami Propagation ");
			lteHelper->SetAttribute("PathlossModel",StringValue("ns3::NakagamiPropagationLossModel"));

			NS_LOG_UNCOND("Pathloss model: OkumuraHata ");
			ObjectFactory modelFactory;
			modelFactory.SetTypeId (OkumuraHataPropagationLossModel::GetTypeId());
			modelFactory.Set ("Environment", StringValue("Urban"));
			modelFactory.Set ("Frequency", DoubleValue(1.8e9));
			//lteHelper->SetAttribute("PathlossModel",StringValue("ns3::OkumuraHataPropagationLossModel"));
	     	//lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
	     	//lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue(1.8e9));
	     	Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue (Seconds(1.00)));

	  //   	NS_LOG_UNCOND("Pathloss model: ItuR1411LosPropagationLossModel ");
			// lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ItuR1411LosPropagationLossModel"));
			// lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue(18100));


			// NS_LOG_UNCOND("Pathloss model: ItuR1411NlosOverRooftopPropagationLossModel ");	
			// lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ItuR1411NlosOverRooftopPropagationLossModel"));
			// lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue(18100));
			// lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
			// lteHelper->SetPathlossModelAttribute("RooftopLevel", DoubleValue(20.0));
			lteHelper->Initialize ();
			lteHelper->GetDownlinkSpectrumChannel()->AddPropagationLossModel(modelFactory.Create<PropagationLossModel>());
			lteHelper->GetUplinkSpectrumChannel()->AddPropagationLossModel(modelFactory.Create<PropagationLossModel>());
		}

		//lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
		
		if (scen == 2 || scen == 4)
		{	
			
			NS_LOG_UNCOND("Pathloss model: Nakagami Propagation ");
			lteHelper->SetAttribute("PathlossModel",StringValue("ns3::NakagamiPropagationLossModel"));

			ObjectFactory modelFactory;
			modelFactory.SetTypeId (OkumuraHataPropagationLossModel::GetTypeId());
			modelFactory.Set ("Environment", StringValue("Urban"));
			modelFactory.Set ("Frequency", DoubleValue(1.8e9));

			NS_LOG_UNCOND("Pathloss model: OkumuraHata ");
			//lteHelper->SetAttribute("PathlossModel",StringValue("ns3::OkumuraHataPropagationLossModel"));
	    	//lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
	    	//lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue(1.8e9));
	    Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue (Seconds(1.00)));
	    
			//modelFactory.SetTypeId (Cost231PropagationLossModel::GetTypeId());
			//modelFactory.Set ("Frequency", DoubleValue(1.8e9));
			// NS_LOG_UNCOND("Pathloss model: HybridBuildingsPropagationLossModel ");
			// lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
			// lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (0));
			// lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (1));
			// lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (1.5));
			//  // use always LOS model
			// lteHelper->SetPathlossModelAttribute ("Los2NlosThr", DoubleValue (1e6));
			// lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");

			//NS_LOG_UNCOND("Pathloss model: ItuR1411LosPropagationLossModel ");
			//lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ItuR1411LosPropagationLossModel"));
			//lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue(1.8e9));

			//NS_LOG_UNCOND("Pathloss model: ItuR1411NlosOverRooftopPropagationLossModel ");
			//lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ItuR1411NlosOverRooftopPropagationLossModel"));
			//lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue(1.8e9));
			//lteHelper->SetPathlossModelAttribute("Environment", StringValue("Urban"));
			//lteHelper->SetPathlossModelAttribute("RooftopLevel", DoubleValue(20.0));

			//modelFactory.SetTypeId (ItuR1411NlosOverRooftopPropagationLossModel::GetTypeId());
			//modelFactory.Set ("Frequency", DoubleValue(1.8e9));
			//modelFactory.Set ("Environment", StringValue("Urban"));
			//modelFactory.Set ("RooftopLevel", DoubleValue(20.0));
		
			lteHelper->Initialize ();
			lteHelper->GetDownlinkSpectrumChannel()->AddPropagationLossModel(modelFactory.Create<PropagationLossModel>());
			lteHelper->GetUplinkSpectrumChannel()->AddPropagationLossModel(modelFactory.Create<PropagationLossModel>());

			//NS_LOG_UNCOND("Pathloss model: ItuR1411LosPropagationLossModel ");
			//modelFactory.SetTypeId (ItuR1411LosPropagationLossModel::GetTypeId());
			//modelFactory.Set ("Frequency", DoubleValue(1.8e9));

			//lteHelper->GetDownlinkSpectrumChannel()->AddPropagationLossModel(modelFactory.Create<PropagationLossModel>());
			//lteHelper->GetUplinkSpectrumChannel()->AddPropagationLossModel(modelFactory.Create<PropagationLossModel>());
	    }

	 
		// Create a single RemoteHost
		NodeContainer remoteHostContainer;
		remoteHostContainer.Create (1);
		Ptr<Node> remoteHost = remoteHostContainer.Get (0);
		InternetStackHelper internet;
		internet.Install (remoteHost);

		// Create the Internet
		PointToPointHelper p2ph;
		p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
		p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1400)); //default 1500
		p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));   //0.010
		NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);


		Ipv4AddressHelper ipv4h;
		ipv4h.SetBase ("10.1.0.0", "255.255.0.0");
		Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
		// interface 0 is localhost, 1 is the p2p device
		Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

		Ipv4StaticRoutingHelper ipv4RoutingHelper;
		Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
		remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
	  
		// Create node containers: UE, UE Overloaded Group ,  eNodeBs, UABSs.
		//NodeContainer ueNodes;
		NodeContainer ueNodesN;
        ueNodes = ueNodesN;
		ueNodes.Create(numberOfUENodes);
		NodeContainer ueOverloadNodes;
		if (scen == 3 || scen == 4)
		{
			ueOverloadNodes.Create(numberOfOverloadUENodes);
		}
		NodeContainer enbNodes;
		enbNodes.Create(numberOfeNodeBNodes);
		NodeContainer UABSNodes;
		if (scen == 2 || scen == 4)
		{
			UABSNodes.Create(numberOfUABS);
		}

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

		Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();
		positionAlloc2->Add (Vector( 1500, 1500 , enBHeight));
		positionAlloc2->Add (Vector( 4500, 1500 , enBHeight));
		positionAlloc2->Add (Vector( 1500, 4500 , enBHeight));
		positionAlloc2->Add (Vector( 4500, 4500 , enBHeight));

		MobilityHelper mobilityenB;
		mobilityenB.SetMobilityModel("ns3::ConstantPositionMobilityModel");
		mobilityenB.SetPositionAllocator(positionAlloc2);
		mobilityenB.Install(enbNodes);

		//BuildingsHelper::Install (enbNodes);
		
		//---------------Set Power of eNodeBs------------------//  
		// Ptr<LteEnbPhy> enodeBPhy; 

		// for( uint16_t i = 0; i < enbLteDevs.GetN(); i++) 
		// {
		// 	enodeBPhy = enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>()->GetPhy();
		// 	enodeBPhy->SetTxPower(eNodeBTxPower);
			
		// }
		Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNodeBTxPower));
		Config::SetDefault( "ns3::LteEnbPhy::NoiseFigure", DoubleValue(5) );    // Default 5
		
		//--------------------Antenna parameters----------------------// 

		//--------------------Cosine Antenna--------------------------//
		// lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");  // CosineAntennaModel associated with an eNB device allows to model one sector of a macro base station
		// lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue(0)); //default is 0
		// lteHelper->SetEnbAntennaModelAttribute("Beamwidth", DoubleValue(60));
		// lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue(0.0)); //default 0

		//--------------------Parabolic Antenna  -- > to use with multisector cells.
		// lteHelper->SetEnbAntennaModelType ("ns3::ParabolicAntennaModel");
		// lteHelper->SetEnbAntennaModelAttribute ("Beamwidth",   DoubleValue (70));
		// lteHelper->SetEnbAntennaModelAttribute ("MaxAttenuation",     DoubleValue (20.0));

		//--------------------Isotropic Antenna--------------------------//
		lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");  //irradiates in all directions

		//-------------------Set frequency. This is important because it changes the behavior of the path loss model
   		lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(100));
    	lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(18100)); 
   		// lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (200));
		
		//-------------------Set Bandwith for enB-----------------------------//
		lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth_enb)); //Set Download BandWidth
		lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth_enb)); //Set Upload Bandwidth

		// ------------------- Install LTE Devices to the nodes --------------------------------//
		NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);


		NS_LOG_UNCOND("Installing Mobility Model in UEs...");

		// ------------------Install Mobility Model User Equipments-------------------//

		MobilityHelper mobilityUEs;
		mobilityUEs.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
									 "Mode", StringValue ("Time"),
									 "Time", StringValue ("1s"),//("1s"),
									 //"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=4.0]"),
									 //"Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
									 "Speed", StringValue ("ns3::UniformRandomVariable[Min=1.0|Max=4.0]"),
									 "Bounds", StringValue ("0|6000|0|6000"));
		// mobilityUEs.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
		// 	 							 "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"),
		// 								 "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"));
		mobilityUEs.SetPositionAllocator("ns3::RandomBoxPositionAllocator",  // to use OkumuraHataPropagationLossModel needs to be in a height greater then 0.
			 							 "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"),
										 "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6000.0]"),
										 "Z", StringValue ("ns3::UniformRandomVariable[Min=0.5|Max=1.50]"));
		//mobilityUEs.SetPositionAllocator(positionAllocUEs);
		mobilityUEs.Install(ueNodes);


		if (scen == 3 || scen == 4)
		{
			// ------------------Install Mobility Model User Equipments that will overload the enB-------------------//
		NS_LOG_UNCOND("Installing Mobility Model in Overloading UEs...");

		MobilityHelper mobilityOverloadingUEs;
		mobilityOverloadingUEs.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
									 "Mode", StringValue ("Time"),
									 "Time", StringValue ("1s"),//("1s"),
									 "Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=8.0]"),
									 "Bounds", StringValue ("0|1000|0|1000"));
		
		mobilityOverloadingUEs.SetPositionAllocator("ns3::RandomBoxPositionAllocator",  // to use OkumuraHataPropagationLossModel needs to be in a height greater then 0.
			 							 "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"), //old setup: 3000
										 "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"),
										 "Z", StringValue ("ns3::UniformRandomVariable[Min=0.5|Max=1.50]"));
		
		mobilityOverloadingUEs.Install(ueOverloadNodes);
		}
	  
		if (scen == 2 || scen == 4)
		{
			NS_LOG_UNCOND("Installing Mobility Model in UABSs...");
			// ----------------Install Mobility Model UABS--------------------//

			Ptr<ListPositionAllocator> positionAllocUABS = CreateObject<ListPositionAllocator> ();
			positionAllocUABS->Add (Vector( 1400, 1400 , enBHeight)); //1
			positionAllocUABS->Add (Vector( 4600, 1400 , enBHeight)); //2
			positionAllocUABS->Add (Vector( 1400, 4600 , enBHeight)); //3
			positionAllocUABS->Add (Vector( 4600, 4600 , enBHeight)); //4
			positionAllocUABS->Add (Vector( 1600, 1600 , enBHeight)); //5
			positionAllocUABS->Add (Vector( 4400, 1600 , enBHeight)); //6
			positionAllocUABS->Add (Vector( 1600, 4400 , enBHeight)); //7
			positionAllocUABS->Add (Vector( 4400, 4400 , enBHeight)); //8
			// have to add as many os UABS will be used in the simulation, in this example there are 8 UABS available 

			MobilityHelper mobilityUABS;
			mobilityUABS.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
			//mobilityUABS.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", 
			//                         "Mode", StringValue ("Time"),
			  //                       "Time", StringValue ("5s"),
									 //"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
				//                     "Speed", StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
				//		     "Bounds", StringValue ("0|2000|0|2000"));
			mobilityUABS.SetPositionAllocator(positionAllocUABS);
			// mobilityUABS.SetPositionAllocator ("ns3::GridPositionAllocator",
			// 								"MinX", DoubleValue (0.0),
			// 								"MinY", DoubleValue (0.0),
			// 								"DeltaX", DoubleValue (m_distance),
			// 								"DeltaY", DoubleValue (m_distance),
			// 								"GridWidth", UintegerValue (3),
			// 								"LayoutType", StringValue ("RowFirst"));

			mobilityUABS.Install(UABSNodes);
			UABSTxPower = 0;
			Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (UABSTxPower));
			Config::SetDefault( "ns3::LteEnbPhy::NoiseFigure", DoubleValue(5) );    // Default 5
		
		//--------------------Antenna parameters----------------------// 
		
		//--------------------Cosine Antenna--------------------------//
		// lteHelper->SetEnbAntennaModelType("ns3::CosineAntennaModel");
		// lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue(0));
		// lteHelper->SetEnbAntennaModelAttribute("Beamwidth", DoubleValue(60));
		// lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue(0.0));
		
		//--------------------Parabolic Antenna  -- > to use with multisector cells.
		// lteHelper->SetEnbAntennaModelType ("ns3::ParabolicAntennaModel");
		// lteHelper->SetEnbAntennaModelAttribute ("Beamwidth",   DoubleValue (70));
		// lteHelper->SetEnbAntennaModelAttribute ("MaxAttenuation",     DoubleValue (20.0));
		
		//--------------------Isotropic Antenna--------------------------------------//
		lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");

		//-------------------Set frequency. This is important because it changes the behavior of the path loss model
   		lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(100));
    	lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(18100)); 
   		// lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (200));
		
		//-------------------Set Bandwith for enB-----------------------------//
		lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth_UABS)); //Set Download BandWidth
		lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth_UABS)); //Set Upload Bandwidth

		}

			  

		// ------------------- Install LTE Devices to the nodes --------------------------------//
		NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
		NetDeviceContainer UABSLteDevs;
		NetDeviceContainer OverloadingUeLteDevs;
		if(scen == 2 || scen == 4)
		{
		UABSLteDevs = lteHelper->InstallEnbDevice (UABSNodes);
		}
		if(scen == 3 || scen == 4)
		{
		OverloadingUeLteDevs = lteHelper->InstallUeDevice (ueOverloadNodes);
		}

		
		if(scen != 0)
		{
		// ---------------Get position of enBs, UABSs and UEs. -------------------//
		Simulator::Schedule(Seconds(5), &GetPositionUEandenB,enbNodes,UABSNodes,enbLteDevs,UABSLteDevs,ueOverloadNodes,ueLteDevs);
		}


		//---------------------- Install the IP stack on the UEs (regular UE and Overloding-UE) ---------------------- //
		NodeContainer ues_all;
		
  		Ipv4InterfaceContainer ue_all_IpIfaces;
  		NetDeviceContainer ueDevs;
  		ues_all.Add (ueNodes);
  		ueDevs.Add (ueLteDevs);
  		if (scen  == 3 || scen  == 4 ) 
      	{
      	ues_all.Add (ueOverloadNodes);
      	ueDevs.Add (OverloadingUeLteDevs);
      	}
      	
      	internet.Install (ues_all);
      	ue_all_IpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
	 
	  //---------------------- Install the IP stack on the UEs---------------------- //
	  // internet.Install (ueNodes);
	  // Ipv4InterfaceContainer ueIpIface;
	  // ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
	  
	  // Assign IP address to UEs, and install applications
	  for (uint16_t i = 0; i < ueNodes.GetN(); i++) 
	  {
		Ptr<Node> ueNode = ueNodes.Get (i);
			// Set the default gateway for the UE
			Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
			ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
	  }

	  // ---------------------- Install the IP stack on the Overloading UEs -----------------------//
	  // internet.Install (ueOverloadNodes);
	  // Ipv4InterfaceContainer ueOverloadIpIface;
	  // ueOverloadIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (OverloadingUeLteDevs));
	  
	  // -------------------Assign IP address to Overloading UEs, and install applications ----------------------//
	  if (scen  == 3 || scen  == 4 )
	  {
		  for (uint16_t i = 0; i < ueOverloadNodes.GetN(); i++) 
		  {
			Ptr<Node> ueOverloadNode = ueOverloadNodes.Get (i);
				// Set the default gateway for the UE
				Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueOverloadNode->GetObject<Ipv4> ());
				ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
		  }
		}

		NS_LOG_UNCOND("Attaching Ues in enBs/UABSs...");
		// Attach one UE per eNodeB // ahora no es un UE por eNodeB, es cualquier UE a cualquier eNodeB
		//lteHelper->AttachToClosestEnb (ueLteDevs, enbLteDevs);
		lteHelper->Attach (ueLteDevs);
		
		// this enables handover for macro eNBs
		lteHelper->AddX2Interface (enbNodes); // X2 interface for macrocells
		
		if (scen == 2 || scen == 4)
		{
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
				//NS_LOG_UNCOND("Creating X2 Interface between UABS " << UABSNodes.Get(i) << " and enB " << enbNodes.Get(j));
			}
		}
		}

		
		// -----------------------Activate EPSBEARER---------------------------//
		lteHelper->ActivateDedicatedEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
	  
	  	//------------------------Get Sinr-------------------------------------//
	  	if(scen != 0)
		{
		Simulator::Schedule(Seconds(5), &GetSinrUE,ueLteDevs,ueNodes, ueOverloadNodes, OverloadingUeLteDevs);
		}

		//----------------Run Python Command to get centroids------------------------//
		if (scen == 2 || scen == 4)
		{
			Simulator::Schedule(Seconds(6), &GetPrioritizedClusters, UABSNodes,  speedUABS,  UABSLteDevs);
		}

		//Scenario A: Failure of an enB, overloads the system (the other enBs):
		if (scen == 1 || scen == 2)
		{	
				
			//Simulator::Schedule(Seconds(30), &enB_Failure,enbLteDevs,ueLteDevs,lteHelper,enBpowerFailure);
			Simulator::Schedule(Seconds(10), &enB_Failure,enbLteDevs,ueLteDevs,lteHelper,enBpowerFailure);
		}


		//Scenario B: enB overloaded, overloads the system:
		if (scen == 3 || scen == 4)
		{	
	
			Simulator::Schedule(Seconds(10),&enB_Overload, lteHelper, OverloadingUeLteDevs, enbLteDevs); //estaba en 10 segundos
			//enB_Overload(lteHelper, OverloadingUeLteDevs, enbLteDevs);
		}

		// ---------------------- Setting video transmition - Start sending-receiving -----------------------//
		//NS_LOG_UNCOND("Resquesting-sending Video...");
	  	NS_LOG_INFO ("Create Applications.");
	   
	  	//requestVideoStream(remoteHost, ueNodes, remoteHostAddr, simTime);//, transmissionStart);
	  	UDPApp(remoteHost, ueNodes, remoteHostAddr, ue_all_IpIfaces);

	  	if (scen == 3 || scen == 4)
		{	
	
			//Simulator::Schedule(Seconds(11),&requestVideoStream, remoteHost, ueOverloadNodes, remoteHostAddr, simTime); //estaba en 11 segundos
			
		}




	  	// ---------------------- Configuration of Netanim  -----------------------//
		AnimationInterface anim ("UOSLTE_run_"+std::to_string(z)+".xml"); // Mandatory
		anim.SetMaxPktsPerTraceFile(5000000); // Set animation interface max packets. (TO CHECK: how many packets i will be sending?) 
		// Cor e Descrição para eNb
		for (uint32_t i = 0; i < enbNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(enbNodes.Get(i), "eNb");
			anim.UpdateNodeColor(enbNodes.Get(i), 0, 255, 0);
			anim.UpdateNodeSize(enbNodes.Get(i)->GetId(),300,300); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < ueNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(ueNodes.Get(i), "UEs");
			anim.UpdateNodeColor(ueNodes.Get(i),  255, 0, 0);
			anim.UpdateNodeSize(ueNodes.Get(i)->GetId(),100,100); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < ueOverloadNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(ueOverloadNodes.Get(i), "UEs OL");
			anim.UpdateNodeColor(ueOverloadNodes.Get(i),  255, 0, 0);
			anim.UpdateNodeSize(ueOverloadNodes.Get(i)->GetId(),100,100); // to change the node size in the animation.
		}
		for (uint32_t i = 0; i < UABSNodes.GetN(); ++i) 
		{
			anim.UpdateNodeDescription(UABSNodes.Get(i), "UABS");
			anim.UpdateNodeColor(UABSNodes.Get(i), 0, 0, 255);
			anim.UpdateNodeSize(UABSNodes.Get(i)->GetId(),200,200); // to change the node size in the animation.
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
		Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",MakeCallback (&NotifyHandoverStartEnb));
		Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",MakeCallback (&NotifyHandoverStartUe));
		Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",MakeCallback (&NotifyHandoverEndOkEnb));
		Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",MakeCallback (&NotifyHandoverEndOkUe));



		//Gnuplot parameters for Throughput
		string fileNameWithNoExtension = "Throughput_run_";
		string graphicsFileName        = fileNameWithNoExtension + std::to_string(z) +".png";
		string plotFileName            = fileNameWithNoExtension + std::to_string(z)+".plt";
		string plotTitle               = "Throughput vs Time";
		string dataTitle               = "Throughput";
		//Gnuplot parameters for PDR
		string fileNameWithNoExtensionPDR = "PDR_run_";
		string graphicsFileNamePDR        = fileNameWithNoExtensionPDR + std::to_string(z) +".png";
		string plotFileNamePDR            = fileNameWithNoExtensionPDR + std::to_string(z)+".plt";
		string plotTitlePDR               = "PDR Mean"; //to check later
		string dataTitlePDR               = "Packet Delivery Ratio Mean";
		//Gnuplot parameters for PLR
		string fileNameWithNoExtensionPLR = "PLR_run_";
		string graphicsFileNamePLR        = fileNameWithNoExtensionPLR + std::to_string(z) +".png";
		string plotFileNamePLR            = fileNameWithNoExtensionPLR + std::to_string(z)+".plt";
		string plotTitlePLR               = "PLR Mean";
		string dataTitlePLR               = "Packet Lost Ratio Mean";

		//Gnuplot parameters for APD
		string fileNameWithNoExtensionAPD = "APD_run_";
		string graphicsFileNameAPD        = fileNameWithNoExtensionAPD + std::to_string(z) +".png";
		string plotFileNameAPD            = fileNameWithNoExtensionAPD + std::to_string(z)+".plt";
		string plotTitleAPD              = "APD Mean";
		string dataTitleAPD               = "Average Packet Delay Mean";

		//Gnuplot parameters for APD
		string fileNameWithNoExtensionJitter = "Jitter_run_";
		string graphicsFileNameJitter       = fileNameWithNoExtensionJitter + std::to_string(z) +".png";
		string plotFileNameJitter            = fileNameWithNoExtensionJitter + std::to_string(z)+".plt";
		string plotTitleJitter              = "Jitter Mean";
		string dataTitleJitter              = "Jitter Mean";

		// Instantiate the plot and set its title.
		//Throughput
		Gnuplot gnuplot (graphicsFileName);
		gnuplot.SetTitle (plotTitle);
		//PDR
		Gnuplot gnuplotPDR (graphicsFileNamePDR);
		gnuplotPDR.SetTitle (plotTitlePDR);
		//PLR
		Gnuplot gnuplotPLR (graphicsFileNamePLR);
		gnuplotPLR.SetTitle (plotTitlePLR);
		//APD
		Gnuplot gnuplotAPD (graphicsFileNameAPD);
		gnuplotAPD.SetTitle (plotTitleAPD);
		//Jitter
		Gnuplot gnuplotJitter (graphicsFileNameJitter);
		gnuplotJitter.SetTitle (plotTitleJitter);

		// Make the graphics file, which the plot file will be when it is used with Gnuplot, be a PNG file.
		//Throughput
		gnuplot.SetTerminal ("png");
		//PDR
		gnuplotPDR.SetTerminal ("png");
		//PLR
		gnuplotPLR.SetTerminal ("png");
		//APD
		gnuplotAPD.SetTerminal ("png");
		//Jitter
		gnuplotJitter.SetTerminal ("png");

		// Set the labels for each axis.
		//Throughput
		gnuplot.SetLegend ("Time (Seconds)", "Throughput");
		//PDR
		gnuplotPDR.SetLegend ("Time (Seconds)", "Packet Delivery Ratio (%)");
		//PLR
		gnuplotPLR.SetLegend ("Time (Seconds)", "Packet Lost Ratio (%)");
		//APD
		gnuplotAPD.SetLegend ("Time (Seconds)", "Average Packet Delay (%)");
		//Jitter
		gnuplotJitter.SetLegend ("Time (Seconds)", "Jitter (%)");

		Gnuplot2dDataset datasetThroughput;
		Gnuplot2dDataset datasetPDR;
		Gnuplot2dDataset datasetPLR;
		Gnuplot2dDataset datasetAPD;

		datasetThroughput.SetTitle (dataTitle);
		datasetPDR.SetTitle (dataTitlePDR);
		datasetPLR.SetTitle (dataTitlePLR);
		datasetAPD.SetTitle (dataTitleAPD);
		datasetAvg_Jitter.SetTitle (dataTitleJitter);

		datasetThroughput.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datasetPDR.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datasetPLR.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datasetAPD.SetStyle (Gnuplot2dDataset::LINES_POINTS);
		datasetAvg_Jitter.SetStyle (Gnuplot2dDataset::LINES_POINTS);

		//Flow Monitor Setup
		FlowMonitorHelper flowmon;
		//Ptr<FlowMonitor> monitor = flowmon.InstallAll();

		//Prueba 1219: intentar solo capturar el trafico de los nodos y el remotehost
		 Ptr<FlowMonitor> monitor;
		 monitor = flowmon.Install(ueNodes);
		 monitor = flowmon.Install(remoteHostContainer); //remoteHostContainer
		// monitor = flowmon.Install(enbNodes);

		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
		Simulator::Schedule(Seconds(1),&ThroughputCalc, monitor,classifier,datasetThroughput,datasetPDR,datasetPLR,datasetAPD);
		
		Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
		if (remMode > 0){
		remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
		remHelper->SetAttribute ("XMin", DoubleValue (0.0));
		remHelper->SetAttribute ("XMax", DoubleValue (6000.0));
		remHelper->SetAttribute ("XRes", UintegerValue (1500));
		remHelper->SetAttribute ("YMin", DoubleValue (0.0));
		remHelper->SetAttribute ("YMax", DoubleValue (6000.0));
		remHelper->SetAttribute ("YRes", UintegerValue (1500));
		remHelper->SetAttribute ("Z", DoubleValue (1.0));
		remHelper->SetAttribute ("Bandwidth", UintegerValue (100));
		//remHelper->SetAttribute ("StopWhenDone", BooleanValue (true));
		remHelper->SetAttribute ("MaxPointsPerIteration", UintegerValue (50000));
		if(remMode == 1){
			remHelper->SetAttribute ("OutputFile", StringValue ("rem-noUABs.out"));
			Simulator::Schedule (Seconds (1.0),&RadioEnvironmentMapHelper::Install,remHelper);
		} else {
			remHelper->SetAttribute ("OutputFile", StringValue ("rem-withUABs.out"));
			Simulator::Schedule (Seconds (20.0),&RadioEnvironmentMapHelper::Install,remHelper);
			}
		}

		NS_LOG_UNCOND("Running simulation...");
		NS_LOG_INFO ("Run Simulation.");
		Simulator::Stop(Seconds(simTime));

		Simulator::Run ();

		// Print per flow statistics
		ThroughputCalc(monitor,classifier,datasetThroughput,datasetPDR,datasetPLR,datasetAPD);
		monitor->SerializeToXmlFile("UOSLTE-FlowMonitor_run_"+std::to_string(z)+".xml",true,true);

		//Gnuplot ...continued
 		//Throughput
		gnuplot.AddDataset (datasetThroughput);
		//PDR
		gnuplotPDR.AddDataset (datasetPDR);
		//PLR
		gnuplotPLR.AddDataset (datasetPLR);
		//APD
		gnuplotAPD.AddDataset (datasetAPD);
		//Jitter
		gnuplotJitter.AddDataset (datasetAvg_Jitter);

		// Open the plot file.
		//Throughput
		ofstream plotFileThroughtput (plotFileName.c_str());
		// Write the plot file.
		gnuplot.GenerateOutput (plotFileThroughtput);
		// Close the plot file.
		plotFileThroughtput.close ();
		
		//PDR
		ofstream plotFilePDR (plotFileNamePDR.c_str());
		// Write the plot file.
		gnuplotPDR.GenerateOutput (plotFilePDR);
		// Close the plot file.
		plotFilePDR.close ();

		//PLR
		ofstream plotFilePLR (plotFileNamePLR.c_str());
		// Write the plot file.
		gnuplotPLR.GenerateOutput (plotFilePLR);
		// Close the plot file.
		plotFilePLR.close ();

		//APD
		ofstream plotFileAPD (plotFileNameAPD.c_str());
		// Write the plot file.
		gnuplotAPD.GenerateOutput (plotFileAPD);
		// Close the plot file.
		plotFileAPD.close ();

		//Jitter
		ofstream plotFileJitter (plotFileNameJitter.c_str());
		// Write the plot file.
		gnuplotJitter.GenerateOutput (plotFileJitter);
		// Close the plot file.
		plotFileJitter.close ();


		UE_UABS.close();
		UABS_Qty.close();

		Simulator::Destroy ();
	  
		NS_LOG_INFO ("Done.");
	}
		return 0;
	}
	 
