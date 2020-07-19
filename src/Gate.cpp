#include "../inc/Gate.h"


Gate::Gate()
   :gateID(""),
   terminal(""),
   region(""),
   arrType(""),
   depType(""),
   bodyType("")
{

}


Gate::Gate(const std::string & id, const std::string & term, const std::string regn, const std::string & arrty,
   const std::string & depty, const std::string & bodyty)
   :gateID(id),
   terminal(term),
   region(regn),
   arrType(arrty),
   depType(depty),
   bodyType(bodyty)
{

}

Gate::~Gate()
{

}



