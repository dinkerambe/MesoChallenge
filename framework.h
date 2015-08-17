#ifndef _FRAMEWORK_H_
#define _FRAMEWORK_H_

#include <iostream>
#include <deque>
#include <list>
#include <queue>
#include "common.h"

using namespace std;

class Framework {
  public: //types
    enum Scheduling_Algo {
      FCFS=0, // first come first serve
      STCF // shortest time to completion first
    };
    struct STCF_COMP{
      bool operator()(const Job* a, const Job* b){
        return a->get_time() > b->get_time();
      }
    };
  public: // functions
    Framework(Scheduling_Algo algo,
              const string& file_name);

    void input(const string& file_name);

    // Returns true if accepts resources. 
    // Will populate task_bundle if true
    bool resource_offer(const Resource& resource,
                        list<Job>& task_bundle); 
    
    void step();  // Framework action to be looped 
    bool is_done();
  private: // functions
    void fcfs_step();  
    void stcf_step();
    // The following are the scheduling algorithms
    bool fcfs_resource_offer(const Resource& resource,
                             list<Job>& task_bundle);
    bool stcf_resource_offer(const Resource& resource,
                             list<Job>& task_bundle);

  private: // data
    Scheduling_Algo _algo;
    uint32_t _time;
    uint32_t _iter;
    bool _done;
    deque<Job> _jobs;
    queue<Job*> _fcfs_q;
    priority_queue<Job*, vector<Job*>, STCF_COMP> _stcf_q;
};

#endif // _FRAMEWORK_H_
