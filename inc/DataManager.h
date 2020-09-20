#pragma once
#include<iostream>
#include <map>
#include <memory>
#include <vector>
#include "Gate.h"
#include "Puck.h"

class ParameterRegistry;
class Ticket;

class DataManager
{

public:
   static DataManager * instance()
   {
      if (!_instance)
      {
         _instance = new DataManager();
      }
      return _instance;
   }

   static void clean()
   {
      if (_instance)
      {
         delete _instance;
         _instance = NULL;
      }
   }
   DataManager();
   ~DataManager();

   //void readData();

   typedef std::map<std::string, std::string> mapAcType;
   static mapAcType acTypeInfo;

   static std::map<std::string, std::pair<int, int> > transitPattern;

   const std::vector<std::shared_ptr<Gate>>& getGates() const
   {
      return _gates;
   }


   const std::vector<std::shared_ptr<Puck>>& getPucks() const
   {
      return _pucks;
   }


   const std::vector<std::shared_ptr<Ticket>>& getTickets() const
   {
      return _tickets;
   }

   const std::vector<std::shared_ptr<Ticket>>& getIncludedTickets() const
   {
      return _includedTickets;
   }

   const std::vector<std::shared_ptr<Puck>>& getIncludedPucks() const
   {
      return _includedPucks;
   }

   int getMinute(const std::string & val);

   void addGates(std::shared_ptr<Gate> gate);
   void addPucks(std::shared_ptr<Puck> puck);
   void addIncludedPucks(std::shared_ptr<Puck> puck);
   void addTickets(std::shared_ptr<Ticket> ticket);



   void readGate(const std::string &dirName);
   void readPuck(const std::string& dirName);
   void readTicket(const std::string& dirName);
   void preprocess();
   //std::vector<std::shared_ptr<Puck>> getincludedPucks() { return _includedPucks; }
   //std::vector<std::shared_ptr<Gate>> getGates() { return _gates; }


public:
   std::vector<std::shared_ptr<Gate>> _gates; 
   std::vector<std::shared_ptr<Puck>> _pucks;
   std::vector<std::shared_ptr<Puck>> _includedPucks;
   std::vector<std::shared_ptr<Ticket>> _tickets;
   std::vector<std::shared_ptr<Ticket>> _includedTickets;

   std::map<const std::string, std::pair<int, int>> ticketsPuckIdx;




private:
   static DataManager * _instance;


};
