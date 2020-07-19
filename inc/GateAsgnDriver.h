#pragma once
#include <memory>

#include "DataManager.h"
#include "gam.h"




class GateAsgnDriver
{
   IloEnv _env;
public:


   GateAsgnDriver();
   ~GateAsgnDriver();
   void initValidVariableAsgnIndex();
   void optimize();
   void solveModel();
   void initializeGAM();
   void solveGAM();
   void extractSolution();


private:
   //DataManager  _dataManager;
   std::unique_ptr<gam> _gam;

};
