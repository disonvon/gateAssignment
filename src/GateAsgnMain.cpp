#include "../inc/GateAsgnDriver.h"


int main(int argc, char* argv[])
{
   GateAsgnDriver gateDriver;

   gateDriver.readConfigurationFile();

   gateDriver.optimize();

   system("pause");


   return 0;
}
