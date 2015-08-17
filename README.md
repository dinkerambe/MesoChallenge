Mesosphere Challenge
====================
Datacenter with Resources and Jobs

Build
=====
Must be done before calling run
```
make
```
Run
===
For small sized test case:
-------------------------
```
make run_short
```

For large sized test case:
--------------------------
```
make run_long
```

Output
======
Look at the following files in side-by-side comparison:
```
fcfs.outfile
stcf_parallel.outfile
```

Inputs for Reference
====================
To view the inputs, look at the following directories and files

For run_short:
```
short_test/resources.input
short_test/jobs.input
```

For run_long:
```
long_test/resources.input
long_test/jobs.input
```

Design
======
Assumptions
-----------
0. Jobs and Resources inputted come in 1 by 1 at each time tick
0. Only one resource offer is allowed per time tick to the Framework. It can accept or reject.
0. Multiple Jobs can run using one Node's resource, but one job cannot be spread across multiple Nodes.
0. If Multiple Jobs are using one Node's resources, as they finish the resources will be released (no need to wait for all to finish)
0. If Resource is accepted by the framework, it must use the entire resource. There CAN be interal fragmentation.
0. If a Node is repeated in the Resource stream, the resources can be bundled together to make for a larger offering [i.e. (1, 10) + (1,8) = (1,18)]
0. There is no job that uses more resources than the largest consolidated Node resource. You cannot get blocked on a job.
0. First Come First Serve has a one-to-one relation for Job to Resource

Mesos based resource management
-------------------------------

To keep the main loop clean, I created a few objects:

Logic driving Objects:
---------------------
0. Master: Consolidates currently avaiable resources. Runs main loop.
0. Slaves (1 object): Running job management. Abstracted (resource,list<Job>) bundles that will "run" task for a period of time and release resources
0. Framework: Decides based on FCFS or STCF-Parallel algorithms to accept resources, and decide which task(s) to run on that resource

Abstractions:
-------------
0. Job: (# Resources, # Duration)
0. Resource: (# ID, # Resources)
0. Bundle (Resource, List of Jobs)
0. HashList: Mechanism for O(1) search for Node id to consolidate resources for that particular node (used by Master)

Scheduling Algorithms
=====================
First Come First Serve 
----------------------
Used as a benchmark for our test. Master will offer the Framework a resource. If the Job at the front of the fcfs_q needs less than or equal to the resource offering, it will accept the resource and send that task to be run on the Node.

Shortest Time To Completion with Job Parallelization
----------------------------------------------------

The enhanced scheduling algorithm.

It uses a priority_queue (O(log(n)) insert) to sort jobs based on time duration.  On top of that, if a resource offering is large enough to fit multiple Jobs, it will pack in as many jobs as possible, while still maintaining STCF ordering.
The idea is that if we finish smaller jobs parallely, we can complete tasks with faster response time and throughput than the FCFS algorithm.

If you look at the run_long outfile and search for "Job start", you can see that many jobs are started at the same time tick, and finish based on their job duration.
Even if there are ample resources available for both scheduling algorithms, the parallelization can be N-ticks faster (to complete all N tasks) if there are N jobs to be run.

```
0  1  2  3  4  5  6 
[Job 5 ticks]
   [Job 5 ticks]            FCFS Best Case
      [Job 5 ticks]

[Job 5 ticks]
[Job 5 ticks]               STCF-Parallel Best Case
[Job 5 ticks] 
```

Metrics and Results
-------------------
Short Input Data:
```
Job Count: 5
Resource Nodes: 2

Result:

FCFS: 73 time ticks
STCF-Parallel: 55 time ticks
```

Long Input Data:
```
Job Count: 3000
Resource Nodes: 10

Result:

FCFS: 22583 time ticks
STCF-Parallel: 4184  time ticks
```

We can see that STCF-Parallel performs a number of times better than FCFS.
This is especially the case with limited compute resources and many tasks.

The outfiles show that STCF does parallely start Jobs on a single resource, and finishes using them quickly.

Final Thoughts
==============
Over all, STCF-Parallel performed very well compared to FCFS.
This is due to allowing for parallelization of tasks for a particular resource, as well as utilizing those resources as quickly as possible.
Since a bundle of jobs consists of roughly similarly timed jobs, the entire resource can be given back from the Slaves to the Master as one large chunck (or within a few time ticks of one another to let the Master consolidate it before offering it to the Framework).

Possible Issues:
----------------
Long jobs will get starved if there are many short jobs.
Didn't implement the best way to maximize parallelism. Just got the smallest timed jobs and tried to fit as many as possible for a particular resource.
A better approach would be to minimize resource-size as well as time-to-complete for a job (or strike a balance). This way you can have many more Jobs running in parallel on a single resource. AND they will release the resources in roughly the same time if the Jobs for that resource bundle have roughly the same time to completion.

Solutions to make this better: 
------------------------------
0. Maximize parallelism for similarly timed jobs. This would allow for many Jobs to be run on 1 Node. If they finish at roughly the same time then the Node can give back resources as one large chunk, therefore preventing breaking up resources. While Master does bundle back resources from the same Node, it could be possible that another job needs that resource, and would cause fragmentation of a Node's resources. 
0. Allow for fair scheduling to allow some longer jobs to run alongside short jobs


