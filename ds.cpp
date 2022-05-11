#include "ds.hpp"
#include <vector>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <cstring>
#include <algorithm>
#include <assert.h>
#include <thread>
#include"mybalance.hpp"
using namespace std;

string local_ip;
string work_dir;
int priotimes1 = 63;//stream pri
int priotimes2 = 32;//batch pri

std::mutex taskmutex;
queue<ExecutorRequestIDMessage> eriqueue;
namespace mesos {
namespace internal {
namespace master {


struct BalanceScheduler balsche;
struct Apportion apport;
MasterProcess::MasterProcess() :
		ProcessBase("master") {
	tasklog = new TaskLog();
}

void MasterProcess::initialize() {
	LOG(INFO) << "Distributed Scheduler started on " << string(self());

	master = self();
	link(negotiator_pid);
	Readetchosts();
	bool ret = getHostInfo(hostname, ip);

	string schedulerid = "DistributedScheduler" + ip;
	info.set_id(schedulerid);
	info.set_ip(2105834626);

	info.set_port(self().address.port);
	if (true == ret) {
		LOG(INFO) << "hostname: " << hostname;
		LOG(INFO) << "local_ip: " << ip;
	}
	lastest_machine = "";
	balsche.SchedulerMethod = SchedulerMethod;
	balsche.work_dir = work_dir;
	thisscheduler_machine=hostname;
	LOG(INFO)<<"balsche.thisscheduler_machine" << thisscheduler_machine<<endl;
	LOG(INFO)<<"balsche.work_dir:" << balsche.work_dir<<endl;
	balsche.getmachinenames();
	install<mesos::scheduler::Call>(&MasterProcess::receive);
	install<QueueOfferMessage>(&MasterProcess::receiveQueueOfferMessage);
	install<RespondMatchResultMessage>(
			&MasterProcess::receiveRespondMatchResultMessage);
	install<UpdateTaskStateMessage>(&MasterProcess::receiveTaskStateMessage);
	install<QueueFreeResourceRequest>(&MasterProcess::receiveresofqueue);
	install<DeliverExecutorMessage>(&MasterProcess::DeliverExecutor);
	sendRequestQueueResourceMessage();
}

UPID MasterProcess::makepid(const char *address, uint16_t port, string str) {
	struct in_addr s;
	inet_pton(AF_INET, address, (void*) &s);
	//string str = "Negotiator";
	net::IP myip(s);
	//uint16_t port = 5555;
	UPID pid(str.c_str(), myip, port);
	return pid;
}

void MasterProcess::receive(const UPID &from, scheduler::Call &&call) {
	if (call.type() == scheduler::Call::SUBSCRIBE) {
		subscribe(from, call.subscribe());
		return;
	}

	FrameworkID frameworkId = call.framework_id();
	Framework *framework = getFramework(frameworkId);
	if(framework == NULL) return ;
	switch (call.type()) {
	case scheduler::Call::SUBSCRIBE:
		// SUBSCRIBE call should have been handled above.
		LOG(FATAL) << "Unexpected 'SUBSCRIBE' call";

	case scheduler::Call::TEARDOWN:
		teardown(framework);
		break;

	case scheduler::Call::ACCEPT:
		accept(framework, std::move(*call.mutable_accept()));
		break;

	case scheduler::Call::DECLINE:
		decline(framework, std::move(*call.mutable_decline()));
		break;

	case scheduler::Call::ACCEPT_INVERSE_OFFERS:
		//acceptInverseOffers(framework, call.accept_inverse_offers());
		break;

	case scheduler::Call::DECLINE_INVERSE_OFFERS:
		//declineInverseOffers(framework, call.decline_inverse_offers());
		break;

	case scheduler::Call::REVIVE:
		//revive(framework, call.revive());
		break;

	case scheduler::Call::KILL:
		//kill(framework, call.kill());
		break;

	case scheduler::Call::SHUTDOWN:
		//shutdown(framework, call.shutdown());
		break;

	case scheduler::Call::ACKNOWLEDGE: {
		acknowledge(framework, std::move(*call.mutable_acknowledge()));
		break;
	}

	case scheduler::Call::ACKNOWLEDGE_OPERATION_STATUS: {
//	      drop(
//	          from,
//	          call,
//	          "'ACKNOWLEDGE_OPERATION_STATUS' is not supported by the v0 API");
		break;
	}

	case scheduler::Call::RECONCILE:
		//reconcile(framework, std::move(*call.mutable_reconcile()));
		break;

	case scheduler::Call::RECONCILE_OPERATIONS:
//	      drop(
//	          from,
//	          call,
//	          "'RECONCILE_OPERATIONS' is not supported by the v0 API");
		break;

	case scheduler::Call::MESSAGE:
		//message(framework, std::move(*call.mutable_message()));
		break;

	case scheduler::Call::REQUEST:
		//request(framework, call.request());
		break;

	case scheduler::Call::SUPPRESS:
		//suppress(framework, call.suppress());
		break;

	case scheduler::Call::UNKNOWN:
		LOG(WARNING) << "'UNKNOWN' call";
		break;
	}
}

unsigned long MasterProcess::getCurrenttime() {
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
FrameworkID MasterProcess::newFrameworkId() {
	std::ostringstream out;
	time_t now = time(0);
	out << info.id() << "-" << std::setw(4) << std::setfill('0')
			<< nextFrameworkId++ << "-" << now;
	FrameworkID frameworkId;
	frameworkId.set_value(out.str());
	return frameworkId;
}

void MasterProcess::subscribe(const process::UPID &from,
		const scheduler::Call::Subscribe &subscribe) {

	if (loglevel > LEVEL0) {
		LOG(INFO) << "************************************";
		LOG(INFO) << "receive RegisterFrameworkMessage...";
		LOG(INFO) << "Framework-name:" << subscribe.framework_info().name();
	}
	int is_flag = 0;
	foreachvalue(Framework* framework,frameworks) {
	if(framework->pid==from) {
		LOG(INFO)<<"create FrameworkRegisteredMessage...";
		FrameworkRegisteredMessage message;
		message.mutable_framework_id()->set_value(framework->id.value());
		message.mutable_master_info()->MergeFrom(info);
		reply(message);
		LOG(INFO)<<"send FrameworkRegisteredMessage...";
		LOG(INFO)<<"************************************";
		return;
	}
}

	time_t t = time(0);
	char now[64];
	strftime(now, sizeof(now), "%Y-%m-%d %H-%M-%S", localtime(&t));
	if (loglevel > LEVEL0) {
		LOG(INFO) << "register-time:" << now;
		LOG(INFO) << "Framework-capability-size:"
				<< subscribe.framework_info().capabilities().size();
	}
	Framework *framework;

	if (subscribe.framework_info().capabilities().size() > 0) {
		foreach(const FrameworkInfo::Capability& capability,subscribe.framework_info().capabilities()){
		//	LOG(INFO)<<"Framework-capability-type:"<<capability.Type_Name(capability.type());
		if(strcmp(capability.Type_Name(capability.type()).c_str(),"GPU_RESOURCES")==0) {
			framework = new Framework(subscribe.framework_info(), newFrameworkId(), from,"GPU");
			break;
		}
	}
}
else if(!strcmp(subscribe.framework_info().name().c_str(),"AppResourceCheck"))
{
	string appname,hostname,frameid,executorid;
	Framework* tempframe=NULL;

	for (int j = 0; j < subscribe.framework_info().labels().labels_size();j++) {
		string key = subscribe.framework_info().labels().labels(j).key();
		string value =subscribe.framework_info().labels().labels(j).value();
		//LOG(INFO) <<key<<":"<<value;
		if (!strcmp(key.c_str(), "appname"))
		{
			appname=value;
		}
		else if(!strcmp(key.c_str(), "hostname"))
		{
			hostname=value;
		}
		else if(!strcmp(key.c_str(), "executorid"))
				{
					executorid=value;
				}
	}
	foreachvalue(Framework* frame,frameworks)
	{
		if(!strcmp(frame->info.name().c_str(),appname.c_str()))
		{
			tempframe=frame;
			break;
		}
	}
	framework = new Framework(subscribe.framework_info(), newFrameworkId(), from,"CPU");
	QueueOfRequest qor;
	maps maps;
	frameworks[framework->id.value()] = framework;
	vector<string> ip;

	//LOG(INFO)<< ip[0];
	if(tempframe && tempframe->usingoffer.size())
	{
		foreachvalue(struct MacAndBatch mab,tempframe->usingoffer)
		{
			//strcmp(mab.hostname,hostname.c_str())==0
			cout << "asdadsa:" << mab.batch<< " "<< mab.hostname<< endl;
			if(mab.execid==executorid)
			{
				is_flag=1;
				ip=getIpByName(mab.hostname);
				para map;
				map.set_key(mab.batch);
				map.set_value(mab.batch);
				maps.add_mapp()->MergeFrom(map);
			}
		}
		if(is_flag)
		{
			qor.mutable_batch()->CopyFrom(maps);
			qor.set_fid(framework->id.value());
			UPID mqpid = makepid(ip[0].c_str(),resouckerport,"ResourceTracker");
			LOG(INFO)<< mqpid;
			send(mqpid, qor);

		}
	}
}
else {
	framework = new Framework(subscribe.framework_info(), newFrameworkId(), from,"CPU");
	framework->register_time=now;
}


	if (subscribe.framework_info().has_labels()) {
		foreach(const Label& label,subscribe.framework_info().labels().labels()){
	        //LOG(INFO)<< label.key() << " "<< label.value();
		if(strcmp(label.key().c_str(),"spark.mesos.gpus.mem")==0) { //pergpu
			framework->pergpumem=atoi(label.value().c_str());
		}
		else if(strcmp(label.key().c_str(),"spark.mesos.gpus.cores")==0) { //percore
			framework->pergpucores=atoi(label.value().c_str());
		}
		else if(strcmp(label.key().c_str(),"spark.mesos.cores.max")==0) { //percore
			framework->required_cpus_max=atoi(label.value().c_str());
		}
		else if(strcmp(label.key().c_str(),"spark.mesos.mem.max")==0) { //percore
			framework->required_mem_max=atoi(label.value().c_str());
		}
		else if(strcmp(label.key().c_str(),"spark.mesos.gpus.max")==0) { //percore
			framework->required_gpus_max=atoi(label.value().c_str());
		}else if(strcmp(label.key().c_str(),"spark.executor.cores")==0){//executor need cores;
			framework->executor_need_cpus=atoi(label.value().c_str());
		}else if(strcmp(label.key().c_str(),"streamorbatch")==0){
			LOG(INFO)<<"get streamorbatch"<<endl;
			framework->attribute=atoi(label.value().c_str());
		}
		}
	}
	//This is for bgraph scheduler
	if(framework->info.name()=="BigDataBench WordCount"){
		LOG(INFO)<<"Thisframeworkis:"<<framework->info.name()<<endl;
		framework->datasize=5000;
		framework->localmachines.push_back("master00");
		framework->localmachines.push_back("slave01");
	}else if(framework->info.name()=="Rodina Kmeans"){
		LOG(INFO)<<"Thisframeworkis:"<<framework->info.name()<<endl;
		framework->datasize=2000;
		framework->localmachines.push_back("slave05");
	}else if(framework->info.name()=="Prime"){
		framework->datasize=1000;
		framework->localmachines.push_back("slave07");
	}else{
		framework->datasize=1000;
		framework->localmachines.push_back("master00");
	}

	LOG(INFO)<<"framework->attribute:"<<framework->attribute<<endl;
	LOG(INFO)<<"fid:"<<framework->id.value()<<",datasize:"<<framework->datasize<<endl;
	for(auto c:framework->localmachines){
		LOG(INFO)<<"fid:"<<framework->id.value()<<",localmachine:"<<c<<endl;
	}

	if (priotimes1 == 32) {
		priotimes1 = 63;
	}
	if(priotimes2==0){
		priotimes2=32;
	}
	if(framework->attribute == 1){
		framework->queueprio = priotimes1;
		priotimes1--;
	}else{
		framework->queueprio = priotimes2;
		priotimes2--;
	}
	LOG(INFO)<<"frameworkid:"<<framework->id.value()<<" priotimes:"<<framework->queueprio<<endl;

//assign value to framework's required_cpus_max,required_gpus_max,required_mem_max
//and total_tasks_num

	unsigned long subtime = getCurrenttime();
	framework->register_time = std::to_string(subtime);
	framework->firstregister = subtime;
	if (loglevel > LEVEL0) {
		LOG(INFO) << "Framework-type:" << framework->type;
		//	LOG(INFO) << "Framework-user:" << framework->info.user();
		LOG(INFO) << "Framework-name:" << framework->info.name();
		LOG(INFO) << "FrameworkID:" << framework->id.value();
		//	LOG(INFO) << "Framework-active:" << framework->active;
		//	LOG(INFO) << "failover_timeout:" << framework->info.failover_timeout();
		LOG(INFO) << "Framework-queueprio:" << framework->queueprio;
		LOG(INFO) << "Framework-pergpumem:" << framework->pergpumem;
		LOG(INFO) << "Framework-pergpucores:" << framework->pergpucores;
		LOG(INFO) << "Framework-percpucores:"
				<< framework->required_cpus_max;
		LOG(INFO) << "Framework-maxmem:" << framework->required_mem_max;
		LOG(INFO) << "Framework-maxgpus:" << framework->required_gpus_max;
		LOG(INFO) << "Framework-maxcpus:" << framework->required_cpus_max;
	}

	if (!strcmp(subscribe.framework_info().name().c_str(),
					"AppResourceCheck")) {
		FrameworkRegisteredMessage message;
		message.mutable_framework_id()->MergeFrom(framework->id);
		message.mutable_master_info()->MergeFrom(info);
		link(framework->pid);
		send(framework->pid, message);
		if(is_flag == 0)
		   sendnoresource(1,framework);
		return;
	}
	addFramework(framework);

}

void MasterProcess::addFramework(Framework *framework) {
	CHECK(frameworks.count(framework->id.value()) == 0);
	frameworks[framework->id.value()] = framework;


	if (loglevel > LEVEL0) {
		LOG(INFO)<<"add framework now frameworksize:"<<frameworks.size()<<endl;
		LOG(INFO) << "framework->pid:" << framework->pid;
		LOG(INFO) << "create FrameworkRegisteredMessage...";
	}
	FrameworkRegisteredMessage message;
	message.mutable_framework_id()->MergeFrom(framework->id);
	message.mutable_master_info()->MergeFrom(info);
	link(framework->pid);
	send(framework->pid, message);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "send FrameworkRegisteredMessage...";
		LOG(INFO) << "************************************";
	}
	if(!apport.Framework_Enough_Resource(framework)){
		LOG(INFO)<<"framework resource is not enough"<<endl;
		if(framework->type == "CPU"){
			LOG(INFO)<<"This framework is cpu framework."<<endl;
			apport.active_cpu_frameworks.insert(framework->id.value());
		}else if(framework->type == "GPU" ){
			LOG(INFO)<<"This framework is gpu framework."<<endl;
			apport.active_gpu_frameworks.insert(framework->id.value());
		}else{
			LOG(INFO)<<"framework type ERROR.."<<endl;
		}
	}
	sendForwardFrameworkMessage(framework);
}

void MasterProcess::sendForwardFrameworkMessage(Framework *framework) {
	if (loglevel > LEVEL0) {
		LOG(INFO) << "************************************";
		LOG(INFO) << "create ForwardFrameworkMessage message...";
	}
	ForwardFrameworkMessage message;
	message.mutable_framework_id()->MergeFrom(framework->id);
	message.mutable_framework_info()->MergeFrom(framework->info);
	message.set_total_tasks_num(framework->total_tasks_num);
	message.set_register_time(framework->register_time);
	if(framework->required_cpus_max){
		para apara;
		apara.set_key("spark.mesos.cores.max");
		apara.set_value(std::to_string(framework->required_cpus_max));
		message.add_framework_labels()->MergeFrom(apara);
	}
	if(framework->required_mem_max){
		para apara;
		apara.set_key("spark.mesos.mem.max");
		apara.set_value(std::to_string(framework->required_mem_max));
		message.add_framework_labels()->MergeFrom(apara);
	}
	if(framework->required_gpus_max){
		para apara;
		apara.set_key("spark.mesos.gpus.max");
		apara.set_value(std::to_string(framework->required_gpus_max));
		message.add_framework_labels()->MergeFrom(apara);
	}
	link(negotiator_pid);
	send(negotiator_pid, message);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "send ForwardFrameworkMessage message...";
		LOG(INFO) << "************************************";
	}
}

void MasterProcess::sendRequestQueueResourceMessage() {
//	if (!frameworks.contains(framework->id.value()))
//		return;
	if (loglevel > LEVEL0) {
		LOG(INFO) << "************************************";
		LOG(INFO) << "create RequestQueueResource message... to Negotiator... ";
	}
	RequestQueueResourceMessage message;
	message.set_requestforoffer("RequestQueueResourceMessage");
	link(negotiator_pid);
	send(negotiator_pid, message);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "send RequestQueueResource message...";
		LOG(INFO) << "************************************";
	}
	delay(Seconds(atoi(this->request_delay.c_str())), self(),
			&MasterProcess::sendRequestQueueResourceMessage);
}

Framework* MasterProcess::getFramework(const FrameworkID &frameworkId) {
	if (frameworks.contains(frameworkId.value())) {
		return frameworks[frameworkId.value()];
	} else {
		return NULL;
	}
}

bool MasterProcess::compareBatch(QueueOffer queueoffer, Framework *frame,
		string offid) {
	OfferID oid;
	oid.set_value(offid);
	foreach(const Resource& resource,frame->getOffer(oid)->resources()) {
	if(strcmp(resource.name().c_str(),"cpus")==0) {
		if(queueoffer.cpu_num() <resource.scalar().value())
		return false;
	}
	if(strcmp(resource.name().c_str(),"gpus")==0) {
		if(queueoffer.gpu_num() <resource.scalar().value())
		return false;

	}
	if(strcmp(resource.name().c_str(),"mem")==0) {
		if(queueoffer.mem_num() <resource.scalar().value())
		return false;
	}
}
	return true;
}
void MasterProcess::receiveQueueOfferMessage(const process::UPID &from,
		QueueOfferMessage &&message) {
	if (loglevel > LEVEL0) {
		LOG(INFO) << "*************************************";
		LOG(INFO)
				<< "receive QueueOfferMessage from Negotiator";
	}
	if (message.queueoffers().size() == 0) {
		if (loglevel > LEVEL0) {
			LOG(INFO)<<"QueueOfferMessage have non_offer.."<<endl ;
			LOG(INFO)<<"*************************************"<<endl ;
		}
		return;
	}
	if(!frameworks.size()) {
		if (loglevel > LEVEL0) {
			LOG(INFO)<<"No frameworks to Scheduler.."<<endl ;
			LOG(INFO)<<"*************************************"<<endl ;
		}
		return ;
	};
	LOG(INFO)<<"isfair:"<<isfair<<endl;
	if(isfair){
		LOG(INFO)<<"fair-scheduler frameworksize:"<<frameworks.size()<<endl;
		if(frameworks.size()!=6) return;
		else{
			LOG(INFO)<<"start test fair scheduler..."<<endl;
		}
	}
	LOG(INFO)<<"START Balance"<<endl;
	//insert balance algorithm
	vector<QueueOffer> cpuoffers;
	vector<QueueOffer> gpuoffers;
	SchedulerPolicy(message,cpuoffers,gpuoffers);
	if(bgorfair=="BG"){
		LOG(INFO)<<"choice BG scheduler"<<endl;
		apport.SchedulerOffer_bgraph(frameworks,cpuoffers,gpuoffers);
	}else{
		LOG(INFO)<<"choice fair scheduler"<<endl;
		apport.SchedulerOffer_fair(frameworks,cpuoffers,gpuoffers);
	}

	dofailtask();
	SendOffer();
}
void MasterProcess::SchedulerPolicy(QueueOfferMessage &message,vector<QueueOffer>& cpuoffers,vector<QueueOffer>& gpuoffers){
	LOG(INFO)<<"QueueOfferMessage offer-size:"<< message.queueoffers().size()<<endl;
	balsche.Balance(message,cpuoffers,gpuoffers);
}
void  MasterProcess::dofailtask(){
	LOG(INFO)<<"restart_task_size:"<<endl;
	for(int i = 0; i < can_restart_framework_executor.size();i++){
		struct FailedTaskInfo  feo = can_restart_framework_executor[i];
		string fid = feo.frameworkId;
		string offerId = feo.offerId;
		string slaveid = feo.slaveid;
		ExecutorInfo execinfo = feo.execinfo;
		Framework *frame =frameworks[fid];
		OfferID oid;
		oid.set_value(offerId);
		LOG(INFO)<<"restart_planuse."<<endl;
		LOG(INFO)<<"framework:"<<fid<< " execinfo:"<<execinfo.executor_id().value()<<"has remove from failtaskset."<<endl;
		sendPlanUseResourceMessage(frame->getOffer(oid),execinfo,frame);
//		for(int j = 0;j <frame->FailTaskSet.size();j++){
//			if((fid == frame->FailTaskSet[j].frameworkId ) && (execinfo.executor_id().value() == frame->FailTaskSet[j].execinfo.executor_id().value())){
//				frame->FailTaskSet.erase(frame->FailTaskSet.begin()+j);
//			}
//		}
		//can_restart_framework_executor.erase(can_restart_framework_executor.begin()+i);
	}
}
//void MasterProcess::Balance(QueueOfferMessage &message, vector<QueueOffer> &GPUoffers,vector<QueueOffer>& CPUoffers){
//	GPUoffers.clear();
//	CPUoffers.clear();
//	int msize = machines.size();
//	int offerssize = message.queueoffers().size();
//	//LOG(INFO) << "There are "<< offerssize << " offers to balance..."<<endl;
//	if(SchedulerMethod == "RR_Scheduler"){
//		for(int i = 0;i < machines.size();i++){
//			queue<QueueOffer> que;
//			foreach(QueueOffer offer,message.queueoffers()){
//				if(machines[offer.machine_name()] == i){
//					que.push(offer);
//				}
//			}
//			std::array<QueueOffer,32> arr;
//			int arr_index = 0;
//			queue<QueueOffer> que_end;
//			while(!que.empty()){
//				QueueOffer temp_q = que.front();
//				que.pop();
//				if(temp_q.gpu_num() != 0 || arr_index == 0) arr[arr_index++] = temp_q;
//				else{
//					for(int k = arr_index-1;k >= 0;k--)
//						arr[k+1] = arr[k];
//					arr[0] = temp_q;
//					arr_index++;
//				}
//			}
//			for(int i = 0;i < arr_index;i++){
////				LOG(INFO)<<"que_endname:"<< arr[i].queue_name()<<endl;
//				que_end.push(arr[i]);
//			}
//			machines_offers[i] =que_end;
//		}
//		int index = machines[lastest_machine];
//		LOG(INFO)<<"RR_Scheduler,start machine:"<<lastest_machine<<endl;
//		bool flag_empty = true;
//		int offer_temp_size = 0;
//		while(1){
////			LOG(INFO)<<"index = " << index << endl;
////			LOG(INFO)<< "offer_temp_size = " << offer_temp_size<<endl;
////			LOG(INFO)<<"offerssize = " << offerssize<<endl;
//			if((flag_empty == true) && (offer_temp_size == offerssize)) break;
//			queue<QueueOffer>& qu = machines_offers[index];
//			if(qu.empty()){
////				LOG(INFO)<<"the queue QueueOffer is empty"<<endl;
//				flag_empty = true;
//				index = (index+1) % machines.size();
//			}else{
////				LOG(INFO)<<"the queue QueueOffer is not empty"<<endl;
//				flag_empty = false;
//				offer_temp_size ++;
//				QueueOffer qo = qu.front();
//				qu.pop();
////				LOG(INFO) << "offer gpu num:" << qo.gpu_num()<<endl;
////				LOG(INFO)<<"RR_Scheduler due offername:"<< qo.queue_name()<<endl;
//				if(qo.gpu_num() == 0){
//					CPUoffers.push_back(qo);
//				}else{
//					GPUoffers.push_back(qo);
//				}
//				index = (index+1) % machines.size();
//			}
//		}
//	}else if(SchedulerMethod == "LB_Scheduler"){
//		for(int i = 0;i < machines.size();i++){
//			queue<QueueOffer> que;
//			foreach(QueueOffer offer,message.queueoffers()){
//				if(machines[offer.machine_name()] == i){
//					que.push(offer);
//					machines_capacity[i] += offer.can_allocate_num();
//					machines_capacity[i] += offer.queueslot();
//				}
//			}
//			machines_offers[i] = que;
//		}
//		vector<int> machines_capacity_sort;
//		for(int i = 0;i < msize;i++){
//			machines_capacity_sort.push_back(i);
//		}
//
//		for(int i = 0;i < msize-1;i++){
//			for(int j = 0; j < msize-1-i;j++){
//				int slot1 = machines_capacity[j];
//				int slot2 = machines_capacity[j+1];
//				if(slot1 < slot2){
//					std::swap(machines_capacity_sort[j],machines_capacity_sort[j+1]);
//				}
//			}
//		}
//		for(int i = 0;i < msize;i++){
//			int mid = machines_capacity[i];
//			queue<QueueOffer> qu = machines_offers[mid];
//			while(!qu.empty()){
//				QueueOffer qo = qu.front();
//				qu.pop();
//				if(qo.gpu_num() == 0){
//					LOG(INFO)<<"This is cpuoffer..."<<endl;
//					LOG(INFO)<<"RR_Scheduler due offername:"<< qo.queue_name()<<endl;
//					CPUoffers.push_back(qo);
//				}else{
//					LOG(INFO)<<"This is gpuoffer..."<<endl;
//					LOG(INFO)<<"RR_Scheduler due offername:"<< qo.queue_name()<<endl;
//					GPUoffers.push_back(qo);
//				}
//			}
//		}
//	}else if(SchedulerMethod == "LD_Scheduler"){
//		for(int i = 0;i < machines.size();i++){
//			queue<QueueOffer> que;
//			foreach(QueueOffer offer,message.queueoffers()){
//				if(machines[offer.machine_name()] == i){
//					que.push(offer);
//				}
//			}
//			machines_offers[i] = que;
//		}
//		for(int i = 0;i < msize;i++){
//			queue<QueueOffer> qu = machines_offers[i];
//			while(!qu.empty()){
//				QueueOffer qo = qu.front();
//				qu.pop();
//				if(qo.gpu_num() == 0){
//					LOG(INFO)<<"This is cpuoffer..."<<endl;
//					CPUoffers.push_back(qo);
//				}else{
//					LOG(INFO)<<"This is gpuoffer..."<<endl;
//					GPUoffers.push_back(qo);
//				}
//			}
//		}
//		SortOffers(GPUoffers);
//		SortOffers(CPUoffers);
//	}
//	if (loglevel > LEVEL2) {
//		LOG(INFO)<< SchedulerMethod <<" Balance  process over..."<<endl;
//		LOG(INFO)<<"GPUoffers:"<<endl;
//		for(int i = 0;i < GPUoffers.size();i++){
//			LOG(INFO)<<"offername:"<<GPUoffers[i].queue_name()<<endl;
//		}
//		LOG(INFO)<<"CPUoffers:"<<endl;
//		for(int i = 0;i < CPUoffers.size();i++){
//			LOG(INFO)<<"offername:"<<CPUoffers[i].queue_name()<<endl;
//		}
//	}
//}
void MasterProcess::SendOffer(){
	foreachpair(string fid,Framework* framework,frameworks){
		if((framework->obtained_cpus_num >= framework->required_cpus_max)
				&& (framework->obtained_gpus_num  >= framework->required_gpus_max)
				&& (framework->obtained_mem_num  >= framework->required_mem_max)
				&& (framework->FailTaskSet.size() == 0) ) {
				//resource is enough & executors no fail
				if(framework->type == "CPU"){
					LOG(INFO)<<"framework is resource enough:"<<framework->id.value()<<endl;
					apport.active_cpu_frameworks.erase(framework->id.value());
				}
				if(framework->type == "GPU"){
					LOG(INFO)<<"framework is resource enough:"<<framework->id.value()<<endl;
					apport.active_gpu_frameworks.erase(framework->id.value());
				}
				continue;
		}
		//framework who need resource do follow:
		if(framework->firstreceiveoffer == 0)
			framework->firstreceiveoffer = getCurrenttime();
		if (loglevel > LEVEL0) {
			LOG(INFO) << framework->id.value() << "get queueoffers time:"
					<< framework->firstreceiveoffer;
		}
		ResourceOffersMessage offermessage;

		LOG(INFO)<<"This framework has "<< framework->now_iterator_offers.size()<<" offers to send"<<endl;
		while(!framework->now_iterator_offers.empty()){
//			LOG(INFO)<<"1"<<endl;
			QueueOffer t = framework->now_iterator_offers.front();
//			LOG(INFO)<<"2"<<endl;
			framework->now_iterator_offers.pop();
//			LOG(INFO)<<"Going to add offer:"<< t.queue_name()<<endl;
			addQueueOfferToMessage(framework, t, offermessage);
//			balsche.lastest_machine= t.queue_name();
//			LOG(INFO)<<"The lastest_machine:"<< balsche.lastest_machine <<endl;
		}
		LOG(INFO)<<"Send ResourceOffersMessage size:"<<offermessage.offers().size() << " to framework:"<< fid<<endl;
		send(framework->pid, offermessage);
	}
}
void MasterProcess::addQueueOfferToMessage(Framework *framework,
		const QueueOffer &queueoffer, ResourceOffersMessage &offermessage) {
//	LOG(INFO) << "*****add " << queueoffer.queue_name()
//			<< " to ResourceOffersMessage*******";
	Offer *offer = new Offer();

	OfferID oid;
	oid.set_value(
			queueoffer.queue_name() + "+" + info.id() + "-"
					+ stringify(nextOfferId++));
	offer->mutable_id()->MergeFrom(oid);
	offer->mutable_framework_id()->MergeFrom(framework->id);

	SlaveID sid;
	sid.set_value(queueoffer.batch_name() + "@" + queueoffer.machine_name());
	offer->mutable_slave_id()->MergeFrom(sid);
	offer->set_hostname(queueoffer.machine_name());

//	LOG(INFO)<<"add "<< queueoffer.batch_name() + "@" + queueoffer.machine_name();
	if (queueoffer.cpu_num() > 0) {
		Resource cpu;
		cpu.set_name("cpus");
		cpu.set_type(Value::SCALAR);
		cpu.mutable_scalar()->set_value(framework->executor_need_cpus);
		offer->add_resources()->MergeFrom(cpu);

		Attribute acpu;
		acpu.set_name("cpus");
		acpu.set_type(Value::SCALAR);
		acpu.mutable_scalar()->set_value(framework->executor_need_cpus);
		offer->add_attributes()->MergeFrom(acpu);
	}
	if (queueoffer.mem_num() > 0) {
		Resource mem;
		mem.set_name("mem");
		mem.set_type(Value::SCALAR);
		mem.mutable_scalar()->set_value(queueoffer.mem_num());
		offer->add_resources()->MergeFrom(mem);

		Attribute amem;
		amem.set_name("mem");
		amem.set_type(Value::SCALAR);
		amem.mutable_scalar()->set_value(queueoffer.mem_num());
		offer->add_attributes()->MergeFrom(amem);
	}
	if (queueoffer.gpu_num() > 0) {
		Resource gpu;
		gpu.set_name("gpus");
		gpu.set_type(Value::SCALAR);
		gpu.mutable_scalar()->set_value(queueoffer.gpu_num());
		offer->add_resources()->MergeFrom(gpu);

		Attribute agpu;
		agpu.set_name("gpus");
		agpu.set_type(Value::SCALAR);
		agpu.mutable_scalar()->set_value(queueoffer.gpu_num());
		offer->add_attributes()->MergeFrom(agpu);

		Resource gpumem;
		gpumem.set_name("gpumem");
		gpumem.set_type(Value::SCALAR);
		gpumem.mutable_scalar()->set_value(queueoffer.gpu_mems(0).gpumem()); //MB
		offer->add_resources()->MergeFrom(gpumem);

		Attribute agpumem;
		agpumem.set_name("gpumem");
		agpumem.set_type(Value::SCALAR);
		agpumem.mutable_scalar()->set_value(queueoffer.gpu_mems(0).gpumem()); //MB
		offer->add_attributes()->MergeFrom(agpumem);

		Resource gpucores;
		gpucores.set_name("gpucores");
		gpucores.set_type(Value::SCALAR);
		gpucores.mutable_scalar()->set_value(queueoffer.gpu_cores(0).gpucore());
		offer->add_resources()->MergeFrom(gpucores);

		Attribute agpucores;
		agpucores.set_name("gpucores");
		agpucores.set_type(Value::SCALAR);
		agpucores.mutable_scalar()->set_value(
				queueoffer.gpu_cores(0).gpucore());
		offer->add_attributes()->MergeFrom(agpucores);


		Resource gpuid;
		gpuid.set_name("gpuids");
		gpuid.set_type(Value::SET);
		for (int i = 0; i < queueoffer.gpu_ids().size(); i++) {
			string value = queueoffer.gpu_ids(i);
			LOG(INFO)<<"gpu id getttt:"<<value<<endl;
			gpuid.mutable_set()->add_item(value);
		}
		offer->add_resources()->MergeFrom(gpuid);
	}

	mesos::Value_Range range;
	range.set_begin(33000);
	range.set_end(34000);
	mesos::Value_Ranges ranges;
	ranges.add_range()->MergeFrom(range);

	Resource port;
	port.set_name("ports");
	port.set_type(Value::RANGES);
	port.mutable_ranges()->CopyFrom(ranges);
	offer->add_resources()->MergeFrom(port);

	foreachkey(const string& executorId,framework->executors) {
	ExecutorID executor_id;
	executor_id.set_value(executorId);
	offer->add_executor_ids()->MergeFrom(executor_id);
}

//add the offer to the message

	bool isused = false;
	if (!isused) {
		offermessage.add_offers()->MergeFrom(*offer);
		offermessage.add_pids(master);
		//add offer to framework's hashmap <offers>
		framework->addOffer(offer);
		int gpus = queueoffer.gpu_num();
		framework->addgpusperOffer(offer->id(), gpus);
	}
	//useless
//	for (int i = 0; i < framework->FailTaskSet.size(); i++) {
//		if (loglevel > LEVEL3) {
//			LOG(INFO) << "FailTaskSet.size()|:"
//					<< framework->FailTaskSet.size();
//			LOG(INFO) << framework->FailTaskSet[i].slaveid << " "
//					<< framework->FailTaskSet[i].offerId;
//		}
//		FrameworkID fid;
//		fid.set_value(framework->FailTaskSet[i].frameworkId);
//		Framework *frame = getFramework(fid);
//		// && strcmp(framework->id.value().c_str(),FailTaskSet[i].frameworkId.c_str())==0
//		if (strcmp(sid.value().c_str(),
//				framework->FailTaskSet[i].slaveid.c_str()) == 0) {
////			launchExecutor(slaveid,FailTaskSet[i].execinfo,framework);
//			if (loglevel > LEVEL0) {
//				LOG(INFO) << "READY to fail task for " + sid.value();
//			}
//			OfferID oid;
//			oid.set_value(framework->FailTaskSet[i].offerId);
//			sendPlanUseResourceMessage(framework->getOffer(oid),
//					framework->FailTaskSet[i].execinfo, framework);
//			framework->FailTaskSet.erase(framework->FailTaskSet.begin() + i);
//			isused = true;
//			break;
//		}
//	}

}

void MasterProcess::accept(Framework *framework,
		scheduler::Call::Accept &&accept) {

	if (loglevel > LEVEL0) {
		LOG(INFO)
				<< "************receive framework Call accept ************** for "
				<< framework->id.value();
		LOG(INFO) << "This accept call have " << accept.offer_ids().size()
				<< " offer:";
	}
	Offer *offer;
	foreach(const OfferID& offerId,accept.offer_ids()) {
	//LOG(INFO)<<offerId.value();
	offer=framework->getOffer(offerId);
	foreach(const Offer::Operation& operation,accept.operations()) {
//	LOG(INFO)<<"operation-type:"<<operation.Type_Name(operation.type());
//	LOG(INFO)<<"opersation-id:"<<operation.id().value();
		if(operation.type()==1) {
			if (loglevel > LEVEL0) {
				LOG(INFO)<<"this operation have "<<operation.launch().task_infos().size()<<" tasks:";
			}
			foreach(const TaskInfo& task,operation.launch().task_infos()) {
				sendPlanUseResourceMessage(offer,task,framework);
//			launchTestCExecutor(offer->slave_id().value(),offer->id().value(),
//					task.executor(), framework);
			}
		}
	}
}
}

void MasterProcess::sendPlanUseResourceMessage(Offer *offer,
		const ExecutorInfo &execInfo, Framework *framework) {
	if (loglevel > LEVEL0) {
		LOG(INFO) << "******create PlanUseResourceMessage*******";
	}
	PlanUseResourceMessage message;
	message.mutable_framawork_id()->MergeFrom(framework->id);
	message.mutable_executor_info()->MergeFrom(execInfo);
	message.set_offer_id(offer->id().value());
	message.mutable_slave_id()->set_value(offer->slave_id().value());
	//LOG(INFO)<<"offerid:"<<offer->
	link(negotiator_pid);
	send(negotiator_pid, message);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "******send PlanUseResourceMessage*******";
	}
	if (loglevel > LEVEL0) {
	  LOG(INFO) << "******create PlanUseResourceMessage*******";
	 }
}

void MasterProcess::sendPlanUseResourceMessage(Offer *offer,
		const TaskInfo &task, Framework *framework) {
	if (loglevel > LEVEL2) {
		LOG(INFO) << "****************************************";
		LOG(INFO) << "(1)task_name:" << task.name();
		LOG(INFO);
		LOG(INFO) << "(2)task_id:" << task.task_id().value();
		LOG(INFO);
		LOG(INFO) << "(3)task_slave_id:" << task.slave_id().value();
		LOG(INFO);
		LOG(INFO) << "(4)task_resources:";
		foreach(const Resource& resource,task.resources()) {
		LOG(INFO)<<"resource-name:"<<resource.name();
		LOG(INFO)<<"resource-type:"<<resource.type();
		LOG(INFO)<<"resource-scalar:"<<resource.scalar().value();
		LOG(INFO)<<"resource-ranges:";
		foreach(const Value::Range& range,resource.ranges().range()) {
			LOG(INFO)<<"range-begin:"<<range.begin();
			LOG(INFO)<<"range-end:"<<range.end();
		}
		LOG(INFO)<<"resource-set:";
		foreach(const string& item,resource.set().item()) {
			LOG(INFO)<<item<<";";
		}
		LOG(INFO)<<"resource-role:"<<resource.role();
	}
		LOG(INFO);

		LOG(INFO) << "(5)task_executor_info: ";
		LOG(INFO) << "executor-type:"
				<< task.executor().Type_Name(task.executor().type());
		LOG(INFO) << "executor-id:"
				<< task.executor().executor_id().value();
		LOG(INFO) << "executor-framework-id:"
				<< task.executor().framework_id().value();
		LOG(INFO) << "executor-commandinfo:";
		LOG(INFO) << "executor-commandinfo-uris:";
		foreach(const CommandInfo::URI& uri,task.executor().command().uris()) {
		LOG(INFO)<< "executor-commandinfo-uri-value:"<< uri.value();
		LOG(INFO)<< "executor-commandinfo-uri-executable:"<<uri.executable();
		LOG(INFO)<< "executor-commandinfo-uri-extract:"<< uri.extract();
		LOG(INFO)<< "executor-commandinfo-uri-cache:"<<uri.cache();
		LOG(INFO)<< "executor-commandinfo-uri-output_file:"<<uri.output_file();
	}
		LOG(INFO) << "executor-commandinfo-environment:";
		foreach(const Environment::Variable& var,task.executor().command().environment().variables()) {
		LOG(INFO)<< "executor-commandinfo-environment-variable-name:"<<var.name();
		LOG(INFO)<< "executor-commandinfo-environment-variable-type:"<<var.Type_Name(var.type());
		LOG(INFO)<< "executor-commandinfo-environment-variable-value:"<<var.value();
		LOG(INFO)<< "executor-commandinfo-environment-variable-secret:secret-type:"<<var.secret().Type_Name(var.secret().type())
		<<";secret-value:"<<var.secret().value().data()<<";secret-reference-name:"<<var.secret().reference().name()<<
		";secret-reference-key:"<<var.secret().reference().key();
	}
		LOG(INFO) << "executor-commandinfo-shell:"
				<< task.executor().command().shell();
		LOG(INFO) << "executor-commandinfo-value:"
				<< task.executor().command().value();
		LOG(INFO) << "executor-commandinfo-arguments:";
		foreach(const string& argument,task.executor().command().arguments()) {
		LOG(INFO)<<argument<<";";
	}
		LOG(INFO) << "executor-commandinfo-user:"
				<< task.executor().command().user();
		LOG(INFO) << "executor-containerinfo-type:"
				<< task.executor().container().Type_Name(
						task.executor().container().type());

		LOG(INFO) << "executor-resources:";
		foreach(const Resource& resource,task.executor().resources()) {
		LOG(INFO)<<"resource-name:"<<resource.name();
		LOG(INFO)<<"resource-type:"<<resource.type();
		LOG(INFO)<<"resource-scalar:"<<resource.scalar().value();
		LOG(INFO)<<"resource-ranges:";
		foreach(const Value::Range& range,resource.ranges().range()) {
			LOG(INFO)<<"range-begin:"<<range.begin();
			LOG(INFO)<<"range-end:"<<range.end();
		}
		LOG(INFO)<<"resource-set:";
		foreach(const string& item,resource.set().item()) {
			LOG(INFO)<<item<<";";
		}
		LOG(INFO);
		LOG(INFO)<<"resource-role:"<<resource.role();
	}
		LOG(INFO) << "executor-name:" << task.executor().name();
		LOG(INFO) << "executor-source:" << task.executor().source();
		LOG(INFO) << "executor-discoveryinfo:";
		LOG(INFO) << "executor-discoveryinfo-visibility:"
				<< task.executor().discovery().Visibility_Name(
						task.executor().discovery().visibility());
		LOG(INFO) << "executor-discoveryinfo-name:"
				<< task.executor().discovery().name();
		LOG(INFO) << "executor-discoveryinfo-environment:"
				<< task.executor().discovery().environment();
		LOG(INFO) << "executor-discoveryinfo-location:"
				<< task.executor().discovery().location();
		LOG(INFO) << "executor-discoveryinfo-version:"
				<< task.executor().discovery().version();
		LOG(INFO) << "executor-discoveryinfo-ports:";
		foreach(const Port& port,task.executor().discovery().ports().ports()) {
		LOG(INFO)<< "executor-discoveryinfo-ports-number:"<<port.number();
		LOG(INFO)<< "executor-discoveryinfo-ports-name:"<<port.name();
		LOG(INFO)<< "executor-discoveryinfo-ports-protocal:"<<port.protocol();
	}
		LOG(INFO) << "executor-discoveryinfo-labels:";
		foreach(const Label& label,task.executor().discovery().labels().labels()) {
		LOG(INFO)<< "executor-discoveryinfo-label-key:"<<label.key();
		LOG(INFO)<< "executor-discoveryinfo-label-value:"<<label.value();
	}

		LOG(INFO) << "executor-durationinfo:"
				<< task.executor().shutdown_grace_period().nanoseconds();

		LOG(INFO) << "executor-labels:";
		foreach(const Label& label,task.executor().labels().labels()) {
		LOG(INFO)<< "executor-label-key:"<<label.key();
		LOG(INFO)<< "executor-label-value:"<<label.value();
	}
		LOG(INFO);

		LOG(INFO) << "(6)task_commandinfo:";
		foreach(const CommandInfo::URI& uri,task.command().uris()) {
		LOG(INFO)<< "task-commandinfo-uri-value:"<< uri.value();
		LOG(INFO)<< "task-commandinfo-uri-executable:"<<uri.executable();
		LOG(INFO)<< "task-commandinfo-uri-extract:"<< uri.extract();
		LOG(INFO)<< "task-commandinfo-uri-cache:"<<uri.cache();
		LOG(INFO)<< "task-commandinfo-uri-output_file:"<<uri.output_file();
	}
		LOG(INFO) << "task-commandinfo-environment:";
		foreach(const Environment::Variable& var,task.command().environment().variables()) {
		LOG(INFO)<< "task-commandinfo-environment-variable-name:"<<var.name();
		LOG(INFO)<< "task-commandinfo-environment-variable-type:"<<var.Type_Name(var.type());
		LOG(INFO)<< "task-commandinfo-environment-variable-value:"<<var.value();
		LOG(INFO)<< "task-commandinfo-environment-variable-secret:secret-type:"<<var.secret().Type_Name(var.secret().type())
		<<";secret-value:"<<var.secret().value().data()<<";secret-reference-name:"<<var.secret().reference().name()<<
		";secret-reference-key:"<<var.secret().reference().key();
	}
		LOG(INFO) << "task-commandinfo-shell:" << task.command().shell();
		LOG(INFO) << "task-commandinfo-value:"
				<< task.executor().command().value();
		LOG(INFO) << "task-commandinfo-arguments:";
		foreach(const string& argument,task.command().arguments()) {
		LOG(INFO)<<argument<<";";
	}
		LOG(INFO) << "task-commandinfo-user:" << task.command().user();
		LOG(INFO);

		LOG(INFO) << "(7)task_containerinfo-type:"
				<< task.container().Type_Name(task.container().type());
		LOG(INFO) << "task_containerinfo-dockerinfo:"
				<< task.container().docker().image();
		LOG(INFO);
		LOG(INFO) << "(8)task_healthcheck:";
		LOG(INFO) << "task-healthcheck-delay_seconds:"
				<< task.health_check().delay_seconds();
		LOG(INFO) << "task-healthcheck-interval_seconds:"
				<< task.health_check().interval_seconds();
		LOG(INFO) << "task-healthcheck-timeout_seconds:"
				<< task.health_check().timeout_seconds();
		LOG(INFO) << "task-healthcheck-consecutive_failures:"
				<< task.health_check().consecutive_failures();
		LOG(INFO) << "task-healthcheck-grace_period_seconds:"
				<< task.health_check().grace_period_seconds();
		LOG(INFO) << "task-healthcheck-type:"
				<< task.health_check().Type_Name(task.health_check().type());
		LOG(INFO);
		LOG(INFO) << "(9)task_checkinfo:";
		LOG(INFO) << "task-checkinfo-type:"
				<< task.check().Type_Name(task.check().type());
		LOG(INFO);
		LOG(INFO) << "(10)task_killpolicy:"
				<< task.kill_policy().grace_period().nanoseconds();
		LOG(INFO);

		LOG(INFO) << "(11)task_data:" << task.data();
		LOG(INFO) << "(12)task_lables:";
		foreach(const Label& label,task.labels().labels()) {
		LOG(INFO)<< "task-label-key:"<<label.key();
		LOG(INFO)<< "task-label-value:"<<label.value();
	}
		LOG(INFO);
		LOG(INFO) << "(13)task-discoveryinfo:";
		LOG(INFO) << "task_discoveryinfo-visibility:"
				<< task.discovery().Visibility_Name(
						task.discovery().visibility());
		LOG(INFO) << "task-discoveryinfo-name:" << task.discovery().name();
		LOG(INFO) << "task-discoveryinfo-environment:"
				<< task.discovery().environment();
		LOG(INFO) << "task-discoveryinfo-location:"
				<< task.discovery().location();
		LOG(INFO) << "task-discoveryinfo-version:"
				<< task.discovery().version();
		LOG(INFO) << "task-discoveryinfo-ports:";
		foreach(const Port& port,task.discovery().ports().ports()) {
		LOG(INFO)<< "task-discoveryinfo-ports-number:"<<port.number();
		LOG(INFO)<< "task-discoveryinfo-ports-name:"<<port.name();
		LOG(INFO)<< "task-discoveryinfo-ports-protocal:"<<port.protocol();
	}
		LOG(INFO) << "task-discoveryinfo-labels:";
		foreach(const Label& label,task.discovery().labels().labels()) {
		LOG(INFO)<< "task-discoveryinfo-label-key:"<<label.key();
		LOG(INFO)<< "task-discoveryinfo-label-value:"<<label.value();
	}
		LOG(INFO);
		LOG(INFO) << "(14)task_durationinfo:"
				<< task.max_completion_time().nanoseconds();
		LOG(INFO);
		LOG(INFO) << "****************************************";

		//create ExecutorInfo
	}
	if (loglevel > LEVEL0) {
		LOG(INFO) << "taskid:" << task.task_id().value();
	}
	ExecutorInfo executor_info;
	executor_info.set_type(task.executor().type());
	ExecutorID executor_id;
	executor_id.set_value(task.task_id().value());
	executor_info.mutable_executor_id()->MergeFrom(executor_id);
	executor_info.mutable_framework_id()->MergeFrom(framework->id);
	executor_info.mutable_command()->MergeFrom(task.command());
	//executor_info.mutable_container()->MergeFrom(task.container());
	executor_info.mutable_resources()->MergeFrom(task.executor().resources());
	executor_info.set_name(task.name());
	executor_info.set_data(task.data());
	//executor_info.mutable_discovery()->MergeFrom(task.discovery());
	//executor_info.mutable_shutdown_grace_period()->MergeFrom(task.max_completion_time());

	if (loglevel > LEVEL0) {
		LOG(INFO) << "******create PlanUseResourceMessage*******";
	}
	PlanUseResourceMessage message;
	message.mutable_framawork_id()->MergeFrom(framework->id);
	message.mutable_executor_info()->MergeFrom(executor_info);
	message.set_offer_id(offer->id().value());
	message.mutable_slave_id()->set_value(offer->slave_id().value());
	//LOG(INFO)<<"offerid:"<<offer->
	link(negotiator_pid);
	send(negotiator_pid, message);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "******send PlanUseResourceMessage*******";
	}
}

void MasterProcess::receiveRespondMatchResultMessage(const UPID &from,
		RespondMatchResultMessage &&message) {
	if (loglevel > LEVEL0) {
		LOG(INFO) << "Receive RespondMatchResultMessage...";
	}
	Framework *framework = getFramework(message.framawork_id());
	if (framework == NULL){
		LOG(INFO)<<"framework is NULL"<<endl;
		return;
	}

//
	string respond = message.respond();
	if (loglevel > LEVEL0) {
		LOG(INFO) << "receive respond:" << respond << endl;
	}
	OfferID oid;
	oid.set_value(message.offer_id());
	if (respond != "USE") {
		//can't use this offer
		struct FailedTaskInfo fail;
		fail.frameworkId = message.framawork_id().value();
		fail.offerId = message.offer_id();
		fail.slaveid = message.slave_id().value();
		fail.execinfo = message.executor_info();
		framework->FailTaskSet[message.executor_info().executor_id().value()] = fail;
		LOG(INFO)<<"framework:"<<message.framawork_id().value()<< " execinfo:"<<message.executor_info().executor_id().value()<<"has again add into failtaskset."<<endl;
		if (loglevel > LEVEL0) {
			LOG(INFO) << message.offer_id()
					<< " be refused ,refuse executorid:"
					<< message.executor_info().executor_id().value() << endl;
		}
	} else {
		if (loglevel > LEVEL0) {
			LOG(INFO) << message.offer_id()
					<< " can use ,will start executorid:"
					<< message.executor_info().executor_id().value() << endl;
		}
		string gpuidx = "-1";
		//launch executor
		if ((framework->obtained_cpus_num < framework->required_cpus_max)
				|| (framework->obtained_gpus_num < framework->required_gpus_max)
				|| (framework->obtained_mem_num < framework->required_mem_max)) {

			foreach(const Resource& resource,framework->getOffer(oid)->resources()) {
			if(strcmp(resource.name().c_str(),"cpus")==0) {
				framework->obtained_cpus_num+=resource.scalar().value();
			}
			if(strcmp(resource.name().c_str(),"gpus")==0) {
				framework->obtained_gpus_num+=resource.scalar().value();
			}
			if(strcmp(resource.name().c_str(),"mem")==0) {
				framework->obtained_mem_num+=resource.scalar().value();
			}
			if(strcmp(resource.name().c_str(),"gpuids")==0) {
				LOG(INFO)<<"obtained_gpuid:"<<resource.set().item(0)<<endl;
				gpuidx=resource.set().item(0);

			}
		}
		apport.calculate_resourcefillrate(framework);
		if(!apport.Framework_Enough_Resource(framework)){
			LOG(INFO)<<"framework resource is not enough"<<endl;
			if(framework->type == "CPU"){
				apport.active_cpu_frameworks.insert(framework->id.value());
			}
			if(framework->type == "GPU" ){
				apport.active_gpu_frameworks.insert(framework->id.value());
			}
		}
		struct MacAndBatch mab;
		//mab=(struct MacAndBatch*)malloc(sizeof(struct MacAndBatch));

//20210514
//		char *ba, *host, temp[30] = {0};
//		strcpy(temp, message.slave_id().value().c_str());
//
//		ba = strtok(temp, "@");
//		strcpy(mab.batch, ba);
//		host = strtok(NULL, " ");
//		strcpy(mab.hostname, host);
//		balsche.lastest_machine= host;
//		LOG(INFO)<<"The lastest_machine:"<< balsche.lastest_machine <<endl;



		mab.execid=message.executor_info().executor_id().value();

		//cout << mab.batch << " "<< mab.hostname<< endl;

		framework->usingoffer.put(message.slave_id().value(),mab);



		if (strcmp(framework->info.name().c_str(), "Jobtest") == 0) {
			launchTestJavaExecutor(message.slave_id().value(),
					message.executor_info(), framework);
		} else if (strcmp(framework->info.name().c_str(), "SchedulerFramework")
				== 0) {
			launchTestCExecutor(message.slave_id().value(),message.offer_id(),
					message.executor_info(), framework);
		} else if(strcmp(framework->info.name().c_str(), "Prime")== 0) {
			launchTestCExecutor(message.slave_id().value(),message.offer_id(),
					message.executor_info(), framework);
		}else if(strcmp(framework->info.name().c_str(), "Rodina Kmeans")== 0){
			launchTestCExecutor(message.slave_id().value(),message.offer_id(),
					message.executor_info(), framework);
		}else {
			launchExecutor(message.slave_id().value(), message.executor_info(),
					framework,gpuidx);
		}
		//		//Determine if you need to continue to request resources
		if (loglevel > LEVEL0) {
			LOG(INFO) << "framework->obtained_cpus_num:"
			<< framework->obtained_cpus_num << " "
			<< "framework->required_cpus_max:"
			<< framework->required_cpus_max;
			LOG(INFO) << "framework->obtained_gpus_num"
			<< framework->obtained_gpus_num << " "
			<< "framework->required_gpus_max:"
			<< framework->required_gpus_max;
			LOG(INFO) << "framework->obtained_mem_num : "
			<< framework->obtained_mem_num << " "
			<< "framework->required_mem_max :"
			<< framework->required_mem_max;
		}
	}
}
}
void MasterProcess::launchTestCExecutor(const string &slaveid, string offerid,
		const ExecutorInfo &executor_info, Framework *framework) {
	string hrmhome = getenv("HRM_HOME");
	string executorid = executor_info.executor_id().value();

	string final_dir = hrmhome + "/FrameworkLog/" + framework->id.value();
	Executor *executor = new Executor(framework->id, executor_info, final_dir);

	framework->addExecutor(executor);

	string pri;
	if (framework->type == "CPU")
		pri = "1";
	else {
		pri = to_string(framework->queueprio);
	}

	if (loglevel > LEVEL0) {
		LOG(INFO) << "executorid:" << executor->id.value();
	}

	unsigned long subtime = getCurrenttime();

	string::size_type start = slaveid.find("@");
	string batch = slaveid.substr(0, start);
	string macname = slaveid.substr(start + 1, slaveid.length() - start);

	ExecutorInfoMessage msg;
	msg.set_batchname(batch);
	if(framework->info.name()=="Rodina Kmeans"){
		msg.add_command(
				"cd /data/006zzy/eclipse-workspace/SchedulerExecutorRodina/Debug");
	}else if(framework->info.name()=="Prime"){
		msg.add_command(
						"cd /data/006zzy/eclipse-workspace/SchedulerExecutorPrime/Debug");
	}else {
		msg.add_command(
						"cd /data/006zzy/eclipse-workspace/SchedulerExecutor/Debug");
	}

	msg.add_command(executor_info.command().value());
	msg.set_pri(pri);

	ExecutorRequestIDMessage message;
	message.mutable_framawork_id()->MergeFrom(framework->id);
	message.mutable_executor_info()->MergeFrom(executor_info);
	message.set_request_id("11");
	message.set_slave_id(slaveid);
	message.set_executor_submit_time(std::to_string(getCurrenttime()));
	msg.mutable_msg()->MergeFrom(message);

	string ip;
	if (!restracker.contains(macname)) {
		foreachpair(string key,string value,hostbyip) {
		if(strcmp(value.c_str(),macname.c_str())==0)
		{
			ip=key;
			break;
		}
	}
	uint16_t port = this->resouckerport;
	UPID respid = makepid(ip.c_str(), port, "ResourceTracker");
	restracker.put(macname, respid);
}

	UPID destpid = restracker[macname];
	if (loglevel > LEVEL0) {
		LOG(INFO) << "*************************send to " << destpid
				<< "for " << executor_info.executor_id().value() << " for "
				<< framework->id.value();
	}
	link(destpid);
	send(destpid, msg);

}
//void *threadarg
void execQsub(Framework *framework, ExecutorInfo executor_info, string slaveid,
		string subtime, string pri) {
	char buffer[128];
	char *firword;
	string sbuffer, ssecword;
	memset(buffer, 0, 128 * sizeof(char));
	struct timeval tv0;
	struct timezone tz0;
	gettimeofday(&tv0, &tz0);
	string hrmhome = getenv("HRM_HOME");
	string filepath = hrmhome + "/shellscript/" + framework->id.value() + "_"
			+ executor_info.executor_id().value() + ".txt";
	string::size_type start = slaveid.find("@");
	string batch = slaveid.substr(0, start);
	string macname = slaveid.substr(start + 1, slaveid.length() - start);
	string command = "qsub -q  " + batch + " " + hrmhome + "/shellscript/"
			+ framework->id.value() + "_" + executor_info.executor_id().value()
			+ "_executor.sh" + " -p " + pri + " -r executor_"
			+ executor_info.executor_id().value() + " -ko  -ke ";

	cout << "commond:" << command << endl;

	cout << "before qsub:" << endl;
	cout << "***frameworkid:" << framework->id.value() << " executorid:"
			<< executor_info.executor_id().value() << " slaveid:" << slaveid;
	//<<"->"<<getCurrentTime()<<endl;
	FILE *qsubout = popen(command.c_str(), "r");
	cout << "after qsub:" << endl;
	cout << "***frameworkid:" << framework->id.value() << " executorid:"
			<< executor_info.executor_id().value() << " slaveid:" << slaveid;
	//<<"->"<<getCurrentTime()<<endl;

	fgets(buffer, 1024, qsubout);
	cout << framework->id.value() << " executorid:"
			<< executor_info.executor_id().value();
	cout << buffer << "for" << executor_info.executor_id().value() << "for "
			<< framework->id.value() << endl;
	pclose(qsubout);
	sbuffer = buffer;
	firword = strtok(buffer, " ");
	ssecword = strtok(NULL, " ");
	if (strcmp(firword, "Request") == 0) {
		cout << "qsub success and request id is " << ssecword << endl;
		ExecutorRequestIDMessage message;
		message.mutable_framawork_id()->MergeFrom(framework->id);
		message.mutable_executor_info()->MergeFrom(executor_info);
		message.set_request_id(ssecword);
		message.set_slave_id(slaveid);
		message.set_executor_submit_time(subtime);
		cout << framework->id.value() << " executorid:"
				<< executor_info.executor_id().value();
		//<< " pid:"<< negotiator_pid;
//		link(negotiator_pid);
//		send(negotiator_pid, message);
		taskmutex.lock();
		eriqueue.push(message);
		taskmutex.unlock();

	}
}
void MasterProcess::launchTestJavaExecutor(const string &slaveid,
		const ExecutorInfo &executor_info, Framework *framework) {
	string hrmhome = getenv("HRM_HOME");
	string sparkhome = getenv("SPARK_HOME");
	string shelldir = hrmhome + "/executor.sh";
	ofstream openfile(hrmhome + "/executor.sh");
	if (openfile.fail()) {
		LOG(ERROR) << "File error!";
	} else {
		if (loglevel > LEVEL0) {
			LOG(INFO) << "create executor.sh!";
			LOG(INFO) << executor_info.command().value() << endl;
		}
		openfile << "#!/bin/bash" << endl;
	}

	string::size_type epos = executor_info.command().value().find(
			"--executor-id");
	string::size_type hpos = executor_info.command().value().find("--port");
	string executorid = executor_info.command().value().substr(epos + 14,
			hpos - epos - 15);

	string::size_type eposs = executor_info.command().value().find("/");
	string::size_type epoo = executor_info.command().value().find("--ip");
	string::size_type epoo1 = executor_info.command().value().find("--port1");
	string execpath = executor_info.command().value().substr(0, eposs);
	string execp = executor_info.command().value().substr(eposs + 1,
			epos - eposs - 1);
	string port = executor_info.command().value().substr(hpos + 6,
			epoo - hpos - 6);
	string ip = executor_info.command().value().substr(epoo + 4,
			executor_info.command().value().length() - epoo - 4);

	openfile << "cd " << hrmhome << "/test/" << execpath << endl;

	string cmd = "java " + execp + " " + executorid + " " + port + " " + ip;
	openfile << cmd << endl;

	openfile.close();
	if (loglevel > LEVEL0) {
		LOG(INFO) << "cmd: " << cmd;
		LOG(INFO) << "executorid:" << executorid;
	}
	string final_dir = tasklog->createDir(work_dir + "/FrameworkLog/",
			framework->id.value(), executorid);
	tasklog->createExec_s(executorid, hrmhome + "/", "executor.sh", final_dir);
//
//	//add executor to framework's hashmap <executors>
	Executor *executor = new Executor(framework->id, executor_info, final_dir);
	framework->addExecutor(executor);

	string pipecommand;
	string qsubout = work_dir + "/qsubout.txt";	//to_string(framework->queueprio)
	string pri;

	if (framework->type == "CPU")
		pri = "1";
	else {
		pri = to_string(framework->queueprio);
	}

	pipecommand = "qsub -q pipe1 -ds " + slaveid + " " + final_dir + executorid
			+ ".sh -p " + pri + " -ko " + " -ke " + " > " + qsubout;
	if (loglevel > LEVEL0) {
		LOG(INFO) << "pipeqsubstr:" << pipecommand;
	}
	unsigned long subtime = getCurrenttime();
	system(pipecommand.c_str());
	ifstream qsuboutfile(qsubout);
	char buffer[128];
	char *firword;
	string sbuffer, ssecword;
	qsuboutfile.getline(buffer, sizeof(buffer));

	sbuffer = buffer;
	firword = strtok(buffer, " ");
	ssecword = strtok(NULL, " ");
	if (strcmp(firword, "Request") == 0) {
		if (loglevel > LEVEL0) {
			LOG(INFO) << "qsub success and request id is " << ssecword;
			//send ExecutorRequestIDMessage to Negotiator
			LOG(INFO) << "*****create ExecutorRequestIDMessage*******";
		}
		ExecutorRequestIDMessage message;
		message.mutable_framawork_id()->MergeFrom(framework->id);
		message.mutable_executor_info()->MergeFrom(executor_info);
		message.set_request_id(ssecword);
		message.set_slave_id(slaveid);
		message.set_executor_submit_time(std::to_string(subtime));
		link(negotiator_pid);
		send(negotiator_pid, message);
		if (loglevel > LEVEL0) {
			LOG(INFO) << "*****send ExecutorRequestIDMessage*******";
		}
	}

}
void MasterProcess::launchExecutor(const string &slaveid,
		const ExecutorInfo &executor_info, Framework *framework,string gpuidx) {

	LOG(INFO)<<"launchExecutor..."<<endl;
	std::string frameworkhome = this->frameworkhome;
	std::string sparkhome = getenv("SPARK_HOME");
	std::string ldconf = getenv("PKG_CONFIG_PATH");
	std::string ldpath = getenv("LD_LIBRARY_PATH");
	ExecutorInfoMessage msg;
	LOG(INFO)<<"start environment..."<<endl;
	foreach(const Environment::Variable& var,executor_info.command().environment().variables()) {
		if(strcmp(var.name().c_str(),"SPARK_EXECUTOR_OPTS")==0) continue;
		msg.add_command("export "+var.name()+"="+var.value());
	}
	if (loglevel > LEVEL0)LOG(INFO) << "ldconf=" << ldconf << endl;
	if (loglevel > LEVEL0)LOG(INFO) << "ldpath=" << ldpath << endl;
	std::string templdconf = "export PKG_CONFIG_PATH=" + ldconf;
	std::string templdpath = "export LD_LIBRARY_PATH=" + ldpath;
	if (gpuidx != "-1")
		msg.add_command("export CUDA_VISIBLE_DEVICES=" + gpuidx);
	msg.add_command(templdconf);
	msg.add_command(templdpath);
	if (loglevel > LEVEL0)LOG(INFO) <<"templdpath="<<templdpath<<endl;
	if (loglevel > LEVEL0)LOG(INFO) <<"templdconf="<<templdconf<<endl;

	foreach(const Environment::Variable& var,executor_info.command().environment().variables()) {
		if (loglevel > LEVEL0)LOG(INFO)<<"varname="<<var.name()<<endl;
		msg.add_command("echo $"+var.name());
	}
	msg.add_command("cd " + sparkhome);
	if (loglevel > LEVEL0)LOG(INFO)<<"sparkhome="<<sparkhome<<endl;
	string::size_type pos = executor_info.command().value().find(
			"org.apache.spark");
	if (loglevel > LEVEL0)LOG(INFO)<<"pos="<<pos<<endl;
	msg.add_command(
			"./bin/spark-class " + executor_info.command().value().substr(pos));

	string::size_type epos = executor_info.command().value().find(
			"--executor-id");
	if (loglevel > LEVEL0)LOG(INFO)<<"epos="<<epos<<endl;
	string::size_type hpos = executor_info.command().value().find("--hostname");
	if (loglevel > LEVEL0)LOG(INFO)<<"hpos="<<hpos<<endl;
	string executorid = executor_info.command().value().substr(epos + 14,
			hpos - epos - 15);
	if (loglevel > LEVEL0) LOG(INFO)<<"executorid ="<<executorid <<endl;
	if (loglevel > LEVEL0) {
		LOG(INFO) << "executorid:" << executorid;
	}

	string final_dir = frameworkhome + "/log/FrameworkLog/" + framework->id.value();
	Executor *executor = new Executor(framework->id, executor_info, final_dir);
	framework->addExecutor(executor);

	string pri;
	if (framework->type == "CPU")
		pri = "1";
	else {
		pri = to_string(framework->queueprio);
	}

	string::size_type start = slaveid.find("@");
	string batch = slaveid.substr(0, start);
	string macname = slaveid.substr(start + 1, slaveid.length() - start);

	msg.set_batchname(batch);
	msg.set_pri(pri);

	ExecutorRequestIDMessage message;
	message.mutable_framawork_id()->MergeFrom(framework->id);
	message.mutable_executor_info()->MergeFrom(executor_info);
	message.set_request_id("11");
	message.set_slave_id(slaveid);
	message.set_executor_submit_time(std::to_string(getCurrenttime()));
	msg.mutable_msg()->MergeFrom(message);

	string ip;
	if (!restracker.contains(macname)) {
		foreachpair(string key,string value,hostbyip) {
		if(strcmp(value.c_str(),macname.c_str())==0)
		{
			ip=key;
			break;
		}
	}
	uint16_t port = this->resouckerport;
	UPID respid = makepid(ip.c_str(), port, "ResourceTracker");
	restracker.put(macname, respid);
}

	UPID destpid = restracker[macname];
	if (loglevel > LEVEL0) {
		LOG(INFO) << "*************************send to " << destpid
				<< "for " << executor_info.executor_id().value() << " for "
				<< framework->id.value();
	}
	link(destpid);
	send(destpid, msg);

}

void MasterProcess::receiveTaskStateMessage(const UPID &from,
		UpdateTaskStateMessage &&message) {

}

void MasterProcess::sendUpdateTaskStateMessage(Framework *framework) {

	UpdateTaskStateMessage msg;
	FrameworkID fid;
	fid.set_value(framework->id.value());
//msg.framawork_id().set_value(framework->id.value());
	msg.mutable_framawork_id()->MergeFrom(fid);
	msg.set_totaltasks(framework->total_tasks_num);

	for (int taskid : framework->finished) {
		UpdateTaskStateMessage::taskstate state;
		state.set_taskid(std::to_string(taskid));
		state.set_task_state(UpdateTaskStateMessage::TASK_FINISH);
		msg.add_state()->MergeFrom(state);
	}
	for (int taskid : framework->starting) {
		UpdateTaskStateMessage::taskstate state;
		state.set_taskid(std::to_string(taskid));
		state.set_task_state(UpdateTaskStateMessage::TASK_STARTING);
		msg.add_state()->MergeFrom(state);
	}
	for (int taskid : framework->failed) {
		UpdateTaskStateMessage::taskstate state;
		state.set_taskid(std::to_string(taskid));
		state.set_task_state(UpdateTaskStateMessage::TASK_FAILED);
		msg.add_state()->MergeFrom(state);
	}
	for (int taskid : framework->lost) {
		UpdateTaskStateMessage::taskstate state;
		state.set_taskid(std::to_string(taskid));
		state.set_task_state(UpdateTaskStateMessage::TASK_LOST);
		msg.add_state()->MergeFrom(state);
	}
	for (int taskid : framework->killed) {
		UpdateTaskStateMessage::taskstate state;
		state.set_taskid(std::to_string(taskid));
		state.set_task_state(UpdateTaskStateMessage::TASK_kILLED);
		msg.add_state()->MergeFrom(state);
	}
	for (int taskid : framework->running) {
		UpdateTaskStateMessage::taskstate state;
		state.set_taskid(std::to_string(taskid));
		state.set_task_state(UpdateTaskStateMessage::TASK_RUNNING);
		msg.add_state()->MergeFrom(state);
	}
    link(negotiator_pid);
	send(negotiator_pid, msg);
}

void MasterProcess::acknowledge(Framework *framework,
		scheduler::Call::Acknowledge &&acknowledge) {

	if (loglevel > LEVEL0) {
		LOG(INFO)
				<< "rescive task state update message for executor**************";
	}
	char *exectaskid, *slavetasks;
	char *execid, *taskid, *state;
	char *slavename, *totaltask;
	int count = 0, sum = 0, i = 0;
	vector<string> vtaskid;
	vector<string> vexecid;
	vector<string> vstate;
	vector<string> vslave;

	string framid = framework->id.value();
	string slaveandtotaltasks = acknowledge.slave_id().value();
	string taskId = acknowledge.task_id().value();
	slavetasks = (char*) malloc(
			(slaveandtotaltasks.length() + 1) * sizeof(char));
	exectaskid = (char*) malloc((taskId.length() + 1) * sizeof(char));

	strcpy(slavetasks, slaveandtotaltasks.c_str());
	while (i < slaveandtotaltasks.length()) {
		if (slavetasks[i] == '+')
			sum++;
		i++;
	}
//	LOG(INFO) << "slave:" << slaveandtotaltasks;
//	LOG(INFO) << "taskid:" << taskId;
	if (loglevel > LEVEL0) {
		LOG(INFO) << "it will update " << sum << " task state";
	}
	if (sum == 0)
		return;

	slavename = strtok(slavetasks, "+");
	vslave.push_back(slavename);
	for (count = 1; count < sum; count++) {
		slavename = strtok(NULL, "+");
		vslave.push_back(slavename);
	}
	totaltask = strtok(NULL, " ");

	strcpy(exectaskid, taskId.c_str());

	execid = strtok(exectaskid, "+");
	taskid = strtok(NULL, "+");
	state = strtok(NULL, " ");
	vexecid.push_back(execid);
	vstate.push_back(state);
	vtaskid.push_back(taskid);
	for (count = 1; count < sum; count++) {
		execid = strtok(NULL, "+");
		taskid = strtok(NULL, "+");
		state = strtok(NULL, " ");
		vexecid.push_back(execid);
		vstate.push_back(state);
		vtaskid.push_back(taskid);
	}

	Framework *frame = frameworks[framework->id.value()];
	frame->total_tasks_num = atoi(totaltask);
	for (count = 0; count < sum; count++) {
		updatetaskstate(frame, atoi(vtaskid[count].c_str()), vstate[count]);
	}
	free(exectaskid);
	free(slavetasks);
	sendUpdateTaskStateMessage(framework);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "sucessfully update task state from "
				<< framework->id.value() << "and send update to negotiator";
		LOG(INFO)
				<< "end rescive task state update message for executor**************";
	}
}

bool MasterProcess::taskisexist(Framework *frame, int taskid) {
	return frame->finished.contains(taskid) || frame->running.contains(taskid)
			|| frame->failed.contains(taskid) || frame->lost.contains(taskid)
			|| frame->killed.contains(taskid)
			|| frame->starting.contains(taskid) || frame->other.contains(taskid);
}

void MasterProcess::updatetaskstate(Framework *frame, int taskid,
		string state) {
	if (taskisexist(frame, taskid)) {
		if (frame->finished.contains(taskid)) {
			if (strcmp(state.c_str(), "FINISHED")) {
				frame->finishedtasks--;
				frame->finished.erase(taskid);
				findSetAndInsert(frame, taskid, state);
			}
		} else if (frame->running.contains(taskid)) {
			if (strcmp(state.c_str(), "RUNNING")) {
				frame->runningtasks--;
				frame->running.erase(taskid);
				findSetAndInsert(frame, taskid, state);
			}
		} else if (frame->failed.contains(taskid)) {
			if (strcmp(state.c_str(), "FAILED")) {
				frame->failedtasks--;
				frame->failed.erase(taskid);
				findSetAndInsert(frame, taskid, state);
			}
		} else if (frame->lost.contains(taskid)) {
			if (strcmp(state.c_str(), "LOST")) {
				frame->losttasks--;
				frame->lost.erase(taskid);
				findSetAndInsert(frame, taskid, state);
			}
		} else if (frame->killed.contains(taskid)) {
			if (strcmp(state.c_str(), "KILLED")) {
				frame->killedtasks--;
				frame->killed.erase(taskid);
				;
				findSetAndInsert(frame, taskid, state);
			}
		} else if (frame->starting.contains(taskid)) {
			if (strcmp(state.c_str(), "LAUNCHING")) {
				frame->startingtasks--;
				frame->starting.erase(taskid);
				findSetAndInsert(frame, taskid, state);
			}
		} else {
			frame->othertasks--;
			frame->other.erase(taskid);
			findSetAndInsert(frame, taskid, state);
		}
	} else {
		findSetAndInsert(frame, taskid, state);
	}
}
void MasterProcess::findSetAndInsert(Framework *frame, int taskid,
		string state) {
	if (strcmp(state.c_str(), "FINISHED") == 0) {
		frame->finished.insert(taskid);
		frame->finishedtasks++;
	} else if (strcmp(state.c_str(), "RUNNING") == 0) {
		frame->running.insert(taskid);
		frame->runningtasks++;
	} else if (strcmp(state.c_str(), "KILLED") == 0) {
		frame->killed.insert(taskid);
		frame->killedtasks++;
	} else if (strcmp(state.c_str(), "FAILED") == 0) {
		frame->failed.insert(taskid);
		frame->failedtasks++;
	} else if (strcmp(state.c_str(), "LOST") == 0) {
		frame->lost.insert(taskid);
		frame->losttasks++;
	} else if (strcmp(state.c_str(), "LAUNCHING") == 0) {
		frame->starting.insert(taskid);
		frame->startingtasks++;
	} else {
		frame->other.insert(taskid);
		frame->othertasks++;
	}
}

void MasterProcess::decline(Framework *framework,
		scheduler::Call::Decline &&decline) {
	if (loglevel > LEVEL2) {
		LOG(INFO) << "************************************";
		foreach(const OfferID& offerId,decline.offer_ids()) {
		LOG(INFO)<<"receive declineOfferMessage "<<offerId.value()<<" from framework "<<framework->info.name()<<"...";
	}
		LOG(INFO) << "************************************";
	}
}

void MasterProcess::teardown(Framework *framework) {
	if (framework == NULL)
		return;
	if (loglevel > LEVEL0) {
		LOG(INFO)
				<< "************receive call to UnregisterFramework****************";
		LOG(INFO) << "TOTAL TASKS: " << framework->total_tasks_num;
		LOG(INFO) << "running tasks: " << framework->runningtasks << " "
				<< framework->running.size();
		LOG(INFO) << "lost tasks: " << framework->losttasks << " "
				<< framework->lost.size();
		LOG(INFO) << "killed: " << framework->killedtasks << " "
				<< framework->killed.size();
		LOG(INFO) << "launching tasks: " << framework->startingtasks << " "
				<< framework->starting.size();
		LOG(INFO) << "failed tasks: " << framework->failedtasks << " "
				<< framework->failed.size();
		LOG(INFO) << "finish tasks: " << framework->finishedtasks << " "
				<< framework->finished.size();
		LOG(INFO) << "other tasks: " << framework->othertasks << " "
				<< framework->other.size();
	}
	if(framework->info.name()!="AppResourceCheck")
	   sendUnregisterTimeMessage(framework);
	removeFramework(framework);
	LOG(INFO) << "************************************";
}

void MasterProcess::removeFramework(Framework *framework) {
	if (loglevel > LEVEL1) {
		LOG(INFO) << "************remove Framework****************";
		LOG(INFO) << "framework executor size:"
				<< framework->executors.size();

		foreachvalue(const Executor* executor,framework->executors) {
		if(framework->executors.empty()) {
			break;
		}
		LOG(INFO)<<"executor-id:"<<executor->id.value();
	}
		LOG(INFO) << "***************begin remove framework************";
	}
	if(framework->type == "GPU") apport.active_gpu_frameworks.erase(framework->id.value());
	if(framework->type == "CPU") apport.active_cpu_frameworks.erase(framework->id.value());

	frameworks.erase(framework->id.value());
	if (frameworks.empty()) {
		LOG(WARNING) << "Master does not have any uncompleted framework.";
	} else {
		LOG(INFO) << "Master still have " << frameworks.size()
				<< " registered frameworks:";
		if (loglevel > LEVEL0) {
			foreachvalue(const Framework* framework,frameworks) {
			LOG(INFO)<<"FrameworkID:"<<framework->id.value();
		}
	}

	}
}

void MasterProcess::sendUnregisterTimeMessage(Framework *framework) {
	if (loglevel > LEVEL0) {
		LOG(INFO) << "*******create UnregisterTimeMessage********";
	}
	UnregisterTimeMessage message;
	message.mutable_framework_id()->MergeFrom(framework->id);
//	time_t t = time(0);
//	char now[64];
//	strftime(now, sizeof(now), "%Y-%m-%d %H-%M-%S", localtime(&t));
//	LOG(INFO) << "unregister-time:" << now;
	unsigned long untime = getCurrenttime();
	string filepath = work_dir + "/DS-"+localip+"-frameworkResult.txt";
	LOG(INFO)<<"frameworkResult path:"<<filepath<<endl;
	ofstream ofile(filepath, ios::app);
	ofile << framework->id.value() << " "<<framework->firstregister<< " "<<framework->firstreceiveoffer << " "<<untime<<endl;
	ofile.close();
	message.set_unregister_time(std::to_string(untime));
//	link(negotiator_pid);
	send(negotiator_pid, message);
	if (loglevel > LEVEL0) {
		LOG(INFO) << "*******send UnregisterTimeMessage********";
	}
}

void MasterProcess::set_request_delay(string str) {
	this->request_delay = str;
}

void MasterProcess::setLogLevel(string loglevel) {
	if (isNum(loglevel))
		this->loglevel = atoi(loglevel.c_str());
	else {
		this->loglevel = LEVEL1;
	}
	LOG(INFO) << "loglevel:" << this->loglevel;
}
bool MasterProcess::isNum(string str) {
	int i = 0;
	char c[5] = { 0 };
	strcpy(c, str.c_str());
	for (; i < str.length(); i++) {
		if (c[i] < '0' && c[i] > '9')
			return false;
	}
	return true;
}

//void MasterProcess::SortOffers(vector<QueueOffer>& offers) {
//	if (loglevel > LEVEL1) {
//			LOG(INFO) << "Sort offers ..." << endl;
//	}
//	if(SchedulerMethod == "LD_Scheduler" || SchedulerMethod== "RR_Scheduler"){
//		for(int i = 0;i < offers.size()-1;i++){
//			for(int j =0; j < offers.size()-1-i;j++){
//				int slot1 = offers[j].can_allocate_num();
//				int slot2 = offers[j+1].can_allocate_num();
//				if(slot1 < slot2){
//					std::swap(offers[j],offers[j+1]);
//				}
//			}
//		}
//	}
//	LOG(INFO) <<"Sort offers result by can_allocate_num:"<<endl;
//	for(int i = 0;i < offers.size();i++){
//		LOG(INFO) <<"SortOffer_result:"<< offers[i].queue_name()<<endl;
//	}
//}
void MasterProcess::Readetchosts() {
	ifstream fs("/etc/hosts");
	if (!fs.is_open()) {
		std::cout << "file open error..." << endl;
	}
	std::vector<string> confs;
	string conf;
	while (getline(fs, conf)) {
		confs.push_back(conf);
	}
//	std::cout << "size :" << confs.size()<<endl;
	for (string::size_type i = 0; i < confs.size(); i++) {
		string t = confs[i];
//		std::cout << "t:"<<t<<endl;
		if (t.find_first_of("#") == 0) {
			continue;
		}
		int index = t.find_first_of(" ");
		if (index == -1)
			continue;
////		std::cout << "index = " << index << endl;
		string ip = t.substr(0, index);
		string name = t.substr(index + 1, t.size() - index - 1);
		//	LOG(INFO) << "ip:" << ip << " name:"<< name<<endl;
		hostbyip.put(ip, name);
	}
	//LOG(INFO)<<"hostbyip.size:"<<hostbyip.size();
}

bool MasterProcess::getHostInfo(string &hostname, string &ip) {
	char name[256];
	gethostname(name, sizeof(name));
	hostname = name;

	struct hostent *host = gethostbyname(name);
	char ipStr[32];
	const char *ret = inet_ntop(host->h_addrtype, host->h_addr_list[0], ipStr,
			sizeof(ipStr));
	if (NULL == ret) {
		cout << "hostname transform to ip failed" << endl;
		return false;
	}
	ip = ipStr;
	return true;
}
void MasterProcess::receiveresofqueue(const process::UPID &from,
		QueueFreeResourceRequest &&message) {
	LOG(INFO) << "receive receiveresofqueue";
//	cout << message.cpuidlecores() << endl;
//	cout << message.memory_usage() << endl;
//	for (int i = 0; i < message.mutable_gpuid()->mapp_size(); i++) {
//		cout << message.mutable_gpuid()->mapp(i).value() << endl;
//	}
//	cout << endl;
//	for (int i = 0; i < message.mutable_gpuutilation()->mapp_size(); i++) {
//		cout << message.mutable_gpuutilation()->mapp(i).value() << endl;
//	}
//	cout << endl;
//	for (int i = 0; i < message.mutable_gpuusedmem()->mapp_size(); i++) {
//		cout << message.mutable_gpuusedmem()->mapp(i).value() << endl;
//	}
	cout << endl;
	Offer *offer = new Offer();
	Framework *framework=frameworks[message.fid()];
	int i;

	OfferID oid;
	oid.set_value("111");
	offer->mutable_id()->MergeFrom(oid);

	offer->mutable_framework_id()->MergeFrom(framework->id);

	SlaveID sid;
	sid.set_value("121");
	offer->mutable_slave_id()->MergeFrom(sid);

	offer->set_hostname("master");

	Resource cpu;
	cpu.set_name("idleacpucores");
	cpu.set_type(Value::SCALAR);
	cpu.mutable_scalar()->set_value(message.cpuidlecores());
	offer->add_resources()->MergeFrom(cpu);

	Resource mem;
	mem.set_name("memusage");
	mem.set_type(Value::SCALAR);
	mem.mutable_scalar()->set_value(message.memory_usage());
	offer->add_resources()->MergeFrom(mem);

	Resource gpuid;
	gpuid.set_name("gpus");
	gpuid.set_type(Value::SET);
	for (i = 0; i < message.mutable_gpuid()->mapp_size(); i++) {
		string value = message.mutable_gpuid()->mapp(i).value();
		gpuid.mutable_set()->add_item(value);
	}
	offer->add_resources()->MergeFrom(gpuid);

	Resource gpuutil;
	gpuutil.set_name("gpuutil");
	gpuutil.set_type(Value::SET);
	for (i = 0; i < message.mutable_gpuutilation()->mapp_size(); i++) {
		string value = message.mutable_gpuutilation()->mapp(i).value();
		gpuutil.mutable_set()->add_item(value);
	}
	offer->add_resources()->MergeFrom(gpuutil);

	Resource gpumemusage;
	gpumemusage.set_name("gpumemusage");
	gpumemusage.set_type(Value::SET);
	for (i = 0; i < message.mutable_gpuusedmem()->mapp_size(); i++) {
		string value = message.mutable_gpuusedmem()->mapp(i).value();
		gpumemusage.mutable_set()->add_item(value);
	}
	offer->add_resources()->MergeFrom(gpumemusage);

	ExecutorID executor_id;
	executor_id.set_value("121");
	offer->add_executor_ids()->MergeFrom(executor_id);

	ResourceOffersMessage msg;
	msg.add_offers()->MergeFrom(*offer);
	msg.add_pids(master);
	//cout << "send to " << framework->info.name() << endl;
	send(framework->pid, msg);
}
void MasterProcess::sendnoresource(int str,Framework* framework) {
	Offer *offer = new Offer();
	int i;
	if (framework)
		LOG(INFO) << "not null";
	OfferID oid;
	oid.set_value("-1");
	offer->mutable_id()->MergeFrom(oid);
	offer->mutable_framework_id()->MergeFrom(framework->id);

	SlaveID sid;
	sid.set_value("121");
	offer->mutable_slave_id()->MergeFrom(sid);

	offer->set_hostname("master");

	Resource cpu;
	cpu.set_name("reason");
	cpu.set_type(Value::SCALAR);
	cpu.mutable_scalar()->set_value(1);
	offer->add_resources()->MergeFrom(cpu);

	ExecutorID executor_id;
	executor_id.set_value("121");
	offer->add_executor_ids()->MergeFrom(executor_id);

	ResourceOffersMessage msg;
	msg.add_offers()->MergeFrom(*offer);
	msg.add_pids(master);
	cout << "send to " << framework->info.name() << endl;
	send(framework->pid, msg);
}
vector<string> MasterProcess::getIpByName(char *mname) {
	struct hostent *he = gethostbyname(mname);

	vector<string> ips;
	char destIP[128];
	char **phe = nullptr;
	if (he == NULL)
		return ips;
	for (phe = he->h_addr_list; *phe != nullptr; ++phe) {
		//change binary ip to char* ip
		inet_ntop(he->h_addrtype, *phe, destIP, sizeof(destIP));
		//printf("addr:%s\n", destIP);
		ips.push_back(destIP);
	}
	return ips;
}
void MasterProcess::DeliverExecutor(const process::UPID &from,DeliverExecutorMessage&& message){
	LOG(INFO)<<"receive DeliverExecutorMessage from Negotiator..."<<endl;
	string frameworkid = message.framework_id();
	string executorid = message.executor_id();
	string sourceofferid = message.source_offerid();
	string targetofferid = message.target_offerid();
	string requestid = message.requestid();
	int sourcemachineindex = sourceofferid.find_first_of("@");
	int targetmachineindex = targetofferid.find_first_of("@");
	string sourcemachinename = sourceofferid.substr(sourcemachineindex+1,sourceofferid.size()-1-sourcemachineindex);
	string targetmachinename = targetofferid.substr(targetmachineindex+1,sourceofferid.size()-1-targetmachineindex);
	LOG(INFO)<<"sourcemachinename:"<<sourcemachinename<<endl;
	LOG(INFO)<<"targetmachinename:"<<targetmachinename<<endl;
	LOG(INFO)<<"requestid:"<< requestid << " will be killed" << endl;
	UPID sourceresourcetrackerpid = restracker[sourcemachinename];
	if (!restracker.contains(targetmachinename)) {
		foreachpair(string key,string value,hostbyip) {
			if(strcmp(value.c_str(),targetmachinename.c_str())==0)
			{
				ip=key;
				break;
			}
		}
		uint16_t port = this->resouckerport;
		UPID respid = makepid(ip.c_str(), port, "ResourceTracker");
		restracker.put(targetmachinename, respid);
	}

	UPID targetsourcetrackerpid = restracker[targetmachinename];

	//kill old executor
	KillRequestIDMessage killmess;
	killmess.set_framework_id(frameworkid);
	killmess.set_executor_id(executorid);
	killmess.set_requestid(requestid);
	killmess.set_offer_id(sourceofferid);
	killmess.set_new_offer_id(targetofferid);
	LOG(INFO)<<"Send KillRequestIDMessage to resourcetracker:"<<sourceresourcetrackerpid<<endl;
	send(sourceresourcetrackerpid,killmess);
	return ;
}
}	// namespace master {
}	// namespace internal {
}	// namespace mesos {

