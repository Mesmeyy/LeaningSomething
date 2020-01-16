#!/bin/bash
export HADOOP_HOME=/soft/hadoop-2.9.1
export PATH=$HADOOP_HOME/bin:$HADOOP_HOME/sbin:$PATH
export LD_LIBRARY_PATH=$JAVA_HOME/jre/lib/amd64/server:/soft/hadoop-2.9.1/lib/native/
export CLASSPATH=$($HADOOP_HOME/bin/hadoop classpath --glob)
CLASSPATH=$CLASSPATH:`hadoop classpath` /data/006zzy/eclipse-workspace/ReadHdfs/Debug/ReadHdfs
