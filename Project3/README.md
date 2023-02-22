# Project 3
**Memory and Storage Performance Profiling**

**Background:**

The goal of this project is to develop a deeper understanding of modern memory and storage applications. To explore this topic, we will be using Intel's [Memory Latency Checker](https://www.intel.com/content/www/us/en/developer/articles/tool/intelr-memory-latency-checker.html) for Cache and Memory, and [Flexible IO Tester](https://github.com/axboe/fio) for Storage. Since our objecitve is to denote and analyze the performance of memory and storage, we will identify how data access throughput affects latency.

The study on this effect is called Queuing Theory, defined by Merriam-Webster as "The mathematical and statiscal theory of queues and waiting lines." Queuing Theory's most applicable definition of this relationship (assuming a Poisson distribution) states that the average number of requests in a queue (L<sub>queue</sub>) is directly linked to server utilization (μ). The relationship is as follows:

$$ L_{queue} = { μ^2 \over  1 - μ } $$

Our experiments will change the number of requests in the queue and measure the latency. Latency is proportionate to utilization. Therefore, if we consider our storage devices to be psuedo-servers, we should be able to derive the referenced equation with a certain leading factor.

**Experiment Environment & Settings:**

**Results:**

**Analysis and Conclusions:**
