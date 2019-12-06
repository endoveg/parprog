#!/usr/bin/bash
mkdir 2g_eo
cd 2g_eo
rm lab2g.{e,o}*
for N in {1..28}
do
	for r in {1..24}
	do
		echo "doing $r run and $N processors"
		qsub -v N=$N ../job_2g.sh
		sleep 40 
	done	
done
cat lab2g.o* >> measurements.txt
rm lab2g.{e,o}*
