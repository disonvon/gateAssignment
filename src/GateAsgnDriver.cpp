#include <ctime>

#include "../inc/GateAsgnDriver.h"


//GateAsgnDriver::GateAsgnDriver()
//   :_gam(new gam())
//{
//   _env.setDeleter(IloLinearDeleterMode);
//}


GateAsgnDriver::GateAsgnDriver()
   :_gam(new gam(_env))
{
   //_env.setDeleter(IloLinearDeleterMode);
}


GateAsgnDriver::~GateAsgnDriver()
{
   _gam.reset();
   //_env.end(); // release Ilog concert environment

}


void GateAsgnDriver::initValidVariableAsgnIndex()
{//TODO:: get back later
   //std::vector<std::shared_ptr<Puck>> pucks = _dataManager.getIncludedPucks();
   //std::vector<std::shared_ptr<Gate>> gates = _dataManager.getGates();
   //for (const std::shared_ptr<Puck> const puck : pucks)
   //{
   //   for (const std::shared_ptr<Gate> const gate : gates)
   //   {
   //      if (puck->getarrType() == gate->getarrType())
   //      {
   //         std::cout << "Dison test!!!!!!!!!!!!!!!!!!!!1";
   //      }
   //   }
   //}
}


void GateAsgnDriver::optimize()
{
   auto dataMgr = DataManager::instance();

   //***************read data**********************************
   clock_t startTime, endTime;
   startTime = clock();
   dataMgr->readPuck();
   dataMgr->readGate();
   dataMgr->readTicket();
   endTime = clock();

   std::cout << "read data elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "S" << std::endl;
   //***************preprocess data**********************************
   startTime = clock();
   dataMgr->preprocess();
   endTime = clock();
   std::cout << "preprocess data elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "S" << std::endl;

   //***************build and solve model elapsed*********************
   startTime = clock();
   solveModel();
   endTime = clock();
   std::cout << "build and solve model elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "S" << std::endl;

   //extract solution



}

void GateAsgnDriver::initializeGAM()
{
   // *****************add data to gam******************
   try
   {
      _gam->addGateData(DataManager::instance()->_gates);//all gates
      _gam->addPuckData(DataManager::instance()->_includedPucks);
      _gam->addTicketData(DataManager::instance()->_includedTickets);//included tickets
      _gam->addTicketPuckIdxData(DataManager::instance()->ticketsPuckIdx);


      _gam->initMaps();
      _gam->initValidVariableAsgnIndex();
      _gam->initValidPuckTermIndex();

      _gam->initModel();
      _gam->initObjective();

      _gam->initConstraints();

   }
   catch (...)
   {
      throw("initialize gam failed");
   }
}


void GateAsgnDriver::solveModel()
{
   clock_t startTime, endTime;
   startTime = clock();
   initializeGAM();
   endTime = clock();
   std::cout << "model initialized elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "S" << std::endl;


   startTime = clock();
   solveGAM();
   endTime = clock();
   std::cout << "solve model elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "S" << std::endl;
   extractSolution();
}


void GateAsgnDriver::solveGAM()
{
   try
   {
      _gam->setGAMName("gamModel");
      std::string lpFile = "gamModel.lp";

      _gam->exportModel(lpFile);
      _gam->solveInitModel();

      std::string solFile = "gamSolution";
      _gam->exportSolution(solFile);

      //solve gam model using ilog cplex
   }
   catch (IloException)
   {
      throw("initialize gam failed");
   }
}


void GateAsgnDriver::extractSolution()
{
   try
   {
      //_gam->querySolution();
   }
   catch (...)
   {
      throw("cannot extract gam solution");
   }
}
