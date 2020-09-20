#pragma once
#include <memory>

#include "DataManager.h"
#include "gam.h"


class ParameterRegistry;

class GateAsgnDriver
{
   IloEnv _env;
public:


   GateAsgnDriver();
   ~GateAsgnDriver();

   void readConfigurationFile();

   void optimize();
   void solveModel();
   void initializeGAM();
   void solveGAM();
   void extractSolution();
   void exportSolution();


private:
   ParameterRegistry* paraRegistry;
   DataManager* dataMgr;
   std::unique_ptr<gam> _gam;

};
