#include <iostream>
#include <queue>
#include <fstream>
#include <sstream>
#include <cassert>
#include <list>
#include <string>
#include "framework.h"
#include "common.h"
using namespace std;

Framework::Framework(Scheduling_Algo algo, const string& file_name) :
                       _algo(algo), _time(0),
                       _iter(0), _done(false) {
  input(file_name); 
}

void Framework::input(const string& file_name) {
  ifstream file;
  file.open(file_name);
  string line;
  uint32_t resource, time, id = 0;
  while (getline(file, line)) {
    line = line.substr(1,line.size()-2);
    stringstream ss(line);
    ss >> resource >> time;

    Job temp(id++, resource, time);
    _jobs.push_back(temp);
  }
}

void Framework::step() { 
  if (is_done()) {
    return;
  }
  if (_time >= _jobs.size()) {
    _time++;
    return;
  }

  if (_algo == FCFS) {
    fcfs_step();  
  } else if (_algo == STCF) {
    stcf_step(); 
  } else {
    assert(false);
  }
  _time++;
}

void Framework::fcfs_step() {
  _fcfs_q.push(&_jobs[_time]);
}
void Framework::stcf_step() {
  _stcf_q.push(&_jobs[_time]);
}

bool Framework::fcfs_resource_offer(const Resource& resource,
                                    list<Job>& task_bundle) {
  assert(task_bundle.empty());
  uint32_t resource_size = resource.get_resource();
  uint32_t curr_res_size = 0;

  if (_fcfs_q.empty()) {
    return false;
  }
  Job* top_job = _fcfs_q.front();
  curr_res_size += top_job->get_resource();
  if (curr_res_size > resource_size) {
    return false;
  }
  task_bundle.push_back(*top_job);
  _fcfs_q.pop();
  return true;
}

bool Framework::stcf_resource_offer(const Resource& resource,
                                    list<Job>& task_bundle) {
  assert(!is_done());
  uint32_t resource_size = resource.get_resource();
  uint32_t curr_res_size = 0;
  bool ret = false;

  // Try to add as many jobs as possible to the resource
  while (curr_res_size <= resource_size) {
    if (_stcf_q.empty()) {
      break;
    }
    Job* top_job = _stcf_q.top();
    assert(top_job != nullptr);
    curr_res_size += top_job->get_resource();
    if (curr_res_size > resource_size) {
      break;
    }
    ret = true;
    task_bundle.push_back(*top_job);
    _stcf_q.pop();
  }

  return ret;
}

bool Framework::resource_offer(const Resource& resource,
                               list<Job>& task_bundle) {
  if (is_done()) {
    return false;
  }

  if (_algo == FCFS) {
    return fcfs_resource_offer(resource, task_bundle);  
  } else if (_algo == STCF) {
    return stcf_resource_offer(resource, task_bundle); 
  } else {
    assert(false);
    return false;
  }

}

bool Framework::is_done() {
    bool time_done = (_jobs.size() <= _time);
    if (_algo == FCFS) {
      if (_fcfs_q.empty() && time_done) {
      	return true;
      }
    } else if (_algo == STCF) {
      if (_stcf_q.empty() && time_done) {
      	return true;
      }
    } else {
      assert(false);
    }
    return false;
}
