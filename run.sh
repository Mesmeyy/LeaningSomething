/soft/spark-2.4.3-bin-hadoop2.7/bin/spark-submit \
	--deploy-mode client \
	--driver-memory 30G \
	--total-executor-cores 1 \
	--executor-cores 1 \
	--executor-memory 30G \
	--master local \
	--packages org.apache.spark:spark-sql-kafka-0-10_2.11:2.4.3 \
	--class app1_dag  /data/006zzy/HUASHIDA/app1-dag/target/app1-dag-1.0.jar input14

