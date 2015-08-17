#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <queue>
#include <cassert>
#include <list>
#include <sstream>
#include <unordered_map>
using namespace std;

class Job {
  public:
    Job() {}
    Job(uint32_t id, uint32_t resource, uint32_t time) :
      _id(id), _resource(resource), _time(time) { } 
    
    uint32_t get_id() const { return _id; }
    uint32_t get_resource() const { return _resource; }
    uint32_t get_time() const { return _time; }
    bool decrement_time() {
      assert(_time != 0);
      if (--_time == 0) {
      	return true;
      }
      return false;
    }
        
  private:
    uint32_t _id;
    uint32_t _resource;
    uint32_t _time;

};

class Resource {
 public:
    Resource() {}
    Resource(uint32_t id, uint32_t resource) :
      _node_id(id), _resource(resource) { } 
    
    uint32_t get_id() const { return _node_id; }
    uint32_t get_resource() const { return _resource; }

    void add_resource(uint32_t resource) {
      _resource += resource;
    }
    bool sub_resource(uint32_t resource) {
      assert(resource <= _resource);
      _resource -= resource;
      return (_resource == 0);
    }
  private:
    uint32_t _node_id;
    uint32_t _resource;
};

// O(1) access to consolodate resources for a particular node
// List used to dispense resources while rotating if rejected
class HashList {
  public:
    void push_back(Resource &r) {
      if (_hashmap.find(r.get_id()) == _hashmap.end()) {
      	_resources.push_back(r);
      	_hashmap.emplace(r.get_id(),&_resources.back());
      } else {
        _hashmap[r.get_id()]->add_resource(r.get_resource());
      }
    }
    Resource& front() {
      return _resources.front();
    }
    void pop_front() {
      Resource res = _resources.front();
      _resources.pop_front();
      _hashmap.erase(res.get_id());
    }
    uint32_t size() {
      return _resources.size();
    }
    bool empty() {
      return _resources.empty();
    }
    uint32_t total_resources() {
      uint32_t total =0;
      for (auto it = _resources.begin(); it != _resources.end(); it++) {
        total += it->get_resource();
      }
      return total;
    }
  private:
    list<Resource> _resources;
    unordered_map<int, Resource*> _hashmap;
};

namespace GLOBAL {
  extern stringstream ss;
  void print_resource(uint32_t time, const Resource& res);
  void print_job(uint32_t time, const Job& job, bool start);
}

#endif // _COMMON_H_
