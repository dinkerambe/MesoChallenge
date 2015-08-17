#include <iostream>
#include <fstream>
#include "framework.h"
#include "common.h"
#include "master.h"
using namespace std;

const string FCFS_FILE = "fcfs.outfile";
const string STCF_PARALLEL_FILE = "stcf_parallel.outfile";

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr << "Need to input the following parameters:\n"
         << "(1) Path to Resource Input File\n"
         << "(2) Path to Job Input File\n"
         << "./scheduler.out Resources.input Jobs.input\n"
         << endl;
  }

  ofstream file;

  GLOBAL::ss << "*************** FCFS *****************\n\n";
  Framework fw(Framework::FCFS, argv[2]);
  Master master(argv[1]);
  master.register_framwork(&fw);

  while (!master.is_done()) {
    master.step();
  }
  file.open(FCFS_FILE); 
  file << GLOBAL::ss.str();
  file.close();
  
  // ============================= reset ===================================
  GLOBAL::ss.str(std::string());
  reset_metrics();

  GLOBAL::ss << "*************** STCF *****************\n\n";
  Framework fw1(Framework::STCF, argv[2]);
  Master master1(argv[1]);
  master1.register_framwork(&fw1);

  while (!master1.is_done()) {
    master1.step();
  }

  file.open(STCF_PARALLEL_FILE); 
  file << GLOBAL::ss.str();
  file.close();

  return 0;
}
