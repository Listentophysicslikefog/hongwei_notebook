#include <iostream>
using namespace std;
int main( int argc, char* argv[] )
{
  cout << "The program name is " << argv[0] << "." << endl;
  if( argc > 1 )
  {
    cout << "With " << argc - 1 << " args as follows:" << endl;
    for( int i = 1; i < argc; ++i )
      cout << argv[i] << endl;
  }
  else
    cout << "With " << argc - 1 << " arguments." << endl;
  return 0;
}