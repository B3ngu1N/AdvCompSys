# Project 3: Memory and Storage Performance Profiling

## Background:

The goal of this project is to develop a deeper understanding of modern memory and storage applications. To explore this topic, we will be using Intel's [Memory Latency Checker](https://www.intel.com/content/www/us/en/developer/articles/tool/intelr-memory-latency-checker.html) for Cache and Memory, and [Flexible IO Tester](https://github.com/axboe/fio) for Storage. Since our objecitve is to denote and analyze the performance of memory and storage, we will identify how data access throughput affects latency.

The study on this effect is called Queuing Theory, defined by Merriam-Webster as "The mathematical and statiscal theory of queues and waiting lines." Queuing Theory's most applicable definition of this relationship (assuming a Poisson distribution) states that the average number of requests in a queue (L<sub>queue</sub>) is directly linked to server utilization (μ). The relationship is as follows:

$$ L_{queue} = { μ^2 \over  1 - μ } $$

Our experiments will change the number of requests in the queue and measure the latency. Latency is proportionate to utilization. Therefore, if we consider our storage devices to be psuedo-servers, we should be able to derive the referenced equation with a certain leading factor.


## Experimental Environment:

**Test System:** Dell XPS-15 9520
* Intel i7-12700H @ 2.3GHz
  - L1: 1.2MB, L2: 11.5MB, L3: 24MB
* 16.0 GB RAM @ 4800 MHz

**FIO Test Environment**
* NVMe Device - Kioxia 512GB ([KXG70ZNV512G](https://www.harddrivebenchmark.net/hdd.php?hdd=NVMe%20KXG70ZNV512G%20NVMe%20KIOXIA%20512GB&id=29707))
* Sequential Reads - 3868 MB/s
* Sequential Writes - 2798 MB/s
* Random Seek Read Write (IOPS 32KQD20) - 2111 MB/s
* IOPS 4KQD1 - 86 MB/s

**Software Environment**
* WSL2 - Ubuntu 22.04 LTS

## Intel MLC Commands & Settings

### General commands used and why they were used:

`--loaded_latency` &rarr; Latencies at different bandwidth points.

`--peak_injection_bandwidth` &rarr; Peak injection memory bandwidth is measured (with all accesses to local memory) for requests with varying amounts of reads and writes (each core generating requests as fast as possible).

`--max_bandwidth` &rarr; Peak injection bandwidth tests are done multiple times and the max bandwidth possible is found.

`--c2c_latency` &rarr; Latency measurement of cache-to-cache transfers in the processor.

### Important Command Parameters

**Data Access Size:**

`-l` &rarr; Sets stride size in bytes (for everything but c2c_latency). Set to 64B and 256B in our testing.

**Read Only:**

`-R` &rarr; Sets to fully read only.

**Write Only:**

`-W6`  &rarr; Sets to fully write only (non-temporal writes).

**Mixed Read-Write:**

`-Wn` &rarr; Specify read to write ratio for the bandwidth generation thread in loaded_latency and max_bandwidth.

`-W2` &rarr; Read:Write = 2:1

`-W3` &rarr; Read:Write = 3:1

`-W5` &rarr; Read:Write = 1:1


## FIO Commands & Settings

A flash drive was used as a test device for random reads and writes. This was done due to the fact that our working environment was WSL2 (Ubuntu) within Windows and it does not recognize partitions on an internal drive (just sees all memory as Virtual Memory). As a result, using an external drive was the best workaround that prevents issues with overwriting OS information during random write performance analysis with FIO.

Analysis was done on a variety of read vs. write intensity ratios (read-only, write-only, multiple ratios of read vs. write) and data access sizes (4KB, 32KB, 128KB). Throughput vs. latency was also analyzed.

### Important Command Parameters:

`--rw` &rarr; Determines analysis type (randread or randwrite).

`--filename` &rarr; Determines device under test (SSD or external drive in our case).

`--bs` &rarr; Determines data access size/batch size.

**Random Reads:**

```shell
fio --name=[global] --ioengine=libaio –filename=/dev/<ssd_device_name_here> --iodepth=128 --rw=randread --bs=4k --direct=1 --numjobs=16 --runtime=10 --group_reporting
```

**Random Writes:**

[Useful FIO Guide Video](https://www.youtube.com/watch?v=RnqnogK5ceo&ab_channel=TechnicalBytes)


## Results:

## Analysis and Conclusions:
