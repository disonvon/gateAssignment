#pragma once
#include <string>
#include<iostream>

class Puck
{
public:
   Puck();
   Puck(const std::string & puckid, const std::string & arrdate, int arrminute, const std::string & arrrflight,
      const std::string & arrtype, const std::string & actype, const std::string & depdata, int depminute,
      const std::string & depflight, const std::string deptype, const std::string & upairp,
      const std::string & downairp, const std::string & puckbodytype);

   ~Puck();



   void setpuckId(const std::string & val) { puckID = val; }
   void setarrDate(const std::string & val) { arrDate = val; }
   void setarrMinute(const int & val) { arrMinute = val; };
   void setarrFlight(const std::string & val) { arrFlight = val; }
   void setarrType(const std::string & val) { arrType = val; }
   void setacType(const std::string & val) { acType = val; }
   void setdepDate(const std::string & val) { depDate = val; }
   void setdepMinute(const int & val) { depMinute = val; };
   void setdepFlight(const std::string & val) { depFlight = val; }
   void setdepType(const std::string & val) { depType = val; }
   void setupAirport(const std::string & val) { upAirport = val; }
   void setdownAirport(const std::string & val) { downAirport = val; }
   void setpuckBodyType(const std::string & val) { puckBodyType = val; }


   const std::string getpuckId() const { return puckID; }
   const std::string getarrDate() const { return arrDate; }
   int getarrMinute() const { return arrMinute; }
   const std::string  getarrFlight() const { return arrFlight; }
   const std::string  getarrType() const { return arrType; }
   const std::string  getacType() const { return acType; }
   const std::string  getdepDate() const { return depDate; }
   int getdepMinute() const { return depMinute; }
   const std::string  getdepFlight() const { return depFlight; }
   const std::string  getdepType() const { return depType; }
   const std::string  getupAirport() const { return upAirport; }
   const std::string  getdownAirport() const { return downAirport; }
   const std::string  getpuckBodyType() const { return puckBodyType; }



   struct sortAlphabeticPtr
   {
      bool operator()(const std::shared_ptr<Puck> pk1, const std::shared_ptr<Puck> pk2) const
      {
         return (pk1->getpuckId() < pk2->getpuckId());
      }
   };

private:
   std::string puckID;
   std::string arrDate;
   int arrMinute;
   std::string arrFlight;
   std::string arrType;
   std::string acType;
   std::string depDate;
   int depMinute;
   std::string depFlight;
   std::string depType;
   std::string upAirport;
   std::string downAirport;
   std::string puckBodyType;


};