#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "master.h"
#include "framework.h"
#include "common.h"
using namespace std;

stringstream GLOBAL::ss;
// Utilities
static uint32_t _time = 0;
static uint32_t _total_resources = 0;
void reset_metrics() {
  _time = 0;
  _total_resources = 0;
}

void GLOBAL::print_resource(uint32_t time, const Resource& res) {
  GLOBAL::ss << "time " << time << " Resource: (" << res.get_id() << " "
       << res.get_resource() << ")" << endl;
}

void GLOBAL::print_job(uint32_t time, const Job& job, bool start) {
  if (start)
    GLOBAL::ss << "time " << time << " Job start: id(";
  else
    GLOBAL::ss << "time " << time << " *** Job end ***: id(";
    
  GLOBAL::ss << job.get_id() << ") ("
       << job.get_resource() << " " << job.get_time() << ")" << endl;
}
// ================================ MASTER ================================
Master::Master(const string& file_name) :
                 _framework(nullptr), _slaves() {
  input(file_name);

}

void Master::input(const string& file_name) {
  ifstream file;
  file.open(file_name);
  string line;
  uint32_t id, resource;
  while (getline(file, line)) {
    line = line.substr(1,line.size() - 2);
    stringstream ss(line);
    ss >> id >> resource;

    Resource temp(id, resource);
    _init_res.push_back(temp);
  }
}

void Master::register_framwork(Framework* fw) {
  _framework = fw;
}

uint32_t Master::total_resources() {
  return _resources.total_resources();
}

void Master::get_next_init_resource() {
  if (!_init_res.empty()) {
    Resource res = _init_res.front();
    _init_res.pop_front();
    _resources.push_back(res);
    _total_resources += res.get_resource();
    GLOBAL::print_resource(_time, res);
  }
}

void Master::step() {
  assert(_framework != nullptr);

  // Get next resource from initial set
  get_next_init_resource();

  // Get all free resources from slaves and consolidate them
  vector<Resource> res_list = _slaves.get_free_resources();
  for (auto &&res : res_list) {
    _resources.push_back(res);
    GLOBAL::print_resource(_time, res);
  }

  // Make the framework get its next jobs
  _framework->step();

  // Try to offer framework resources
  if (!_resources.empty()) {
    Bundle bundle;
    bundle.res = _resources.front();
    _resources.pop_front();

    if (_framework->resource_offer(bundle.res, bundle.jobs)) { // accepted offer
      assert(!bundle.jobs.empty());
      _slaves.assign_tasks(bundle);
    } else { // rejected offer
      _resources.push_back(bundle.res); // put resource on back of list
    }
  }

  assert_total_resources();
  _slaves.step();
  assert_total_resources();
  
  // Advance time tick
  _time++;
}

void Master::assert_total_resources() {
  uint32_t t_res = _slaves.total_resources() + total_resources();
  assert(t_res == _total_resources);
}

bool Master::is_done() {
  if (_framework->is_done() && _slaves.is_done()) {
    return true;
  }
  return false;
}

// ================================ SLAVES ===================================
Slaves::Slaves() :
  _my_time(0) { }

void Slaves::assign_tasks(Bundle bundle) {
  // cerr << "MYTIME: " << _my_time << " TIME: " << _time << endl;
  assert(_my_time == _time);
  assert(!bundle.jobs.empty());
  for (auto it = bundle.jobs.begin(); it != bundle.jobs.end(); ++it) {
    GLOBAL::print_job(_time, *it, true);
  }

  _compute_cluster.push_back(bundle);

}

void Slaves::step() {
  // all compute nodes
  _my_time++;
  for (auto it = _compute_cluster.begin(); it != _compute_cluster.end();) { // Each Bundle
    
    // find total number resources released and jobs finished
    list<Job> *job_bundle = &(it->jobs);
    for (auto job_it = job_bundle->begin(); job_it != job_bundle->end();) { // Each Job in each bundle
      // decrement each job time
      if (job_it->decrement_time()) { // Job is done
        uint32_t num_resources_released = job_it->get_resource(); 

        assert (num_resources_released <= it->res.get_resource());
        // Free the resource the job was using
        Resource free_res(it->res.get_id(), num_resources_released);

        it->res.sub_resource(num_resources_released);
        _free_resources.push_back(free_res);

        GLOBAL::print_job(_time, *job_it, false);
        job_it = job_bundle->erase(job_it); 
      } else {
      	++job_it;
      }
    }

    // If the bundle has no jobs, free resources
    if (it->jobs.empty()) {
        int num_res = it->res.get_resource();
        Resource free_res(it->res.get_id(), num_res);
        _free_resources.push_back(free_res);
        it->res.sub_resource(num_res);
    }

    if (it->res.get_resource() == 0) { // erase resource bundle
      // make sure no jobs are left in bundle
      assert(it->jobs.empty());
      assert(it->res.get_resource() == 0);
      // update iterator
      it = _compute_cluster.erase(it);
    } else {
      // update it
      ++it;
    }

  }
}

vector<Resource> Slaves::get_free_resources() {
  vector<Resource> ret = _free_resources; 
  _free_resources.clear();
  return ret;
}

bool Slaves::is_done() {
  return (_compute_cluster.empty() && _my_time > 0);
}

uint32_t Slaves::total_resources() {
  uint32_t total = 0;
  for (auto it =_compute_cluster.begin(); it != _compute_cluster.end(); it++){
    total += it->res.get_resource();
  }
  total += free_resources(); 
  return total;
}

uint32_t Slaves::free_resources() {
  uint32_t total = 0;
  for (auto &&res : _free_resources) {
    total += res.get_resource();
  }
  return total;
}
