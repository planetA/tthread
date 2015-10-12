# Inspector

- Combines tthread and [Intel Pt](https://software.intel.com/en-us/blogs/2013/09/18/processor-tracing) to build
  a concurrent data provenance graph

## Preqrequisites

- build libtthread (see https://github.com/Mic92/tthread#building-tthread).
- hardware which is capable of Intel PT (processors since the Broadwell
  architecture)
- A linux kernel with the Intel PT PMU driver for perf (was merged in v3.16-rc4) and `perf_event` cgroup:
  - check if `/sys/bus/event\_source/devices/intel\_pt` exists
  - check if kernel option is activated `zgrep CONFIG_CGROUP_PERF /proc/config.gz`
  - check if perf_event cgroup is mounted in `/sys/fs/cgroup/perf_event`
- Perf tool with support for Intel PT (merged release candiates of v4.3):
  - check with `perf list | grep intel\_pt`
- python3 to run the script

## Usage

To trace system wide and manage cgroups inspector needs **root privileges**

```
usage: inspector [-h] [--libtthread-path LIBTTHREAD_PATH]
                 [--perf-command PERF_COMMAND] [--perf-log PERF_LOG]
                 command [arguments [arguments ...]]

Run program with tthread and intel PT

positional arguments:
  command               command to execute with
  arguments             arguments passed to command

optional arguments:
  -h, --help            show this help message and exit
  --libtthread-path LIBTTHREAD_PATH
                        path to libtthread.so (default: ../../libtthread.so -
                        relative to script path)
  --perf-command PERF_COMMAND
                        Path to perf tool
  --perf-log PERF_LOG   File name to write log
```
