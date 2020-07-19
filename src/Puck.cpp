#include "../inc/Puck.h"


Puck::Puck()
   :puckID(""),
   arrDate(""),
   arrMinute(0),
   arrFlight(""),
   arrType(""),
   acType(""),
   depDate(""),
   depMinute(0),
   depFlight(""),
   depType(""),
   upAirport(""),
   downAirport(""),
   puckBodyType("")
{

}


Puck::Puck(const std::string & puckid, const std::string & arrdate, int arrminute, const std::string & arrrflight,
   const std::string & arrtype, const std::string & actype, const std::string & depdata, int depminute,
   const std::string & depflight, const std::string deptype, const std::string & upairp,
   const std::string & downairp, const std::string & puckbodytype)
   :puckID(puckid),
   arrDate(arrdate),
   arrMinute(arrminute),
   arrFlight(arrrflight),
   arrType(arrtype),
   acType(actype),
   depDate(depdata),
   depMinute(depminute),
   depFlight(depflight),
   depType(deptype),
   upAirport(upairp),
   downAirport(downairp),
   puckBodyType(puckbodytype)
{
   
}

Puck::~Puck()
{

}

//void Puck::setarrMinute(const std::string & val)
//{
//   int h, m;
//   if (sscanf_s(val.c_str(), "%d:%d", &h, &m) >= 1)
//   {
//      arrMinute = h * 60 + m;
//   }
//   else
//   {
//      std::cout << "parse time/minute error" <<std::endl;
//   }
//}
//
//void Puck::setdepMinute(const std::string & val)
//{
//   int h, m;
//   if (sscanf_s(val.c_str(), "%d:%d", &h, &m) >= 1)
//   {
//      depMinute = h * 60 + m;
//   }
//   else
//   {
//      std::cout << "parse time/minute error" << std::endl;
//   }
//}