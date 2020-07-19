#include <set>
#include <vector>
#include <algorithm>

#include "../inc/gam.h"
#include "../inc/DataManager.h"


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



void gam::initMaps()
{

   // Vector mapping for puck, gate within gam module

   // sort puck based on puckID
   std::stable_sort(pucks.begin(), pucks.end(), Puck::sortAlphabeticPtr());
   // aircraft information
   const size_t nPuck = pucks.size();
   for (size_t j = 0; j < nPuck; j++)
   {
      puckIndex[pucks[j]->getpuckId()] = j;
      puckMap[pucks[j]->getpuckId()] = pucks[j];
   }

   // gate information
   std::stable_sort(gates.begin(), gates.end(), Gate::sortAlphabeticPtr());
   size_t nGates = gates.size();
   for (size_t j = 0; j < nGates; j++)
   {
      gateIndex[gates[j]->getgateID()] = j;
      gateMap[gates[j]->getgateID()] = gates[j];
   }

   // ticket information
   std::stable_sort(tickets.begin(), tickets.end(), Ticket::sortAlphabeticPtr());
   size_t nTickets = tickets.size();
   for (size_t j = 0; j < nTickets; j++)
   {
      ticketIndex[tickets[j]->getpaxID()] = j;
      ticketMap[tickets[j]->getpaxID()] = tickets[j];
   }

}



void gam::initValidVariableAsgnIndex()
{
   size_t itr1 = 0;
   const size_t nPucks = pucks.size();
   size_t k = 0;
   for (auto gate : gates)
   {
      for (auto puck : pucks)
      {
         // recorder valid assign index for each puck-gate
         if ((gate->getarrType().find(puck->getarrType()) != std::string::npos) & (gate->getdepType().find(puck->getdepType()) != std::string::npos)
            & (puck->getpuckBodyType() == gate->getbodyType()))
         {
            size_t itr2 = puckIndex[puck->getpuckId()];      // determine gateIndex
            size_t j = itr1 * nPucks + itr2;              // determine corresponding index
            validGateAsgnIndex[j] = k;
            k++;
         }
      }
      itr1++;
   }
}



void gam::initValidPuckTermIndex()
{
   size_t nPucks = pucks.size();


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
      if (gates[iGateIdx]->getterminal() == "T" & gates[iGateIdx]->getregion() == "North")
      {
         regionNum = 0;//T_North-T_North
      }
      else if (gates[iGateIdx]->getterminal() == "T" & gates[iGateIdx]->getregion() == "Center")
      {
         regionNum = 1;//
      }
      else if (gates[iGateIdx]->getterminal() == "T" & gates[iGateIdx]->getregion() == "South")
      {
         regionNum = 2;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" & gates[iGateIdx]->getregion() == "North")
      {
         regionNum = 3;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" & gates[iGateIdx]->getregion() == "Center")
      {
         regionNum = 4;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" & gates[iGateIdx]->getregion() == "South")
      {
         regionNum = 5;//
      }
      else if (gates[iGateIdx]->getterminal() == "S" & gates[iGateIdx]->getregion() == "East")
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
   const size_t nPuckGates = validGateAsgnIndex.size();
   const size_t nPucks = pucks.size();//just a large number
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
      PuckLoationSIndicator = IloIntVarArray(_env, nPuckTermS, 0, 1);
      PuckLoationTIndicator = IloIntVarArray(_env, nPuckTernT, 0, 1);
      PuckRegionIndicator = IloIntVarArray(_env, nPuckRegions, 0, 1);

      TicketRegionPairIndicator = IloIntVarArray(_env, nTicketRegions, 0, 1);

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
      if (solveModel2)
      {
         addPuckLocationTerm(objExpr);
      }

      if(solveModel3)
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

      if (solveModel2)
      {
         formualtePuckLocationConstr();
      }

      if (solveModel3)
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
   const size_t nPucks = pucks.size();

   for (const auto & iValidGateAssign : validGateAsgnIndex)
   {
      int iGateIdx = iValidGateAssign.first / nPucks;
      int iPuckIdx = iValidGateAssign.first % nPucks;

      int j = iValidGateAssign.second;

      objExpr += normalGateAssignBonus * PuckAssign[j];

      //set variable name
      std::string varName = "PuckAssign_" + gates[iGateIdx]->getgateID() + "_" + std::to_string(pucks[iPuckIdx]->getarrMinute()) + 
         "_" + std::to_string(pucks[iPuckIdx]->getdepMinute()) + "_" + pucks[iPuckIdx]->getpuckId();
      PuckAssign[j].setName(varName.c_str());
   }
}


void gam::addPuckLocationTerm(IloExpr& objExpr)
{
   size_t itr1 = 0;
   for (auto puckTerm: puckIDtoS)
   {
      //set variable name
      //auto idx = std::distance(puckIDtoS.begin(), puckIDtoS.find(puckTerm));
      std::string varName = pucks[puckTerm]->getpuckId() + "Term_S";
      PuckLoationSIndicator[itr1].setName(varName.c_str());
      ++itr1;
   }

   size_t itr2 = 0;
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

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) & (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" & depType == "D")
            {
               turnTime = 45;
            }
            else
            {
               turnTime = 20;
            }

            objExpr += PuckLoationSIndicator[idxArr] * PuckLoationSIndicator[idxDep] * turnTime * ticket->getPaxNum() * paxConnectPenaltyPerMinute;
         }

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) & (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" & depType == "D")
            {
               turnTime = 40;
            }
            else
            {
               turnTime = 30;
            }

            objExpr += PuckLoationSIndicator[idxArr] * PuckLoationTIndicator[idxDep] * turnTime * ticket->getPaxNum()* paxConnectPenaltyPerMinute;
         }

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) & (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" & depType == "D")
            {
               turnTime = 35;
            }
            else
            {
               turnTime = 20;
            }

            objExpr += PuckLoationTIndicator[idxArr] * PuckLoationTIndicator[idxDep] * turnTime * ticket->getPaxNum() * paxConnectPenaltyPerMinute;
         }

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) & (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" & depType == "D")
            {
               turnTime = 40;
            }
            else
            {
               turnTime = 30;
            }

            objExpr += PuckLoationTIndicator[idxArr] * PuckLoationSIndicator[idxDep] * turnTime * ticket->getPaxNum() * paxConnectPenaltyPerMinute;
         }
      }
   }

   std::cout << "total number connection Pax: " << numConnectPax << std::endl;
}


void gam::addPuckRemote(IloExpr& objExpr)
{
   size_t j = 0;
   for (const auto puck : pucks)
   {


      objExpr += remoteGateAssignBonus * PuckRemote[j];

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
   int numRegions = 7;
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
      if (arrType == "D" & depType == "D")
      {
         caseStartIdx = 0;
      }
      else if (arrType == "D" & depType == "I")
      {
         caseStartIdx = 4;
      }
      else if (arrType == "I" & depType == "D")
      {
         caseStartIdx = 8;
      }
      else if (arrType == "I" & depType == "I")
      {
         caseStartIdx = 12;
      }
      
      for (auto arrRegion : puckRegion[arrPuckIdx])
      {
         int walkTime, tempFlowTrainTime = 0;

         for (auto depRegion : puckRegion[depPuckIdx])
         {
            int addIdx;
            if (arrRegion < 2 & depRegion < 2)
            {
               addIdx = 0;
            }
            else if (arrRegion < 2 & depRegion >= 2)
            {
               addIdx = 1;
            }
            else if (arrRegion >= 2 & depRegion < 2)
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
   size_t nPucks = pucks.size();
   size_t i = 0;

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

      if (_algo.solve())
      {
         _modelSolved = true;
         gam_obj = _algo.getObjValue();
      }
      else
      {
         _modelSolved = false;
         gam_obj = -1;

      }
   }
   catch (IloException&)
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

   size_t itr1 = 0;

   for (const auto gate : gates)
   {
      objExpr += fixedGatePenalty * GatesUsed[itr1];

      //set variable name
      std::string varName = "Gate_" + gate->getgateID();
      GatesUsed[itr1].setName(varName.c_str());
      GatesUsed[itr1].setLB(0);
      GatesUsed[itr1].setUB(1);

      itr1++;
   }

   //add extra remote gate used penalty
   objExpr += remoteFixedGatePenalty*GatesUsed[itr1];
   //set variable name
   std::string varName = "Gate_Remote";
   GatesUsed[itr1].setName(varName.c_str());
   GatesUsed[itr1].setLB(0);
   GatesUsed[itr1].setUB(1);

}



void gam::formualtePuckLocationConstr()
{
   size_t nPucks = pucks.size();
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

   size_t nPucks = pucks.size();

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
         if (((pucks[jPuckIdx]->getarrMinute() > pucks[iPuckIdx]->getarrMinute() - acConnectTime) & (pucks[jPuckIdx]->getarrMinute() < pucks[iPuckIdx]->getdepMinute() + acConnectTime)) |
            ((pucks[jPuckIdx]->getdepMinute() > pucks[iPuckIdx]->getarrMinute() - acConnectTime) & (pucks[jPuckIdx]->getdepMinute() < pucks[iPuckIdx]->getdepMinute() + acConnectTime)) |
            (pucks[jPuckIdx]->getarrMinute() <= pucks[iPuckIdx]->getarrMinute() - acConnectTime) & (pucks[jPuckIdx]->getdepMinute() >= pucks[iPuckIdx]->getdepMinute() + acConnectTime))

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

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) & (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" & depType == "D")
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

         if ((puckIDtoS.find(arrPuckIdx) != puckIDtoS.end()) & (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoS.begin(), puckIDtoS.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" & depType == "D")
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

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) & (puckIDtoT.find(depPuckIdx) != puckIDtoT.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoT.begin(), puckIDtoT.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 15;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 35;
            }
            else if (arrType == "I" & depType == "D")
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

         if ((puckIDtoT.find(arrPuckIdx) != puckIDtoT.end()) & (puckIDtoS.find(depPuckIdx) != puckIDtoS.end()))
         {
            auto idxArr = std::distance(puckIDtoT.begin(), puckIDtoT.find(arrPuckIdx));
            auto idxDep = std::distance(puckIDtoS.begin(), puckIDtoS.find(depPuckIdx));

            int turnTime = 0;
            if (arrType == "D" & depType == "D")
            {
               turnTime = 20;
            }
            else if (arrType == "D" & depType == "I")
            {
               turnTime = 40;
            }
            else if (arrType == "I" & depType == "D")
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
