#!/bin/bash
for i in {0..9..1}
do
  cat "Throughput_run_$i.plt" >> plot_thr
done
