#pragma once
#include <string>
#include <iostream>
#include <memory>

class Ticket
{
public:
   Ticket();
   Ticket(const std::string id, int num, const std::string arrflight, const std::string arrdate,
   const std::string depflight, const std::string depdate);
   ~Ticket();



   const std::string getpaxID() const { return paxID; }
   int getPaxNum() { return paxNum; }
   std::string getarrFlight() { return arrFlight; }
   std::string getarrDate() { return arrDate; }
   std::string getdepFlight() { return depFlight; }
   std::string getdepDate() { return depDate; }


   void setpaxId(const std::string & val) { paxID = val; }

   void setpaxNum(const int val) { paxNum = val; }
   void setpaxNum(const std::string val);
   void setarrFlight(const std::string & val) { arrFlight = val; }
   void setarrDate(const std::string & val) { arrDate = val; }
   void setdepFlight(const std::string & val) { depFlight = val; }
   void setdepDate(const std::string & val) { depDate = val; }


   struct sortAlphabeticPtr
   {
      bool operator()(const std::shared_ptr<Ticket> tk1, const std::shared_ptr<Ticket> tk2) const
      {
         return (tk1->getpaxID() < tk2->getpaxID());
      }
   };



private:
   std::string paxID;
   int paxNum;
   std::string arrFlight;
   std::string arrDate;
   std::string depFlight;
   std::string depDate;


};