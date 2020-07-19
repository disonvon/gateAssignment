#include "../inc/Ticket.h"


Ticket::Ticket()
   :paxID(""),
   paxNum(0),
   arrFlight(""),
   arrDate(""),
   depFlight(""),
   depDate("")
{

}


Ticket::Ticket(const std::string id, int num, const std::string arrflight, const std::string arrdate,
   const std::string depflight, const std::string depdate)
   :paxID(id),
   paxNum(num),
   arrFlight(arrflight),
   arrDate(arrdate),
   depFlight(depflight),
   depDate(depdate)
{

}


Ticket::~Ticket()

{

}

void Ticket::setpaxNum(const std::string val)
{
   paxNum = std::stoi(val);
}