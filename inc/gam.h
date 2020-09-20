#pragma once
#include <map>
#include <memory>
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


   const std::vector<std::shared_ptr<Gate>>& getGates() const
   {
      return gates;
   }
   void addGateData(const std::vector<std::shared_ptr<Gate>> &givenGates)
   {
      gates = givenGates;
   }


   const std::vector<std::shared_ptr<Puck>>& getPucks() const
   {
      return pucks;
   }
   void addPuckData(const std::vector<std::shared_ptr<Puck>>& givenPucks)
   {
      pucks = givenPucks;
   }

   const std::vector<std::shared_ptr<Ticket>>& getTickets() const
   {
      return tickets;
   }
   void addTicketData(const std::vector<std::shared_ptr<Ticket>>& givenTickets)
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
   int getRegionIdx(const int gateIdx);



   void formulatePuckCoverConstr();
   void formulateGateUsedConstr();
   void formualteGroundConnctConstr();
   void formualtePuckLocationConstr();
   void formualtePuckRegionConstr();
   void formualteTicketRegionConstr();

   void solveInitModel();
   void querySolution();
   void runTabuSearchModel();
   void updateSolutionGatePuckAssigned(IloNumArray puckAssign);
   double evaluateObjective();
   //double getCostChange(const int ithKeyIdx, const int jthKeyIdx, IloNumArray puckRemote);

   void tabuOptimization(const int numIterations);
   static const int PaxWalkTime[7][7];
   static const std::map<int, int> flowTransitTime;
  
   static std::stringstream values;
   static std::stringstream& getCoefficients() { return values; };
   static void clearCoefficients() { values.str(""); };


private:
   // Vector mapping for puck, gate within gam module
   std::map<std::string, int>      gateIndex; // gate index
   std::map<std::string, std::shared_ptr<Gate>>   gateMap;   // gate map

   std::map<std::string, int>      puckIndex; // puck index
   std::map<std::string, std::shared_ptr<Puck>>   puckMap;   // puck map

   std::map<std::string, int>      ticketIndex; // puck index
   std::map<std::string, std::shared_ptr<Ticket>>   ticketMap;   // puck map

   std::vector<std::shared_ptr<Gate>> gates;
   std::vector<std::shared_ptr<Puck>> pucks;
   //std::vector<std::shared_ptr<Puck>> includedPucks;
   std::vector<std::shared_ptr<Ticket>> tickets;
   std::map<const std::string, std::pair<int, int>> ticketsPuckIdx;

   std::map<int, int> validGateAsgnIndex; //<gates*pucks+jthpuck, ith>
   std::map<int, int> validAsgnIndex; //<ith, gates*pucks+jthpuck>

   std::map<int, std::set<int>> puckGateMap;
   std::map<int, int>::iterator iValidGateAsgnIndex;

   std::set<int> puckIDtoS;
   std::set<int> puckIDtoT;

   std::map<int, int> puckIDtoRegion;
   std::map<int, std::set<int>> puckRegion;


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

public:
   std::map<int, std::set<int>> solutionGateAssginedPuck;

};
