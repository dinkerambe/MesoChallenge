#ifndef _MASTER_H_
#define _MASTER_H_

#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include "framework.h"
#include "common.h"
using namespace std;


struct Bundle{
  Bundle() {}
  Resource res;
  list<Job> jobs;
};

class Slaves {
  public:
    Slaves();
    void assign_tasks(Bundle);
    vector<Resource> get_free_resources(); // returns free resources
    void step(); // Slave action to be looped 
    bool is_done();
    uint32_t total_resources();
    uint32_t free_resources();
  private:
    // Node id to a bundle of resource,jobs 
    list<Bundle> _compute_cluster;
    vector<Resource> _free_resources;
    uint32_t _my_time;
};

class Master {
  public:
    Master(const string& file_name);
    void input(const string& file_name);
    void register_framwork(Framework* fw);
    void step(); // Main loop
    bool is_done();
    uint32_t total_resources();
  private: //functions
    void assert_total_resources();
    void get_next_init_resource();
  private:
    Framework* _framework;
    Slaves _slaves;
    HashList _resources;
    list<Resource> _init_res;
};

void reset_metrics();

#endif //_MASTER_H_
