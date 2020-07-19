#pragma once
#include <map>
#include <vector>
#include "Gate.h"
#include "Puck.h"

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

   const std::vector<Gate *>& getGates() const
   {
      return _gates;
   }


   const std::vector<Puck *>& getPucks() const
   {
      return _pucks;
   }


   const std::vector<Ticket *>& getTickets() const
   {
      return _tickets;
   }

   const std::vector<Ticket *>& getIncludedTickets() const
   {
      return _includedTickets;
   }

   const std::vector<Puck *>& getIncludedPucks() const
   {
      return _includedPucks;
   }

   int getMinute(const std::string & val);

   void addGates(Gate * gate);
   void addPucks(Puck * puck);
   void addIncludedPucks(Puck * puck);
   void addTickets(Ticket * ticket);



   void readGate();
   void readPuck();
   void readTicket();
   void preprocess();
   //std::vector<Puck *> getincludedPucks() { return _includedPucks; }
   //std::vector<Gate *> getGates() { return _gates; }


public:
   std::vector<Gate *> _gates; 
   std::vector<Puck *> _pucks;
   std::vector<Puck *> _includedPucks;
   std::vector<Ticket *> _tickets;
   std::vector<Ticket *> _includedTickets;

   std::map<const std::string, std::pair<int, int>> ticketsPuckIdx;




private:
   static DataManager * _instance;
   //static DataManager * _dataManager;


};
