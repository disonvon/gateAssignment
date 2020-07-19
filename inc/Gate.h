#pragma once
#include <string>


class Gate
{

public:
   Gate();
   Gate(const std::string & id, const std::string & term, const std::string regn, const std::string & arrty,
      const std::string & depty, const std::string & bodyty);
   ~Gate();


   void setgateId(const std::string & val) { gateID = val; }
   void setterminal(const std::string & val) { terminal = val; }
   void setRegion(const std::string & val) { region = val; }
   void setarrType(const std::string & val) { arrType = val; };
   void setdepType(const std::string & val) { depType = val; }
   void setbodyType(const std::string & str) { bodyType = str; }

   const std::string getgateID() const { return gateID; }
   std::string getterminal() const { return terminal; }
   std::string getregion() const { return region; }
   std::string getarrType() const { return arrType; }
   std::string getdepType() const { return depType; }
   std::string getbodyType() const { return bodyType; }


   struct sortAlphabeticPtr
   {
      bool operator()(const Gate* gt1, const Gate* gt2) const
      {
         return (gt1->getgateID() < gt2->getgateID());
      }
   };


private:
   std::string gateID;
   std::string terminal;
   std::string region;
   std::string arrType;
   std::string depType;
   std::string bodyType;



};
