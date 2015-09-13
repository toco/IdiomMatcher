#include <idc.idc>
static main()
{
  Message("Waiting for the end of auto analys...\n");
  Wait();
  
  RunPlugin("IdiomMatcher", 1);
  Wait();
  Wait();
  Wait();
  Message("Starting to dump to JSON...\n");
  dumpToJSON();
  Exit(0); // exit to OS, error code 0 - success
}
t