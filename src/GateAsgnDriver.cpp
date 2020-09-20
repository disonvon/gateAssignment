#include <ctime>

#include "../inc/GateAsgnDriver.h"

#include "../inc/DataManager.h"
#include "../inc/ParameterRegistry.h"


GateAsgnDriver::GateAsgnDriver()
   :_gam(new gam(_env))
,paraRegistry(ParameterRegistry::instance())
,dataMgr(DataManager::instance())
{
   //_env.setDeleter(IloLinearDeleterMode);
}


GateAsgnDriver::~GateAsgnDriver()
{
   _gam.reset();
   //_env.end(); // release Ilog concert environment

}

void GateAsgnDriver::readConfigurationFile()
{
   std::string configFile = "./config.txt";
   std::ifstream inputFile(configFile.c_str());
   if (!inputFile)
   {
      return;
   }

   std::string fieldName[100];
   short dataValue[100];
   int i = 0;
   std::cout << "\nReading the configuration file" << std::endl;
   while (!inputFile.eof())
   {
      inputFile >> fieldName[i];
      if (fieldName[i] == "END")
      {
         break;
      }
      else if (fieldName[i] == "SCENE_DIRECTORY")
      {
         inputFile >> paraRegistry->directory;
         for (int k = 0; k < paraRegistry->directory.size(); k++)
         {
            if (paraRegistry->directory[k] == '\\')
            {
               paraRegistry->directory[k] = '/';
            }
         }
         paraRegistry->directory += "/";

         std::cout << fieldName[i] << "     " << paraRegistry->directory << std::endl;
         i++;
         continue;
      }

      inputFile >> dataValue[i];

      if (fieldName[i] == "DEBUG_VERBOSE")
      {
         if (dataValue[i] == 1 || dataValue[i] == 0)
         {
            paraRegistry->setParameters_dataAnalysis(dataValue[i] > 0);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }
      else  if (fieldName[i] == "PROBLEMTOSOLVE")
      {
         if (dataValue[i] == 1 || dataValue[i] == 2 || dataValue[i] == 3)
         {
            paraRegistry->setParameters_OBJECTIVE_FUNCTION(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }
      else if (fieldName[i] == "TAUBOOITERATIONS")
      {
         if (dataValue[i] >= 0)
         {
            paraRegistry->setParameters_totalNumTabuIter(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }
      else if (fieldName[i] == "CPLEXTIMELIMITATION")
      {
         if (dataValue[i] >= 0)
         {
            paraRegistry->setParameters_timeLimitation(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }
      else if (fieldName[i] == "NORMALGATEASSIGNBONUS")
      {
         if (dataValue[i] >= -100000000000000)
         {
            paraRegistry->setParameters_normalGateAssignBonus(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }

      else if (fieldName[i] == "REMOTEGATEASSIGNBONUS")
      {
         if (dataValue[i] >= -100000000000000)
         {
            paraRegistry->setParameters_remoteGateAssignBonus(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }
      else if (fieldName[i] == "FIXEDGATECOST")
      {
         if (dataValue[i] >= -100000000000000)
         {
            paraRegistry->setParameters_fixedGatePenalty(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }

      else if (fieldName[i] == "REMOTEGATEFIXEDCOST")
      {
         if (dataValue[i] >= -100000000000000)
         {
            paraRegistry->setParameters_remoteFixedGatePenalty(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }
      else if (fieldName[i] == "PAXCONNCOSTPERMIN")
      {
         if (dataValue[i] >= -100000000000000)
         {
            paraRegistry->setParameters_paxConnectPenaltyPerMinute(dataValue[i]);
            std::cout << fieldName[i] << "       " << dataValue[i] << std::endl;
         }
      }

      i++;
      if (i == 50)
      {
         break;
      }
   }
   inputFile.close();
   return;
}


void GateAsgnDriver::optimize()
{
   //***************read data**********************************
   clock_t startTime, endTime;
   startTime = clock();
   dataMgr->readPuck(paraRegistry->directory);
   dataMgr->readGate(paraRegistry->directory);
   dataMgr->readTicket(paraRegistry->directory);
   endTime = clock();

   std::cout << "read data elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
   //***************preprocess data**********************************
   startTime = clock();
   dataMgr->preprocess();
   endTime = clock();
   std::cout << "preprocess data elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;

   //***************build and solve model elapsed*********************
   startTime = clock();
   solveModel();
   endTime = clock();
   std::cout << "build and solve model elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;

   //extract solution



}

void GateAsgnDriver::initializeGAM()
{
   // *****************add data to gam******************
   try
   {
      _gam->addGateData(dataMgr->_gates);//all gates
      _gam->addPuckData(dataMgr->_includedPucks);
      _gam->addTicketData(dataMgr->_includedTickets);//included tickets
      _gam->addTicketPuckIdxData(dataMgr->ticketsPuckIdx);


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
   std::cout << "model initialized elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;


   startTime = clock();
   solveGAM();
   endTime = clock();
   std::cout << "solve model elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
   extractSolution();
}


void GateAsgnDriver::solveGAM()
{
   try
   {  //solve gam model using ilog cplex
      _gam->setGAMName("gamModel");

      _gam->solveInitModel();

      if (paraRegistry->dataAnalysis)
      {
         std::string lpFile =  paraRegistry->directory + "gamModel.lp";
         _gam->exportModel(lpFile);
         std::string solFile = paraRegistry->directory + "gamSolution.xml";
         _gam->exportSolution(solFile);
      }


      //tabu search
      if (paraRegistry->objFunction == 3)
      {
         clock_t startTime, endTime;
         startTime = clock();
         _gam->runTabuSearchModel();
         endTime = clock();
         std::cout << "tabu search elapsed : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
      }

      exportSolution();

   }
   catch (IloException)
   {
      throw("solve gam failed");
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


void GateAsgnDriver::exportSolution()
{
   //export solution file

   std::ofstream solutionStrm;
   //const std::string debugFileName = paraRegistry->debugDirectory + "solution.csv";
   const std::string debugSolutionFileName = paraRegistry->directory + "solution.csv";
   solutionStrm.open(debugSolutionFileName.c_str());
   if (solutionStrm.good())
   {
      solutionStrm << "PUCKID, GATEID, START, END" << std::endl;

      for (int iGate = 0; iGate < _gam->getGates().size(); ++iGate)
      {
         auto iGateAssigned = _gam->solutionGateAssginedPuck[iGate];
         for (const auto iPuck : iGateAssigned)
         {
            solutionStrm << _gam->getPucks()[iPuck]->getpuckId() << "," <<
               _gam->getGates()[iGate]->getgateID() << "," << _gam->getPucks()[iPuck]->getarrMinute() << "," << _gam->getPucks()[iPuck]->getdepMinute() << std::endl;

         }
      }

   }
   solutionStrm.close();
}
