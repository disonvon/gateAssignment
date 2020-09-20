#include <iostream>
#include <sstream>
#include <fstream>

#include "../inc/DataManager.h"
#include "../inc/Puck.h"
#include "../inc/Ticket.h"
#include "../inc/ParameterRegistry.h"


DataManager * DataManager::_instance = nullptr;








std::map<std::string, std::string> DataManager::acTypeInfo = { { "332", "W" },{ "333", "W" },{ "33E", "W" },
                                                                        { "33H", "W" }, { "33L", "W" }, { "773", "W" },
                                                                  {"319","N"}, {"320", "N"}, {"321", "N"}, {"323", "N"},
                                                                     {"325", "N"}, {"738", "N"},
                                                               {"73A", "N"}, {"73E", "N"}, {"73H", "N"}, {"73L", "N"} };


std::map<std::string, std::pair<int, int>> DataManager::transitPattern =
                                                         { {"DT-DT", {15, 0}}, {"DT-DS", {20, 1}}, {"DS-DT", {20, 1}}, {"DS-DS", {15, 0}},
                                                         {"DT-IT",{35, 0}}, {"DT-IS", {40, 1}}, {"DS-IT", {40, 1}}, {"DS-IS", {35, 0}},
                                                         {"IT-DT", {35, 0}}, {"IT-DS", {40, 1}}, {"IT-IT", {20, 0}}, {"IT-IS", {30, 1}},
                                                         {"IS-DT", {40, 1}}, {"IS-DS", {45, 2}}, {"IS-IT", {30, 1}}, {"IS-IS", {20, 0}} };




DataManager::DataManager()
{
   
}

DataManager::~DataManager()
{
}




int DataManager::getMinute(const std::string & val)
{
   int h, m, minute;
   if (sscanf_s(val.c_str(), "%d:%d", &h, &m) >= 1)
   {
      minute = h * 60 + m;
   }
   else
   {
      std::cout << "parse time/minute error" << std::endl;
   }
   return minute;
}
void DataManager::addGates(std::shared_ptr<Gate> gate)
{
   _gates.push_back(gate);
}

void DataManager::addPucks(std::shared_ptr<Puck> puck)
{
   _pucks.push_back(puck);
}

void DataManager::addTickets(std::shared_ptr<Ticket> ticket)
{
   _tickets.push_back(ticket);
}

void DataManager::addIncludedPucks(std::shared_ptr<Puck> puck)
{
   _includedPucks.push_back(puck);
}


void DataManager::readPuck(const std::string& dirName)
{
   std::string inputFile = dirName + "data/puck.csv";
   std::ifstream puck (inputFile);
   std::string line;
   if (puck.good())
   {
      int lineNumber = 0;
      Puck tempPuck;
      while (std::getline(puck, line))
      {
         if (lineNumber >= 1)
         {
            // Create a stringstream of the current line
            std::istringstream ss(line);
            std::vector<std::string> fields;
            std::string field;
            while (getline(ss, field, ','))
            {
               fields.push_back(field);
               //std::cout << "line:" << lineNumber << ", " << field << std::endl;
            }
            std::string puckID, arrDate, arrFlight, arrType, acType, depDate, depFlight, depType, upAirport, downAirport;
            int arrMinute, depMinute;
            
            puckID = fields[0];
            arrDate = fields[1];
            arrMinute = getMinute(fields[2]);
            arrFlight = fields[3];
            arrType = fields[4];
            acType = fields[5];
            depDate = fields[6];
            depMinute = getMinute(fields[7]);
            depFlight = fields[8];
            depType = fields[9];
            upAirport = fields[10];
            downAirport = fields[11];

            std::shared_ptr<Puck> puckptr( new Puck(puckID, arrDate, arrMinute, arrFlight, arrType, acType,
               depDate, depMinute, depFlight, depType, upAirport, downAirport, ""));

            _pucks.push_back(puckptr);
         }
         lineNumber++;
      }
   }
}

void DataManager::readTicket(const std::string& dirName)
{
   std::string inputFile = dirName + "data/tickets.csv";
   std::ifstream ticket(inputFile);
   std::string line;

   if (ticket.good())
   {
      int lineNumber = 0;
      //Gate * gateptr = DataManager::instance()->_gates[lineNumber];
      Ticket * ticketptr = new Ticket();
      while (std::getline(ticket, line))
      {
         if (lineNumber >= 1)
         {
            // Create a stringstream of the current line
            std::istringstream ss(line);
            std::vector<std::string> fields;
            std::string field;
            while (getline(ss, field, ','))
            {
               fields.push_back(field);
               //std::cout << "line:" << lineNumber << ", " << field << std::endl;
            }

            std::string id, arrflight, arrdate, depflight, depdate;
            int num;
            id = fields[0];
            num = std::stoi(fields[1]);
            arrflight = fields[2];
            arrdate = fields[3];
            depflight = fields[4];
            depdate = fields[5];


            std::shared_ptr<Ticket> ticketPtr( new Ticket(id, num, arrflight, arrdate, depflight, depdate));

            _tickets.push_back(ticketPtr);
         }
         lineNumber++;
      }

   }
}

void DataManager::readGate(const std::string& dirName)
{
   std::string inputFile = dirName + "data/gates.csv";
   std::ifstream gate(inputFile);
   std::string line;

   if (gate.good())
   {
      int lineNumber = 0;
      //Gate * gateptr = DataManager::instance()->_gates[lineNumber];
      Gate * gatetptr = new Gate();
      while (std::getline(gate, line))
      {
         if (lineNumber >= 1)
         {
            // Create a stringstream of the current line
            std::istringstream ss(line);
            std::vector<std::string> fields;
            std::string field;
            while (getline(ss, field, ','))
            {
               fields.push_back(field);
               //std::cout << "line:" << lineNumber << ", " << field << std::endl;
            }

            std::string id, term, regn, arrty, depty, bodyty;

            id = fields[0];
            term = fields[1];
            regn = fields[2];
            arrty = fields[3];
            depty = fields[4];
            bodyty = fields[5];


            std::shared_ptr<Gate> gatekptr ( new Gate(id, term, regn, arrty, depty, bodyty));

            _gates.push_back(gatekptr);
         }
         lineNumber++;
      }

   }
}




void DataManager::preprocess()
{
   try
   {
      //initialize puck body type info
      //auto pucks = this->getPucks();
      //add body type info for each puck
      for (auto puck : _pucks)
      {
         mapAcType::const_iterator pos = acTypeInfo.find(puck->getacType());
         if (pos == acTypeInfo.end())
         {
            std::cout << "warning: cannot find body type info for equipment: " << puck->getacType() << std::endl;
         }
         else {
            puck->setpuckBodyType(pos->second);
         }
      }


      //convert time type into minutes
      std::string startDate = "20-Jan-18";
      std::string endDate = "20-Jan-18";
      int today = std::stoi(startDate.substr(0, 2));
      for (auto puck : _pucks)
      {
         int arrDay = std::stoi(puck->getarrDate().substr(0, 2));
         int depDay = std::stoi(puck->getdepDate().substr(0, 2));

         int numMinuteDay = 1440;
         int tempDepMinute = puck->getdepMinute() + (depDay - today)*numMinuteDay;
         int tempArrMinute = puck->getarrMinute() + (arrDay - today)*numMinuteDay;

         puck->setdepMinute(tempDepMinute);
         puck->setarrMinute(tempArrMinute);

      }




      // only consider pucks in startDate and endDate

      for (auto puck : _pucks)
      {
         if (puck->getarrDate() == startDate || puck->getdepDate() == endDate)
         {
            _includedPucks.push_back(puck);
         }
      }

      for (auto ticket : _tickets)
      {
         if (ticket->getarrDate() == startDate || ticket->getdepDate() == endDate)
         {
            _includedTickets.push_back(ticket);
         }
      }

      //init ticket corresponding puck idx, ticket departure leg pair with 
      for (auto ticket : _includedTickets)
      {

         int tempArrIdx = -1, temDepIdx = -1;

         int itr1 = 0;
         for (auto puck : _includedPucks)
         {
            if (ticket->getarrDate() == puck->getarrDate() && ticket->getarrFlight() == puck->getarrFlight())
            {
               tempArrIdx = itr1;
               break;
            }
            ++itr1;
         }

         int itr2 = 0;
         for (auto puck : _includedPucks)
         {
            if (ticket->getdepDate() == puck->getdepDate() && ticket->getdepFlight() == puck->getdepFlight())
            {
               temDepIdx = itr2;
               break;
            }
            ++itr2;
         }
         if ((tempArrIdx != -1) && (temDepIdx != -1))
         {
            ticketsPuckIdx[ticket->getpaxID()].first = tempArrIdx;
            ticketsPuckIdx[ticket->getpaxID()].second = temDepIdx;
         }


      }


   }
   catch (...)
   {
      throw("DataManger::preprocess failed");
   }

}
