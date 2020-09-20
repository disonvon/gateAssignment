#include <set>
#include <vector>
#include <algorithm>

#include "../inc/gam.h"
#include "../inc/DataManager.h"
#include  "../inc/ParameterRegistry.h"

gam::gam(const IloEnv& env)
   : _env(env)
{

}

//gam::gam():
//_env()
//{
//
//}

gam::~gam()
{
   
}


const int gam::PaxWalkTime[7][7] = { { 10, 15, 20, 25, 20, 25, 25 },
{ 15, 10, 15, 20, 15, 20, 20 },
{ 20, 15, 10, 25, 20, 25, 25 },
{ 25, 20, 25, 10, 15, 20, 20 },
{ 20, 15, 20, 15, 10, 15, 15 },
{ 25, 20, 25, 20, 15, 10, 20 },
{ 25, 20, 25, 20, 15, 20, 10 } };


const std::map<int, int> gam::flowTransitTime = { {0, 15}, {1, 28}, {2, 28}, {3, 15}, {4, 35}, {5, 48}, {6, 48}, {7, 35}, {8, 35},
                                                         {9, 48}, {10, 48}, {11, 61}, {12, 20}, {13, 38}, {14, 38}, {15, 20}};

std::stringstream gam::values;

void gam::initMaps()
{

   // Vector mapping for puck, gate within gam module

   // sort puck based on puckID
   std::stable_sort(pucks.begin(), pucks.end(), Puck::sortAlphabeticPtr());
   // aircraft information
   const int nPuck = pucks.size();
   for (int j = 0; j < nPuck; j++)
   {
      puckIndex[pucks[j]->getpuckId()] = j;
      puckMap[pucks[j]->getpuckId()] = pucks[j];
   }

   // gate information
   std::stable_sort(gates.begin(), gates.end(), Gate::sortAlphabeticPtr());
   int nGates = gates.size();
   for (int j = 0; j < nGates; j++)
   {
      gateIndex[gates[j]->getgateID()] = j;
      gateMap[gates[j]->getgateID()] = gates[j];
   }

   // ticket information
   std::stable_sort(tickets.begin(), tickets.end(), Ticket::sortAlphabeticPtr());
   int nTickets = tickets.size();
   for (int j = 0; j < nTickets; j++)
   {
      ticketIndex[tickets[j]->getpaxID()] = j;
      ticketMap[tickets[j]->getpaxID()] = tickets[j];
   }

}



void gam::initValidVariableAsgnIndex()
{
   int itr1 = 0;
   int k = 0;
   const int nPucks = pucks.size();
   for (auto gate : gates)
   {

      for (auto puck : pucks)
      {
         // recorder valid assign index for each puck-gate
         if ((gate->getarrType().find(puck->getarrType()) != std::string::npos) && (gate->getdepType().find(puck->getdepType()) != std::string::npos)
            && (puck->getpuckBodyType() == gate->getbodyType()))
         {
            int puckIdx = puckIndex[puck->getpuckId()];      // determine puckIdx
            int gateIdx = gateIndex[gate->getgateID()];      // determine puckIdx

            int j = itr1 * nPucks + puckIdx;              // determine corresponding index
            validGateAsgnIndex[j] = k;
            validAsgnIndex[k] = j;
            puckGateMap[puckIdx].insert(gateIdx);
            k++;
         }
      }
      itr1++;
   }
}



void gam::initValidPuckTermIndex()
{
   int nPucks = pucks.size();


   for (auto const & ivalidAsssign : validGateAsgnIndex)
   {
      int iGateIdx = ivalidAsssign.first / nPucks;
      int iPuckIdx = ivalidAsssign.first % nPucks;


      // two sets indicates which terminal can be assigned with which pucks
      if (iGateIdx < 41)//hard code here, will improve it later
      {
         if (puckIDtoS.find(iPuckIdx) == puckIDtoS.end())
         {
            puckIDtoS.insert(iPuckIdx);
         }

      }
      else
      {
         if (puckIDtoT.find(iPuckIdx) == puckIDtoT.end())
         {
            puckIDtoT.insert(iPuckIdx);
         }
      }

      int regionNum = -1;
      if (gates[iGateIdx]->getterminal() == "T" && gates[iGateIdx]->getregion() == "North")
      {
         regionNum = 0;//T_North-T_North
      }
      else if (gates[iGateIdx]->getterminal() == "T" && gates[iGateIdx]->getregion() == "Center")
      {
         regionNum = 1;//
      }
      else if (gates[iGateIdx]->getterminal() == "T" && gates[iGateIdx]->getregion() == "South")
      {
         regionNum = 2;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" && gates[iGateIdx]->getregion() == "North")
      {
         regionNum = 3;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" && gates[iGateIdx]->getregion() == "Center")
      {
         regionNum = 4;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" && gates[iGateIdx]->getregion() == "South")
      {
         regionNum = 5;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" && gates[iGateIdx]->getregion() == "East")
      {
         regionNum = 6;//
      }

      //add puck-gate combination with region information
      puckIDtoRegion[ivalidAsssign.first] = regionNum;
      puckRegion[iPuckIdx].insert(regionNum);//not used yet
   }

}



void gam::initModel()
{
   //local parameters
   const int nPuckGates = validGateAsgnIndex.size();
   const int nPucks = pucks.size();//just a large number
   const short nGates = gates.size();
   const int nPuckTermS = puckIDtoS.size();
   const int nPuckTernT = puckIDtoT.size();
   const int nRegions = 7;

   int nPuckRegions = 0;
   for (auto & iPuckRegion : puckRegion)
   {
      nPuckRegions += iPuckRegion.second.size();
   }


   //const int nPuckRegions = nPucks * nRegions;
   int nTicketRegions = 0;

   for (auto & iTicket: ticketsPuckIdx)
   {
      int firstPuck, secondPuck = 0;
      firstPuck = iTicket.second.first;
      secondPuck = iTicket.second.second;
      nTicketRegions += puckRegion[firstPuck].size() * puckRegion[secondPuck].size();

   }

   try
   {
      _model = IloModel(_env);
      _obj = IloAdd(_model, IloMinimize(_env));

      //variable declaration
      PuckAssign = IloIntVarArray(_env, nPuckGates, 0, 1);
      PuckRemote = IloIntVarArray(_env, nPucks, 0, 1);
      GatesUsed = IloNumVarArray(_env, nGates + 1, 0, 1);
      if (ParameterRegistry::instance()->objFunction == 2)
      {
         PuckLoationSIndicator = IloIntVarArray(_env, nPuckTermS, 0, 1);
         PuckLoationTIndicator = IloIntVarArray(_env, nPuckTernT, 0, 1);
      }
      else if (ParameterRegistry::instance()->objFunction == 3)
      {
         PuckRegionIndicator = IloIntVarArray(_env, nPuckRegions, 0, 1);
         TicketRegionPairIndicator = IloIntVarArray(_env, nTicketRegions, 0, 1);
      }



      //constraints initialization
      PuckCover = IloRangeArray(_env, 0);
      GateBinding = IloRangeArray(_env, 0);
      PuckGroundConnectionTime = IloRangeArray(_env, 0);
      PuckLocationConstrs = IloRangeArray(_env, 0);
      PuckRegionConstrs = IloRangeArray(_env, 0);
      TicketRegionConstrs = IloRangeArray(_env, 0);


   }
   catch (IloException&)
   {
      std::cout << "----------initialize model failed---------------------" << std::endl;
   }
}


void gam::initObjective()
{
   // local parameters
   try
   {
      const int nPucks = pucks.size();

      IloExpr objExpr(_env);

      //add puck assignment
      addPuckAssignTerm(objExpr);

      //add puck cancellation or add remote puck assignment
      addPuckRemote(objExpr);

      //add number of each gate used term
      addGateNumberTerm(objExpr);

      ////add puck location term
      if (ParameterRegistry::instance()->objFunction == 2)
      {
         addPuckLocationTerm(objExpr);
      }

      if (ParameterRegistry::instance()->objFunction == 3)
      {
         addPuckRegionTerm(objExpr);
         addTicketRegionTerm(objExpr);
      }
      //add objective function expression to ilog cplex class
      _obj.setExpr(objExpr);

      //delete temporary IloExpr - memory management
      objExpr.end();
   }
   catch (IloException&)
   {
      std::cout << "-----------add objective falied------------------" << std::endl;
   }

}

void gam::initConstraints()
{
   try
   {

      formulatePuckCoverConstr();
      formulateGateUsedConstr();
      formualteGroundConnctConstr();

      if (ParameterRegistry::instance()->objFunction == 2) 
      {
         formualtePuckLocationConstr();
      }

      if (ParameterRegistry::instance()->objFunction == 3)
      {
         formualtePuckRegionConstr();
         formualteTicketRegionConstr();
      }
   }
   catch (IloException)
   {
      std::cout << "----------initialize constraints failed---------------------" << std::endl;

   }
}


void gam::addPuckAssignTerm(IloExpr &objExpr)
{
   //determin objective function value
   const int nPucks = pucks.size();

   for (const auto & iValidGateAssign : validGateAsgnIndex)
   {
      int iGateIdx = iValidGateAssign.first / nPucks;
      int iPuckIdx = iValidGateAssign.first % nPucks;

      int j = iValidGateAssign.second;

      objExpr += ParameterRegistry::instance()->normalGateAssignBonus * PuckAssign[j];

      //set variable name
      std::string varName = "PuckAssign_" + gates[iGateIdx]->getgateID() + "_" + std::to_string(pucks[iPuckIdx]->getarrMinute()) + 
         "_" + std::to_string(pucks[iPuckIdx]->getdepMinute()) + "_" + pucks[iPuckIdx]->getpuckId();
      PuckAssign[j].setName(varName.c_str());
   }
}


void gam::addPuckLocationTerm(IloExpr& objExpr)
{
   int itr1 = 0;
   for (auto puckTerm: puckIDtoS)
   {
      //set variable name
      //auto idx = std::distance(puckIDtoS.begin(), puckIDtoS.find(puckTerm));
      std::string varName = pucks[puckTerm]->getpuckId() + "Term_S";
      PuckLoationSIndicator[itr1].setName(varName.c_str());
      ++itr1;
   }

   int itr2 = 0;
   for (auto puckTerm : puckIDtoT)
   {
      //set variable name
      //auto idx = std::distance(puckIDtoT.begin(), puckIDtoT.find(puckTerm));
      std::string varName = pucks[puckTerm]->getpuckId() + "Term_T";
      PuckLoationTIndicator[itr2].setName(varName.c_str());
      ++itr2;
   }


   int numConnectPax = 0;
   for (auto ticket : tickets)
   {

      if (ticketsPuckIdx.find(ticket->getpaxID()) != ticketsPuckIdx.end() )
      {
         numConnectPax += ticket->getPaxNum();
         int arrPuckIdx = ticketsPuckIdx[ticket->getpaxID()].first;
         int depPuckIdx = ticketsPuckIdx[ticket->getpaxID()].second;

         //get domestic/internation information for arrvial/departure

         std::string arrType = pucks[arrPuckIdx]->getarrType();
         std::string depType = pucks[depPuckIdx]->getdepType();

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) && (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 45;
            }
            else
            {
               turnTime = 20;
            }

            objExpr += PuckLoationSIndicator[idxArr] * PuckLoationSIndicator[idxDep] * turnTime * ticket->getPaxNum() * ParameterRegistry::instance()->paxConnectPenaltyPerMinute;
         }

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) && (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 40;
            }
            else
            {
               turnTime = 30;
            }

            objExpr += PuckLoationSIndicator[idxArr] * PuckLoationTIndicator[idxDep] * turnTime * ticket->getPaxNum()* ParameterRegistry::instance()->paxConnectPenaltyPerMinute;
         }

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) && (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 35;
            }
            else
            {
               turnTime = 20;
            }

            objExpr += PuckLoationTIndicator[idxArr] * PuckLoationTIndicator[idxDep] * turnTime * ticket->getPaxNum() * ParameterRegistry::instance()->paxConnectPenaltyPerMinute;
         }

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) && (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 40;
            }
            else
            {
               turnTime = 30;
            }

            objExpr += PuckLoationTIndicator[idxArr] * PuckLoationSIndicator[idxDep] * turnTime * ticket->getPaxNum() * ParameterRegistry::instance()->paxConnectPenaltyPerMinute;
         }
      }
   }

   std::cout << "total number connection Pax: " << numConnectPax << std::endl;
}


void gam::addPuckRemote(IloExpr& objExpr)
{
   int j = 0;
   for (const auto puck : pucks)
   {


      objExpr += ParameterRegistry::instance()->remoteGateAssignBonus * PuckRemote[j];

      std::string varName = "PuckRemote_" + puck->getpuckId();
      PuckRemote[j].setName(varName.c_str());
      j++;
   }
}

void gam::addPuckRegionTerm(IloExpr& objExpr)
{

   int i = 0;

   for (auto & iPuckRegion : puckRegion)
   {
      int puckIdx = iPuckRegion.first;
      for (auto & jRegion : iPuckRegion.second)
      {
         int regionIdx = jRegion;
         std::string varName = pucks[puckIdx]->getpuckId() + "_" + std::to_string(regionIdx);
         PuckRegionIndicator[i].setName(varName.c_str());
         ++i;
      }

   }
}


void gam::addTicketRegionTerm(IloExpr& objExpr)
{
   int i = 0;
   int flightTransitTime;
   for (auto const & ticketPuck : ticketsPuckIdx)
   {
      int arrPuckIdx = ticketsPuckIdx[ticketPuck.first].first;
      int depPuckIdx = ticketsPuckIdx[ticketPuck.first].second;

      auto arrPuck = pucks[arrPuckIdx];
      auto depPuck = pucks[depPuckIdx];

      flightTransitTime = depPuck->getdepMinute() - arrPuck->getarrMinute();
      std::string arrType = arrPuck->getarrType();
      std::string depType = depPuck->getdepType();

      int caseStartIdx = 0;
      if (arrType == "D" && depType == "D")
      {
         caseStartIdx = 0;
      }
      else if (arrType == "D" && depType == "I")
      {
         caseStartIdx = 4;
      }
      else if (arrType == "I" && depType == "D")
      {
         caseStartIdx = 8;
      }
      else if (arrType == "I" && depType == "I")
      {
         caseStartIdx = 12;
      }
      
      for (auto arrRegion : puckRegion[arrPuckIdx])
      {
         int walkTime, tempFlowTrainTime = 0;

         for (auto depRegion : puckRegion[depPuckIdx])
         {
            int addIdx;
            if (arrRegion < 2 && depRegion < 2)
            {
               addIdx = 0;
            }
            else if (arrRegion < 2 && depRegion >= 2)
            {
               addIdx = 1;
            }
            else if (arrRegion >= 2 && depRegion < 2)
            {
               addIdx = 2;
            }
            else
            {
               addIdx = 3;
            }
            std::string varName = ticketPuck.first + "_" + std::to_string(arrRegion) + "_" + std::to_string(depRegion);

            TicketRegionPairIndicator[i].setName(varName.c_str());

            walkTime = PaxWalkTime[arrRegion][depRegion];
            tempFlowTrainTime = flowTransitTime.find(addIdx + caseStartIdx)->second;
            objExpr += (TicketRegionPairIndicator[i] * (walkTime + tempFlowTrainTime)) / flightTransitTime;
            ++i;
            
         }
         
      }

   }
}






void gam::formulatePuckCoverConstr()
{
   int nPucks = pucks.size();
   int i = 0;

   IloNumExprArray temExprGatePuck(_env);

   for (int i = 0; i < pucks.size(); i++)
   {
      temExprGatePuck.add(PuckRemote[i]);
   }


   for (const auto & iValidGateAsgnIndex : validGateAsgnIndex)
   {
      int iPuckIdx = iValidGateAsgnIndex.first % nPucks;

      temExprGatePuck[iPuckIdx] += PuckAssign[iValidGateAsgnIndex.second];

   }


   for (int i = 0; i < pucks.size(); i++)
   {
      // determine rhsValue based on inputted data
      int rhsValue = 1;
      PuckCover.add(temExprGatePuck[i] == rhsValue);
      //set constraint name
      std::string cName = "PuckCover_" + pucks[i]->getpuckId();
      PuckCover[i].setName(cName.c_str());
   }

   temExprGatePuck.end();
   IloAdd(_model, PuckCover);
}


void gam::exportModel(const std::string & fileName)
{
   std::cout << "Exporting Gate Assignment Model ... " << std::endl;

   if (!modelExtracted)
   {
      // Extract assembled model and solve using Cplex
      _algo = IloCplex(_model);
      modelExtracted = true;
   }
   _algo.exportModel(fileName.c_str());
}


void gam::solveInitModel()
{
   _modelSolved = false;
   try
   {
      if (!modelExtracted)
      {
         // Extract assembled model and solve using Cplex
         _algo = IloCplex(_model);
         modelExtracted = true;
         //_algo.setOut(solver_srm);
      }
      _algo.setParam(IloCplex::TiLim, ParameterRegistry::instance()->timeLimitation);

      if (_algo.solve())
      {
         _modelSolved = true;
         gam_obj = _algo.getObjValue();

         IloNumArray puckAssign(_env);

         //extract mip solution
         _algo.getValues(PuckAssign, puckAssign);
         updateSolutionGatePuckAssigned(puckAssign);

         //debug only
         if (ParameterRegistry::instance()->dataAnalysis == 1)
         {
            std::ofstream solutionStrm;
            const std::string debugSolutionFileName = "mipsolution.csv";
            solutionStrm.open(debugSolutionFileName.c_str());
            if (solutionStrm.good())
            {
               solutionStrm << "PUCKID, GATEID, START, END" << std::endl;

               for (int iGate = 0; iGate < gates.size(); ++iGate)
               {
                  auto iGateAssigned = solutionGateAssginedPuck[iGate];
                  for (const auto iPuck : solutionGateAssginedPuck[iGate])
                  {
                     solutionStrm << pucks[iPuck]->getpuckId() << "," <<
                        gates[iGate]->getgateID() << "," << pucks[iPuck]->getarrMinute() << "," << pucks[iPuck]->getdepMinute() << std::endl;

                  }
               }

            }
            solutionStrm.close();
         }
         puckAssign.end();
      }
      else
      {
         _modelSolved = false;
         gam_obj = -1;

      }
   }
   catch (IloException)
   {
      std::cout << "Failed to solve Model ... " << std::endl;
   }
}



void gam::exportSolution(const std::string& filename)
{
   if(_modelSolved)
   {
      _algo.writeSolution(filename.c_str());
   }
}


void gam::addGateNumberTerm(IloExpr& objExpr)
{
   //determin objective function value

   int itr1 = 0;

   for (const auto gate : gates)
   {
      objExpr += ParameterRegistry::instance()->fixedGatePenalty * GatesUsed[itr1];

      //set variable name
      std::string varName = "Gate_" + gate->getgateID();
      GatesUsed[itr1].setName(varName.c_str());
      GatesUsed[itr1].setLB(0);
      GatesUsed[itr1].setUB(1);

      itr1++;
   }

   //add extra remote gate used penalty
   objExpr += ParameterRegistry::instance()->remoteFixedGatePenalty*GatesUsed[itr1];
   //set variable name
   std::string varName = "Gate_Remote";
   GatesUsed[itr1].setName(varName.c_str());
   GatesUsed[itr1].setLB(0);
   GatesUsed[itr1].setUB(1);

}



void gam::formualtePuckLocationConstr()
{
   int nPucks = pucks.size();
   IloNumExprArray temExprTermS(_env);
   IloNumExprArray temExprTermT(_env);

   for (int i = 0; i < puckIDtoS.size(); i++)
   {
      temExprTermS.add(PuckLoationSIndicator[i]);
   }

   for (int i = 0; i < puckIDtoT.size(); i++)
   {
      temExprTermT.add(PuckLoationTIndicator[i]);
   }

   for (auto & iValidGateAsgnIndex : validGateAsgnIndex)
   {
      int iGateIdx = iValidGateAsgnIndex.first / nPucks;
      int iPuckIdx = iValidGateAsgnIndex.first % nPucks;


      if (iGateIdx < 41)//hard code here, will improve it later: Term S or term T
      {

         auto idx = std::distance(puckIDtoS.begin(), puckIDtoS.find(iPuckIdx));
         temExprTermS[idx] -= PuckAssign[iValidGateAsgnIndex.second];

      }
      else
      {
         auto idx = std::distance(puckIDtoT.begin(), puckIDtoT.find(iPuckIdx));
         temExprTermT[idx] -= PuckAssign[iValidGateAsgnIndex.second];

      }
   }


   //set constraint name and add constraints
   int i = 0;
   int rhsValue = 0;
   for (auto idx : puckIDtoS)
   {

      PuckLocationConstrs.add(temExprTermS[i] == rhsValue);

      std::string cNameS = "PuckLocationConstrs_" + pucks[idx]->getpuckId() + "Term_S";

      PuckLocationConstrs[i].setName(cNameS.c_str());
      ++i;
   }
   temExprTermS.end();

   int j = 0;
   for (auto idx : puckIDtoT)
   {

      PuckLocationConstrs.add(temExprTermT[j] == rhsValue);

      std::string cNameT = "PuckLocationConstrs_" + pucks[idx]->getpuckId() + "Term_T";

      PuckLocationConstrs[i+j].setName(cNameT.c_str());
      ++j;
   }
   temExprTermT.end();

   IloAdd(_model, PuckLocationConstrs);
}


void gam::formulateGateUsedConstr()
{

   int nPucks = pucks.size();

   IloNumExprArray temExprGateUsed(_env);

   for (const auto & iValidGateAsgnIndex : validGateAsgnIndex)
   {
      int iGateIdx = iValidGateAsgnIndex.first / nPucks;

      temExprGateUsed.add(GatesUsed[iGateIdx]);
   }

   int i = 0;
   for (const auto & iValidGateAsgnIndex : validGateAsgnIndex)
   {
      int iPuckIdx = iValidGateAsgnIndex.first % nPucks;
      int iGateIdx = iValidGateAsgnIndex.first / nPucks;

      temExprGateUsed[i] -= PuckAssign[iValidGateAsgnIndex.second];

      // determine rhsValue based on inputted data
      int rhsValue = 0;
      rhsGateBinding.push_back(rhsValue);
      GateBinding.add(temExprGateUsed[i] >= rhsValue);

      //set constraint name
      std::string cName = "GateBinding_" + gates[iGateIdx]->getgateID() + "_" + pucks[iPuckIdx]->getpuckId();
      GateBinding[i].setName(cName.c_str());

      ++i;
   }

   temExprGateUsed.end();

   //add remote gate assign index
   int nGates = gates.size();
   int j = 0;

   for (const auto puck : pucks)
   {
      IloExpr temExpr(_env);
      temExpr += GatesUsed[nGates];

      temExpr -= PuckRemote[j];
      j++;
      // determine rhsValue based on inputted data
      int rhsValue = 0;
      rhsGateBinding.push_back(rhsValue);
      GateBinding.add(temExpr >= rhsValue);

      //set constraint name
      std::string cName = "GateBinding_remote_puck_" + puck->getpuckId();
      GateBinding[i].setName(cName.c_str());
      temExpr.end();
      i++;
   }

   IloAdd(_model, GateBinding);
}



void gam::formualteGroundConnctConstr()
{

   int nPucks = pucks.size();

   int iConstrs = 0;

   for (const auto & iValidGateAsgnIndex : validGateAsgnIndex)
   {
      int iGateIdx = iValidGateAsgnIndex.first / nPucks;
      int iPuckIdx = iValidGateAsgnIndex.first % nPucks;

      int j = (iGateIdx + 1) * nPucks;
      for (const auto & jValidGateAsgnIndex : validGateAsgnIndex)
      {
         if (jValidGateAsgnIndex.first <= iValidGateAsgnIndex.first)
         {
            continue;
         }

         int jPuckIdx = jValidGateAsgnIndex.first % nPucks;

         int acConnectTime = 45;//aircraft connection time is configurable later
         if (((pucks[jPuckIdx]->getarrMinute() > pucks[iPuckIdx]->getarrMinute() - acConnectTime) && (pucks[jPuckIdx]->getarrMinute() < pucks[iPuckIdx]->getdepMinute() + acConnectTime)) ||
            ((pucks[jPuckIdx]->getdepMinute() > pucks[iPuckIdx]->getarrMinute() - acConnectTime) && (pucks[jPuckIdx]->getdepMinute() < pucks[iPuckIdx]->getdepMinute() + acConnectTime)) ||
            (pucks[jPuckIdx]->getarrMinute() <= pucks[iPuckIdx]->getarrMinute() - acConnectTime) && (pucks[jPuckIdx]->getdepMinute() >= pucks[iPuckIdx]->getdepMinute() + acConnectTime))

         {
            IloExpr temExpr(_env);

            temExpr += PuckAssign[iValidGateAsgnIndex.second];
            temExpr += PuckAssign[jValidGateAsgnIndex.second];
            int rhsValue = 1;
            // determine rhsValue based on inputted data
            PuckGroundConnectionTime.add(temExpr <= rhsValue);
            //set constraint name
            std::string cName = "PuckGroundConnectionTime_" + gates[iGateIdx]->getgateID() + "_" + pucks[iPuckIdx]->getpuckId() + "_" + pucks[jPuckIdx]->getpuckId();
            PuckGroundConnectionTime[iConstrs].setName(cName.c_str());
            temExpr.end();
            iConstrs++;
         }

         if (jValidGateAsgnIndex.first >= j)
         {
            break;
         }
      }
   }

   IloAdd(_model, PuckGroundConnectionTime);
}




void gam::formualtePuckRegionConstr()
{
   IloNumExprArray temExprPuckRegion(_env);
   for (int i = 0; i < PuckRegionIndicator.getSize(); i++)
   {
      temExprPuckRegion.add(PuckRegionIndicator[i]);
   }

   int ithPuckID = 0;
   for (auto & iPuckRegion : puckIDtoRegion)
   {
      int nthPuck = iPuckRegion.first % 303;
      int tempRegion = iPuckRegion.second;

      int j = 0;
      for (auto & iPuckRegion : puckRegion)
      {

         if (iPuckRegion.first == nthPuck)
         {
            int tempIdx = std::distance( iPuckRegion.second.begin(), iPuckRegion.second.find(tempRegion));
            j += tempIdx;
            temExprPuckRegion[j] -= PuckAssign[ithPuckID];

            break;
         }

         j += iPuckRegion.second.size();

      }

      ++ithPuckID;
   }

   int ithConstrs = 0;
   for (auto & iPuckRegion : puckRegion)
   {
      int puckIdx = iPuckRegion.first;
      for (auto & jRegion : iPuckRegion.second)
      {
         int regionIdx = jRegion;
         std::string cNameR = "PuckRegionConstrs_" + pucks[puckIdx]->getpuckId() + "_" + std::to_string(regionIdx);
         temExprPuckRegion[ithConstrs].setName(cNameR.c_str());
         PuckRegionConstrs.add(temExprPuckRegion[ithConstrs] == 0);
         ++ithConstrs;
      }

   }
   temExprPuckRegion.end();

   IloAdd(_model, PuckRegionConstrs);

}

void gam::formualteTicketRegionConstr()
{

   int i = 0;

   for ( auto const & ticketPuck : ticketsPuckIdx)
   {

      int arrPuckIdx = ticketsPuckIdx[ticketPuck.first].first;
      int depPuckIdx = ticketsPuckIdx[ticketPuck.first].second;

      int ithPuckRegion = 0;
      
      for (auto & iPuckRegion : puckRegion)
      {
         int puckIdx = iPuckRegion.first;
         if (puckIdx == arrPuckIdx)
         {
            break;
         }

         ithPuckRegion += iPuckRegion.second.size();
      }
      int jthPuckRegion = 0;

      for (auto & iPuckRegion : puckRegion)
      {
         int puckIdx = iPuckRegion.first;
         if (puckIdx == depPuckIdx)
         {
            break;
         }

         jthPuckRegion += iPuckRegion.second.size();
      }


      for (auto & arrRegion : puckRegion[arrPuckIdx])
      {
         int temjthPuckRegion = jthPuckRegion;
         for (auto & depRegion : puckRegion[depPuckIdx])
         {

            IloExpr temExpr(_env);

            temExpr += TicketRegionPairIndicator[i];
            temExpr -= PuckRegionIndicator[ithPuckRegion];
            temExpr -= PuckRegionIndicator[temjthPuckRegion];
            temExpr += 1;
            TicketRegionConstrs.add(temExpr >= 0);
            temExpr.end();
            //set constraint name
            std::string cName = ticketPuck.first + std::to_string(ithPuckRegion) + "_" + std::to_string(jthPuckRegion);
            TicketRegionConstrs[i].setName(cName.c_str());
            ++temjthPuckRegion;
            ++i;

         }
         ++ithPuckRegion;
      }
   }
   IloAdd(_model, TicketRegionConstrs);

}


void gam::querySolution()
{
   IloNumArray gateUsed(_env);
   IloNumArray puckAssign(_env);
   IloNumArray puckRemote(_env);
   IloNumArray puckLoationSIndicator(_env);
   IloNumArray puckLoationTIndicator(_env);


   if (!_modelSolved)
   {
      throw("[gam::querySolution] Must solve gam model before extracting solution.");
   }

   try
   {
      // extract final solution from algorithm class
      if (_modelSolved)
      {
         _algo.getValues(gateUsed, GatesUsed);
      }

      int numSGateUsed = 0;
      int numTGateUsed = 0;
      for (int i = 0; i < GatesUsed.getSize() - 1; ++i)
      {
         int val = _algo.getValue(GatesUsed[i]);
         if (i < 41)
         {
            numSGateUsed += gateUsed[i];
         }
         else
         {
            numTGateUsed += gateUsed[i];
         }
      }

      std::cout << "# gates used in Terminal S: " << numSGateUsed << std::endl;
      std::cout << "# gates used in Terminal T: " << numTGateUsed << std::endl;



   }
   catch (IloException&)
   {
      throw("[gam::querySolution] Solution extraction phase - gateused.");
   }


   try
   {
      // extract final solution from algorithm class
      if (_modelSolved) { _algo.getValues(puckAssign, PuckAssign); }


      int numPucksAssigned = 0;
      for (int i = 0; i < puckAssign.getSize(); ++i)
      {
         numPucksAssigned += puckAssign[i];
      }

      std::cout << "# pucks are successfully assigned: " << numPucksAssigned << std::endl;
   }
   catch (IloException&)
   {
      throw("[gam::querySolution] Solution extraction phase - puckAssign.");
   }

   try
   {
      // extract final solution from algorithm class
      if (_modelSolved) { _algo.getValues(puckRemote, PuckRemote); }

      int numRemotePuckUsed = 0;
      for (int i = 0; i < puckRemote.getSize(); ++i)
      {
         numRemotePuckUsed += puckRemote[i];
      }
      std::cout << "# pucks are unsuccessfully assigned: " << numRemotePuckUsed << std::endl;

   }
   catch (IloException&)
   {
      throw("[gam::querySolution] Solution extraction phase - puckRemote.");
   }

   try
   {
      // extract final solution from algorithm class
      if (_modelSolved) { _algo.getValues(puckLoationSIndicator, PuckLoationSIndicator); }
   }
   catch (IloException&)
   {
      throw("[gam::querySolution] Solution extraction phase - puckLoationSIndicator.");
   }

   try
   {
      // extract final solution from algorithm class
      if (_modelSolved) { _algo.getValues(puckLoationTIndicator, PuckLoationTIndicator); }
   }
   catch (IloException&)
   {
      throw("[gam::querySolution] Solution extraction phase - puckLoationTIndicator.");
   }

   //calculate pax connection time

   int obj = 0;
   int numPaxConnection = 0;
   for (auto ticket : tickets)
   {
      if (ticketsPuckIdx.find(ticket->getpaxID()) != ticketsPuckIdx.end())
      {
         int arrPuckIdx = ticketsPuckIdx[ticket->getpaxID()].first;
         int depPuckIdx = ticketsPuckIdx[ticket->getpaxID()].second;

         //get domestic/internation information for arrvial/departure

         std::string arrType = pucks[arrPuckIdx]->getarrType();
         std::string depType = pucks[depPuckIdx]->getdepType();

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) && (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 45;
            }
            else
            {
               turnTime = 20;
            }

            obj += puckLoationSIndicator[idxArr] * puckLoationSIndicator[idxDep] * turnTime * ticket->getPaxNum();
            numPaxConnection += puckLoationSIndicator[idxArr] * puckLoationSIndicator[idxDep] * ticket->getPaxNum();
         }

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) && (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 40;
            }
            else
            {
               turnTime = 30;
            }

            obj += puckLoationSIndicator[idxArr] * puckLoationTIndicator[idxDep] * turnTime * ticket->getPaxNum();
            numPaxConnection += puckLoationSIndicator[idxArr] * puckLoationTIndicator[idxDep] * ticket->getPaxNum();
         }

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) && (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 35;
            }
            else
            {
               turnTime = 20;
            }

            obj += puckLoationTIndicator[idxArr] * puckLoationTIndicator[idxDep] * turnTime * ticket->getPaxNum();
            numPaxConnection += puckLoationTIndicator[idxArr] * puckLoationTIndicator[idxDep] * ticket->getPaxNum();
         }

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) && (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" && depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" && depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" && depType == "D")
            {
               turnTime = 40;
            }
            else
            {
               turnTime = 30;
            }

            obj += puckLoationTIndicator[idxArr] * puckLoationSIndicator[idxDep] * turnTime * ticket->getPaxNum();
            numPaxConnection += puckLoationTIndicator[idxArr] * puckLoationSIndicator[idxDep] * ticket->getPaxNum();
         }
      }

   }

   std::cout << "pax connection time is: " << obj << std::endl;
   std::cout << "total num pax connection: " << numPaxConnection << std::endl;

}

void gam::updateSolutionGatePuckAssigned(IloNumArray puckAssign)
{
   solutionGateAssginedPuck.clear();
   int nPucks = pucks.size();

   int i = 0;
   for (auto & ivalidAsssign : validGateAsgnIndex)
   {
      if (puckAssign[i] >= 1)
      {
         int iGateIdx = ivalidAsssign.first / nPucks;
         int iPuckIdx = ivalidAsssign.first % nPucks;

         solutionGateAssginedPuck[iGateIdx].insert(iPuckIdx);

      }
      ++i;
   }
}


void gam::runTabuSearchModel()
{
   int numIterations = ParameterRegistry::instance()->totalNumTabuIter;
   tabuOptimization(numIterations);

}


double gam::evaluateObjective()
{
   double bestost = 0;
   //int numPuckViolations = 0;
   double flightTransitTime;

   for (auto const & ithticketsPuckIdx : ticketsPuckIdx)
   {
      int arrPuckIdx = ithticketsPuckIdx.second.first;
      int depPuckIdx = ithticketsPuckIdx.second.second;

      auto arrPuck = pucks[arrPuckIdx];
      auto depPuck = pucks[depPuckIdx];

      int iArrGateIdx = -1;
      int iDepGateIdx = -1;

      for (auto const& ithGateAssignedPuck : solutionGateAssginedPuck)
      {
         if (iArrGateIdx == -1)
         {
            auto arrPuckIdxAssigned = ithGateAssignedPuck.second.find(arrPuckIdx);
            if (arrPuckIdxAssigned != ithGateAssignedPuck.second.end())
            {
               iArrGateIdx = ithGateAssignedPuck.first;
            }
         }

         if (iDepGateIdx == -1)
         {
            auto depPuckIdxAssigned = ithGateAssignedPuck.second.find(depPuckIdx);
            if (depPuckIdxAssigned != ithGateAssignedPuck.second.end())
            {
               iDepGateIdx = ithGateAssignedPuck.first;
            }
         }

         if ((iArrGateIdx != -1) && (iDepGateIdx != -1))
         {
            break;
         }
      }
      if ((iArrGateIdx != -1) && (iDepGateIdx != -1))
      {
         //flightTransitTime = depPuck->getdepMinute() - arrPuck->getarrMinute();
         //if (flightTransitTime < 45)
         //{
         //   numPuckViolations++;
         //}
         std::string arrType = arrPuck->getarrType();
         std::string depType = depPuck->getdepType();

         int caseStartIdx = 0;
         if (arrType == "D" && depType == "D")
         {
            caseStartIdx = 0;
         }
         else if (arrType == "D" && depType == "I")
         {
            caseStartIdx = 4;
         }
         else if (arrType == "I" && depType == "D")
         {
            caseStartIdx = 8;
         }
         else if (arrType == "I" && depType == "I")
         {
            caseStartIdx = 12;
         }

         int arrRegion, depRegion;

         //TODO:use a function to extract region
         arrRegion = getRegionIdx(iArrGateIdx);
         depRegion = getRegionIdx(iDepGateIdx);

         int walkTime;
         int checkTrainTime;

         int addIdx;
         if (arrRegion < 2 && depRegion < 2)
         {
            addIdx = 0;
         }
         else if (arrRegion < 2 && depRegion >= 2)
         {
            addIdx = 1;
         }
         else if (arrRegion >= 2 && depRegion < 2)
         {
            addIdx = 2;
         }
         else
         {
            addIdx = 3;
         }
         flightTransitTime = depPuck->getdepMinute() - arrPuck->getarrMinute();
         walkTime = PaxWalkTime[arrRegion][depRegion];
         checkTrainTime = flowTransitTime.find(addIdx + caseStartIdx)->second;
         int tickIdx = ticketIndex[ithticketsPuckIdx.first];
         bestost += tickets[tickIdx]->getPaxNum() * (walkTime + checkTrainTime) / flightTransitTime;
      }
      
   }
   //bestost += numPuckViolations * 10000;//penalty for those conflict flight puck assignment - default 0 conflict
   return bestost;
}


//double gam::getCostChange(const int ithKeyIdx, const int jthKeyIdx, IloNumArray puckRemoteVal)
//{
//   double costChange = 0;
//
//   int oldPuckIdx = validAsgnIndex.find(ithKeyIdx)->second() % pucks.size();
//   int newPuckIdx = validAsgnIndex.find(jthKeyIdx)->second() % pucks.size();
//   int gateIdx = validAsgnIndex.find(ithKeyIdx)->second() / pucks.size();
//
//   double bestost = 0;
//   double flightTransitTime;
//
//   for (auto const& ithticketsPuckIdx : ticketsPuckIdx)
//   {
//      int arrPuckIdx = ithticketsPuckIdx.second.first;
//      int depPuckIdx = ithticketsPuckIdx.second.second;
//
//      auto arrPuck = pucks[arrPuckIdx];
//      auto depPuck = pucks[depPuckIdx];
//
//      if (puckRemoteVal[arrPuckIdx] == 0 && puckRemoteVal[depPuckIdx] == 0)
//      {
//         int iArrGateIdx = -1;
//         int iDepGateIdx = -1;
//
//         //TODO: improve performance
//         for (auto const& ithGateAssignedPuck : solutionGateAssginedPuck)
//         {
//            if (iArrGateIdx == -1)
//            {
//               auto arrPuckIdxAssigned = ithGateAssignedPuck.second.find(arrPuckIdx);
//               if (arrPuckIdxAssigned != ithGateAssignedPuck.second.end())
//               {
//                  iArrGateIdx = ithGateAssignedPuck.first;
//               }
//            }
//
//            if (iDepGateIdx == -1)
//            {
//               auto depPuckIdxAssigned = ithGateAssignedPuck.second.find(depPuckIdx);
//               if (depPuckIdxAssigned != ithGateAssignedPuck.second.end())
//               {
//                  iDepGateIdx = ithGateAssignedPuck.first;
//               }
//            }
//
//            if ((iArrGateIdx != -1) && (iDepGateIdx != -1))
//            {
//               break;
//            }
//         }
//         if ((iArrGateIdx != -1) && (iDepGateIdx != -1))
//         {
//            std::string arrType = arrPuck->getarrType();
//            std::string depType = depPuck->getdepType();
//
//            int caseStartIdx = 0;
//            if (arrType == "D" && depType == "D")
//            {
//               caseStartIdx = 0;
//            }
//            else if (arrType == "D" && depType == "I")
//            {
//               caseStartIdx = 4;
//            }
//            else if (arrType == "I" && depType == "D")
//            {
//               caseStartIdx = 8;
//            }
//            else if (arrType == "I" && depType == "I")
//            {
//               caseStartIdx = 12;
//            }
//
//            int arrRegion, depRegion;
//
//            //TODO:use a function to extract region
//            arrRegion = getRegionIdx(iArrGateIdx);
//            depRegion = getRegionIdx(iDepGateIdx);
//
//            int walkTime;
//            int checkTrainTime;
//
//            int addIdx;
//            if (arrRegion < 2 && depRegion < 2)
//            {
//               addIdx = 0;
//            }
//            else if (arrRegion < 2 && depRegion >= 2)
//            {
//               addIdx = 1;
//            }
//            else if (arrRegion >= 2 && depRegion < 2)
//            {
//               addIdx = 2;
//            }
//            else
//            {
//               addIdx = 3;
//            }
//
//            walkTime = PaxWalkTime[arrRegion][depRegion];
//            checkTrainTime = flowTransitTime.find(addIdx + caseStartIdx)->second;
//            int tickIdx = ticketIndex[ithticketsPuckIdx.first];
//            bestost += tickets[tickIdx]->getPaxNum() * (walkTime + checkTrainTime) / flightTransitTime;
//         }
//      }
//   }
//   return costChange;
//}


void gam::tabuOptimization(const int numIterations)
{
   IloNumArray puckRemote(_env);
   IloNumArray puckAssign(_env);

   _algo.getValues(PuckRemote, puckRemote);
   _algo.getValues(PuckAssign, puckAssign);

   int nPucks = pucks.size();
   std::vector<int> puckAssignTabuList(validGateAsgnIndex.size(), 0);
   double bestCost = evaluateObjective();
   double cost = bestCost;

   int i = 0;
   while (i < numIterations)
   {
      for ( int j = 0; j < validGateAsgnIndex.size(); ++j )
      {
         if (puckAssignTabuList[j] > 0)
         {
            puckAssignTabuList[j] -= 1;
         }
      }

      int exchangeIthIdxOld = -1;
      int exchangeJthIdxOld = -1;
      int exchangeIthIdxNew = -1;
      int exchangeJthIdxNew = -1;
      //bool isOuterrExchange = false;
      bool isInnerExchange = false;
      double minCostChange = 1000000000;

      std::cout << "tabu iteration: " << i << " cost: " << cost << std::endl;

      int iRand = rand() % 10;

      //outer swap: swap assigned puck with unassigned puck

      for (int ithPuck=0; ithPuck < nPucks;++ithPuck)
      {
         if (puckRemote[ithPuck] > 0)
         {
            //clock_t startTimeOuter = clock();
            auto validAssignedGates = puckGateMap[ithPuck];
            for (auto & ithValidGate : validAssignedGates)
            {
               auto ithGateAssignedPuck = solutionGateAssginedPuck[ithValidGate];

               for (auto & ithPuckAssigned : ithGateAssignedPuck)
               {
                  ////only swap with legal turn time
                  bool swappable = true;
                  for (auto& ithPuckAssignedCompare : ithGateAssignedPuck)
                  {
                     if (ithPuckAssigned != ithPuckAssignedCompare)
                     {
                        if (((pucks[ithPuckAssignedCompare]->getarrMinute() > pucks[ithPuck]->getarrMinute() - 45) 
                           && (pucks[ithPuckAssignedCompare]->getarrMinute() < pucks[ithPuck]->getdepMinute() + 45)) ||
                           ((pucks[ithPuckAssignedCompare]->getdepMinute() > pucks[ithPuck]->getarrMinute() - 45) 
                              && (pucks[ithPuckAssignedCompare]->getdepMinute() < pucks[ithPuck]->getdepMinute() + 45)) ||
                           (pucks[ithPuckAssignedCompare]->getarrMinute() <= pucks[ithPuck]->getarrMinute() - 45) 
                           && (pucks[ithPuckAssignedCompare]->getdepMinute() >= pucks[ithPuck]->getdepMinute() + 45))
                        {
                           swappable = false;
                           continue;
                        }
                     }
                  }

                  if (!swappable)
                  {
                     continue;
                  }

                  int ithKeyIdx = ithValidGate * nPucks + ithPuck;
                  int jthKeyIdx = ithValidGate * nPucks + ithPuckAssigned;

                  int ithIdxNew = validGateAsgnIndex.find(ithKeyIdx)->second;
                  int jthIdxNew = validGateAsgnIndex.find(jthKeyIdx)->second;


                  puckAssign[jthIdxNew] = 0;
                  puckAssign[ithIdxNew] = 1;

                  solutionGateAssginedPuck[ithValidGate].insert(ithPuck);
                  solutionGateAssginedPuck[ithValidGate].erase(ithPuckAssigned);

                  //double costChange = getCostChange(ithKeyIdx, jthKeyIdx);
                  double costChange = evaluateObjective();

                  //clock_t endTime = clock();
                  //std::cout << "evaluate: " << ithPuck << "," << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;


                  if (costChange - cost < minCostChange)
                  {
                     if ((puckAssignTabuList[ithIdxNew] > 0) && (costChange > bestCost))
                     {
                        continue;
                     }

                     exchangeIthIdxNew = ithIdxNew;
                     exchangeJthIdxNew = jthIdxNew;
                     //isOuterrExchange = true;
                     minCostChange = costChange - cost;
                  }

                  //set them to orignal value
                  puckAssign[jthIdxNew] = 1;
                  puckAssign[ithIdxNew] = 0;

                  solutionGateAssginedPuck[ithValidGate].insert(ithPuckAssigned);
                  solutionGateAssginedPuck[ithValidGate].erase(ithPuck);

               }

            }

            //clock_t endTimeOuter = clock();
            //std::cout << "outer-------------puck: " << ithPuck << "," << (double)(endTimeOuter - startTimeOuter) / CLOCKS_PER_SEC << "s" << std::endl;

         }
      }

      //inner swap: swap within assigned pucks
      for (int ithPuck = 0; ithPuck < nPucks; ++ithPuck)
      {
         if (puckRemote[ithPuck] <= 0)
         {
            //clock_t startTimeInner, endTimeInner;
            //startTimeInner = clock();
            int ithIdx = 0;
            auto validAssignedGates = puckGateMap[ithPuck];
            int tempGate = 0;
            for (auto& ithValidGate : validAssignedGates)
            {
               int tempIdx = validGateAsgnIndex.find(ithValidGate * nPucks + ithPuck)->second;
               //get the idx of the current puck assigned gate
               if (puckAssign[tempIdx] > 0)
               {
                  ithIdx = tempIdx;
                  tempGate = ithValidGate;
                  break;
               }
               
            }
            for (auto & jthValidGate : validAssignedGates)
            {
               //puck should be swapped to another gate
               if (tempGate == jthValidGate)
               {
                  continue;
               }

               auto jthGateAssignedPuck = solutionGateAssginedPuck[jthValidGate];
               auto ithGateAssignedPuck = solutionGateAssginedPuck[tempGate];


               for (auto & jthPuck : jthGateAssignedPuck)
               {
                  //only swap i, j to validate i, j gate
                  if (puckGateMap[jthPuck].find(tempGate) == puckGateMap[jthPuck].end())
                  {
                     continue;
                  }
                  ////only swap with legal turn time
                  bool swappable = true;
                  for (auto& jthPuckAssignedCompare : jthGateAssignedPuck)
                  { // validate on jth gate: ithpuck vs all pucks on ith gate except the one swapped out 
                     if (jthPuck != jthPuckAssignedCompare)
                     {
                        if (((pucks[jthPuckAssignedCompare]->getarrMinute() > pucks[ithPuck]->getarrMinute() - 45)
                           && (pucks[jthPuckAssignedCompare]->getarrMinute() < pucks[ithPuck]->getdepMinute() + 45)) ||
                           ((pucks[jthPuckAssignedCompare]->getdepMinute() > pucks[ithPuck]->getarrMinute() - 45)
                              && (pucks[jthPuckAssignedCompare]->getdepMinute() < pucks[ithPuck]->getdepMinute() + 45)) ||
                           (pucks[jthPuckAssignedCompare]->getarrMinute() <= pucks[ithPuck]->getarrMinute() - 45)
                           && (pucks[jthPuckAssignedCompare]->getdepMinute() >= pucks[ithPuck]->getdepMinute() + 45))
                        {
                           swappable = false;
                           break;
                        }
                     }
                  }
                  if (!swappable)
                  {
                     continue;
                  }
                  //validate on ith gate: jth puck with all the rest on ith gate except ithpuck
                  for (auto& ithPuckAssignedCompare : ithGateAssignedPuck)
                  {  if (ithPuck != ithPuckAssignedCompare)
                     {
                        if (((pucks[ithPuckAssignedCompare]->getarrMinute() > pucks[jthPuck]->getarrMinute() - 45)
                           && (pucks[ithPuckAssignedCompare]->getarrMinute() < pucks[jthPuck]->getdepMinute() + 45)) ||
                           ((pucks[ithPuckAssignedCompare]->getdepMinute() > pucks[jthPuck]->getarrMinute() - 45)
                              && (pucks[ithPuckAssignedCompare]->getdepMinute() < pucks[jthPuck]->getdepMinute() + 45)) ||
                           (pucks[ithPuckAssignedCompare]->getarrMinute() <= pucks[jthPuck]->getarrMinute() - 45)
                           && (pucks[ithPuckAssignedCompare]->getdepMinute() >= pucks[jthPuck]->getdepMinute() + 45))
                        {
                           swappable = false;
                           break;
                        }
                     }
                  }

                  if (!swappable)
                  {
                     continue;
                  }

                  int jthIdx = validGateAsgnIndex.find(jthValidGate * nPucks + jthPuck)->second;

                  puckAssign[ithIdx] = 0;
                  puckAssign[jthIdx] = 0;
                  int ithIdxNew = validGateAsgnIndex.find(tempGate * nPucks + jthPuck)->second;
                  int jthIdxNew = validGateAsgnIndex.find(jthValidGate * nPucks + ithPuck)->second;
                  puckAssign[ithIdxNew] = 1;
                  puckAssign[jthIdxNew] = 1;


                  solutionGateAssginedPuck[jthValidGate].insert(ithPuck);
                  solutionGateAssginedPuck[jthValidGate].erase(jthPuck);

                  solutionGateAssginedPuck[tempGate].insert(jthPuck);
                  solutionGateAssginedPuck[tempGate].erase(ithPuck);


                  //updateSolutionGatePuckAssigned(puckAssign);

                  double costChange = evaluateObjective();
                  //double costChange = getCostChange(ithKeyIdx, jthKeyIdx);


                  if (costChange - cost < minCostChange)
                  {
                     if ((puckAssignTabuList[jthIdxNew] > 0 || puckAssignTabuList[ithIdxNew] > 0) && (costChange > bestCost))
                     {
                        continue;
                     }
                     exchangeIthIdxOld = ithIdx;
                     exchangeJthIdxOld = jthIdx;
                     exchangeIthIdxNew = ithIdxNew;
                     exchangeJthIdxNew = jthIdxNew;
                     isInnerExchange = true;
                     minCostChange = costChange - cost;
                  }

                  //set them to orignal value
                  puckAssign[ithIdx] = 1;
                  puckAssign[jthIdx] = 1;
                  puckAssign[ithIdxNew] = 0;
                  puckAssign[jthIdxNew] = 0;
                  //updateSolutionGatePuckAssigned(puckAssign);

                  solutionGateAssginedPuck[jthValidGate].insert(jthPuck);
                  solutionGateAssginedPuck[jthValidGate].erase(ithPuck);

                  solutionGateAssginedPuck[tempGate].insert(ithPuck);
                  solutionGateAssginedPuck[tempGate].erase(jthPuck);
                  
               }
            }
            //endTimeInner = clock();
            //std::cout << "inner-----------puck: " << ithPuck << ","<< (double)(endTimeInner - startTimeInner) / CLOCKS_PER_SEC << "s" << std::endl;
         }
      }

      if (isInnerExchange)
      {
         //update solution for evaluation
         puckAssign[exchangeIthIdxNew] = 1;
         puckAssign[exchangeJthIdxNew] = 1;
         puckAssign[exchangeIthIdxOld] = 0;
         puckAssign[exchangeJthIdxOld] = 0;


         solutionGateAssginedPuck[(validAsgnIndex.find(exchangeIthIdxNew)->second) / nPucks].insert((validAsgnIndex.find(exchangeIthIdxNew)->second) % nPucks);
         solutionGateAssginedPuck[(validAsgnIndex.find(exchangeIthIdxOld)->second) / nPucks].erase((validAsgnIndex.find(exchangeIthIdxOld)->second) % nPucks);

         solutionGateAssginedPuck[(validAsgnIndex.find(exchangeJthIdxNew)->second) / nPucks].insert((validAsgnIndex.find(exchangeJthIdxNew)->second) % nPucks);
         solutionGateAssginedPuck[(validAsgnIndex.find(exchangeJthIdxOld)->second) / nPucks].erase((validAsgnIndex.find(exchangeJthIdxOld)->second) % nPucks);
         //std::cout << "######################################################" << std::endl;
         //std::cout << "exchangeIthIdxNew: " << exchangeIthIdxNew << " insert: " << " validAsgnIndex.find(exchangeIthIdxNew)->second" << validAsgnIndex.find(exchangeIthIdxNew)->second << std::endl;

         //std::cout << "gate: " << (validAsgnIndex.find(exchangeIthIdxNew)->second) / nPucks << " insert: " << validAsgnIndex.find(exchangeIthIdxNew)->second % nPucks << std::endl;
         //std::cout << "gate: " << (validAsgnIndex.find(exchangeIthIdxOld)->second) / nPucks << " erase: " << validAsgnIndex.find(exchangeIthIdxOld)->second % nPucks << std::endl;

         //std::cout << "gate: " << (validAsgnIndex.find(exchangeJthIdxNew)->second) / nPucks << " insert: " << validAsgnIndex.find(exchangeJthIdxNew)->second % nPucks << std::endl;
         //std::cout << "gate: " << (validAsgnIndex.find(exchangeJthIdxOld)->second) / nPucks << " erase: " << validAsgnIndex.find(exchangeJthIdxOld)->second % nPucks << std::endl;



      }
      else
      {  
         //update solution for evaluation
         puckRemote[(validAsgnIndex.find(exchangeIthIdxNew)->second) % nPucks] = 0;
         puckRemote[(validAsgnIndex.find(exchangeJthIdxNew)->second) % nPucks] = 1;
         puckAssign[exchangeJthIdxNew] = 0;
         puckAssign[exchangeIthIdxNew] = 1;

         solutionGateAssginedPuck[(validAsgnIndex.find(exchangeIthIdxNew)->second) / nPucks].insert((validAsgnIndex.find(exchangeIthIdxNew)->second) % nPucks);
         solutionGateAssginedPuck[(validAsgnIndex.find(exchangeJthIdxNew)->second) / nPucks].erase((validAsgnIndex.find(exchangeJthIdxNew)->second) % nPucks);
      }

      std::cout << "min cost change:  " << minCostChange << std::endl;
      if( minCostChange < 1000000000)
      {
         if (!isInnerExchange)
         {
            puckAssignTabuList[exchangeIthIdxNew] = iRand + 10;
         }
         else
         {
            puckAssignTabuList[exchangeIthIdxNew] = iRand + 10;
            puckAssignTabuList[exchangeJthIdxNew] = iRand + 10;

         }

         //puckAssignTabuList[exchangeJthIdxNew] = 5;
         //update optimal value
         if (cost + minCostChange < bestCost)
         {
            bestCost = cost + minCostChange;
            cost = bestCost;

            //updateSolutionGatePuckAssigned(puckAssign);
         }
      }


      ++i;
   }

   puckAssign.end();
   puckRemote.end();
}


int gam::getRegionIdx(const int gateIdx)
{
   int tempRegion;
   if (gates[gateIdx]->getterminal() == "T" && gates[gateIdx]->getregion() == "North")
   {
      tempRegion = 0;//T_North-T_North
   }
   else if (gates[gateIdx]->getterminal() == "T" && gates[gateIdx]->getregion() == "Center")
   {
      tempRegion = 1;//
   }
   else if (gates[gateIdx]->getterminal() == "T" && gates[gateIdx]->getregion() == "South")
   {
      tempRegion = 2;//
   }
   else if (gates[gateIdx]->getterminal() == "S" && gates[gateIdx]->getregion() == "North")
   {
      tempRegion = 3;//
   }
   else if (gates[gateIdx]->getterminal() == "S" && gates[gateIdx]->getregion() == "Center")
   {
      tempRegion = 4;//
   }
   else if (gates[gateIdx]->getterminal() == "S" && gates[gateIdx]->getregion() == "South")
   {
      tempRegion = 5;//
   }
   else if (gates[gateIdx]->getterminal() == "S" && gates[gateIdx]->getregion() == "East")
   {
      tempRegion = 6;//
   }

   return tempRegion;
}
