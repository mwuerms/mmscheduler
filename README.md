# mmscheduler
Martins Micro scheduler in standard C, for micro processors

## structure

+ `scheduler` main scheduler, add processes, run, send events
  + `events` managing event queue as well as timed events (put in event queue later)
+ uses external components from mmlib
  + `fifo` 

+ coop scheduler for mcu
+ based on agnar-os https://github.com/mwuerms/agnar-os
+ make use of mmlib https://github.com/mwuerms/mmlib
