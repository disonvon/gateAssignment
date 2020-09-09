#pragma once
#include <string>

class ParameterRegistry
{
public:

   enum typeOfObjFunction
   {
      gateAssignment = 1,
      gateAssignmentWithPaxConn,
      gateAssignmentWithPaxConnFairness

   };

   static void clean()
   {
      if (_paraInstance)
      {
         delete _paraInstance;
         _paraInstance = 0;
      }
   }

   ParameterRegistry();
   ~ParameterRegistry();

   static ParameterRegistry* instance()
   {
      if (!_paraInstance)
      {
         _paraInstance = new ParameterRegistry();
      }
      return _paraInstance;
   }

   bool   dataAnalysis;                                // verbose flag for output
   std::string   directory;                                // working directory for flatfiles
   std::string  debugDirectory;                                     // directory for debug files
   typeOfObjFunction objFunction;

   double normalGateAssignBonus; //coeeficient value for assigning pucks on normal gates
   double remoteGateAssignBonus; //coeeficient value for assigning pucks on remote gates
   double fixedGatePenalty; //coeeficient value for assigning pucks on remote gates
   double remoteFixedGatePenalty; //coeeficient value for assigning pucks on remote gates
   double paxConnectPenaltyPerMinute; //


   int totalNumTabuIter;
   int timeLimitation; //will set it as configurable from external file

   void  setParameters_dataAnalysis(const bool bVal) { dataAnalysis = bVal; }
   void  setParameters_directory(const std::string& val) { directory = val; }
   void  setParameters_debugDirectory(const std::string& val) { debugDirectory = val; }


   void  setParameters_OBJECTIVE_FUNCTION(const int ival) { objFunction = static_cast<ParameterRegistry::typeOfObjFunction>(ival); }
   void  setParameters_normalGateAssignBonus(const double dval) { normalGateAssignBonus = dval; }
   void  setParameters_remoteGateAssignBonus(const double dval) { remoteGateAssignBonus = dval; }
   void  setParameters_fixedGatePenalty(const double dval) { fixedGatePenalty = dval; }
   void  setParameters_remoteFixedGatePenalty(const double dval) { remoteFixedGatePenalty = dval; }
   void  setParameters_paxConnectPenaltyPerMinute(const int ival) { paxConnectPenaltyPerMinute = ival/1000.0; }

   void  setParameters_totalNumTabuIter(const int ival) { totalNumTabuIter = ival; }
   void  setParameters_timeLimitation(const int ival) { timeLimitation = ival; }







private:
   static ParameterRegistry* _paraInstance;

};
