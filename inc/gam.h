#pragma once
#include <map>
#include <string>
#include <vector>

#include "DataManager.h"
#include "Gate.h"
#include "Puck.h"
#include "Ticket.h"

#include <set>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>


class gam
{
   // ILOG Concert parameters
   IloEnv _env;
   IloModel _model;
   IloObjective _obj;
   IloCplex _algo;
public:
   //gam();
   explicit gam(const IloEnv& env);
   ~gam();


   const std::vector<Gate *>& getGates() const
   {
      return gates;
   }
   void addGateData(const std::vector<Gate *> &givenGates)
   {
      gates = givenGates;
   }


   const std::vector<Puck *>& getPucks() const
   {
      return pucks;
   }
   void addPuckData(const std::vector<Puck *>& givenPucks)
   {
      pucks = givenPucks;
   }

   const std::vector<Ticket *>& getTickets() const
   {
      return tickets;
   }
   void addTicketData(const std::vector<Ticket *>& givenTickets)
   {
      tickets = givenTickets;
   }


   void addTicketPuckIdxData(std::map<const std::string, std::pair<int, int>> giventicketsPuckIdx)
   {
      ticketsPuckIdx = giventicketsPuckIdx;
   }

   void  setGAMName(const std::string & name) { gamName = name; }
   const std::string & getGAMName() const { return gamName; }
   void exportModel(const std::string & fileName);
   void exportSolution(const std::string & filename);


   void initMaps();
   void initValidVariableAsgnIndex();
   void initValidPuckTermIndex();

   void initModel();
   void initConstraints();
   void initObjective();
   void addPuckAssignTerm(IloExpr &objExpr);
   void addPuckRemote(IloExpr &objExpr);
   void addGateNumberTerm(IloExpr &objExpr);
   void addPuckLocationTerm(IloExpr &objExpr);

   void addPuckRegionTerm(IloExpr &objExpr);
   void addTicketRegionTerm(IloExpr &objExpr);



   void formulatePuckCoverConstr();
   void formulateGateUsedConstr();
   void formualteGroundConnctConstr();
   void formualtePuckLocationConstr();
   void formualtePuckRegionConstr();
   void formualteTicketRegionConstr();

   void solveInitModel();
   void querySolution();
   static const int PaxWalkTime[7][7];
   static const std::map<int, int> flowTransitTime;
  



private:
   // Vector mapping for puck, gate within gam module
   std::map<std::string, size_t>      gateIndex; // gate index
   std::map<std::string, Gate*>   gateMap;   // gate map

   std::map<std::string, size_t>      puckIndex; // puck index
   std::map<std::string, Puck*>   puckMap;   // puck map

   std::map<std::string, size_t>      ticketIndex; // puck index
   std::map<std::string, Ticket*>   ticketMap;   // puck map

   std::vector<Gate *> gates;
   std::vector<Puck *> pucks;
   //std::vector<Puck *> includedPucks;
   std::vector<Ticket *> tickets;
   std::map<const std::string, std::pair<int, int>> ticketsPuckIdx;

   std::map<size_t, size_t> validGateAsgnIndex;
   std::map<size_t, size_t>::iterator iValidGateAsgnIndex;

   std::set<int> puckIDtoS;
   std::set<int> puckIDtoT;

   std::map<size_t, int> puckIDtoRegion;
   std::map<int, std::set<int>> puckRegion;


   DataManager * datamanager;

   const double normalGateAssignBonus = -100; //coeeficient value for assigning pucks on normal gates
   const double remoteGateAssignBonus = 10; //coeeficient value for assigning pucks on remote gates
   const double fixedGatePenalty = 50; //coeeficient value for assigning pucks on remote gates
   const double remoteFixedGatePenalty = 10000; //coeeficient value for assigning pucks on remote gates
   const double paxConnectPenaltyPerMinute = 0.001; //

   const bool solveModel2 = true;
   const bool solveModel3 = false;


   //decison variable
   IloIntVarArray PuckAssign;
   IloIntVarArray PuckRemote;
   IloNumVarArray GatesUsed;
   IloIntVarArray PuckLoationSIndicator;
   IloIntVarArray PuckLoationTIndicator;
   IloIntVarArray PuckRegionIndicator;
   IloIntVarArray TicketRegionPairIndicator;




   //rhs constant value
   std::vector<int>     rhsPuckCover;      // puck coverage constraint
   std::vector<int>     rhsGateBinding;      // puck coverage constraint
   //std::vector<int>     rhsGateBinding;      // puck coverage constraint


   //Constraint expression
   IloRangeArray  PuckCover;
   IloRangeArray  GateBinding;
   IloRangeArray  PuckGroundConnectionTime;
   IloRangeArray  PuckLocationConstrs;
   IloRangeArray  PuckRegionConstrs;
   IloRangeArray  TicketRegionConstrs;



   std::string gamName;
   bool modelDefined;               // set to true of model has been assembled
   bool modelExtracted;           // set to true if model has been extracted
   bool _modelSolved;          // true if SrmModel solves successfully
                               //SRM objective
   double gam_obj;
};
