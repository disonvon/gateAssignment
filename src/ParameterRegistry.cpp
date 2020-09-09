#include "../inc/ParameterRegistry.h"

ParameterRegistry* ParameterRegistry::_paraInstance = nullptr;


ParameterRegistry::ParameterRegistry()
   : objFunction(gateAssignment)
, normalGateAssignBonus(-100)
, remoteGateAssignBonus(10)
, fixedGatePenalty(50)
, remoteFixedGatePenalty(1000)
, paxConnectPenaltyPerMinute(0.001)
, totalNumTabuIter(10)
, timeLimitation(300)
{
   
}

ParameterRegistry::~ParameterRegistry()
{

}



