function LogImpl_runAV_y() {
    dt=$(date +"%d-%m-%y %T")
    echo $dt $1 >> "runAv.log"
    echo -e "$dt : \033[33;33m$1""\033[0m"
}

function LogImpl_runAV_g() {
    dt=$(date +"%d-%m-%y %T")
    echo $dt $1 >> "runAv.log"
    echo -e "$dt : \033[33;32m$1""\033[0m"
}

function LogImpl_runAV() {
    dt=$(date +"%d-%m-%y %T")
    echo $dt $1 >> "runAv.log"
    echo "$dt : $1"
}

function PrepareVMForFile() {
	fileSrc=$2
	vm=$1
	echo -e "\033[33;36m"
	if [[ ! -z "$fileSrc" ]] ; then
		name=$(basename "$fileSrc")
		LogImpl_runAV "|-->"$name
		source prepareVM.sh $vm 903 $fileSrc
	fi
	echo -e "\033[0m"
}

function StartVM() {
	fileSrc=$2
	vm=$1
	if [[ ! -z "$fileSrc" ]] ; then
		LogImpl_runAV_y "start "$vm
		qm start $vm
		sleep 10
	fi
}

function StopVM() {
	fileSrc=$2
	vm=$1
	if [[ ! -z "$fileSrc" ]] ; then
		LogImpl_runAV_y "stop "$vm
		qm stop $vm
	fi
}

virusDirName=$1
path=/var/lib/vz/samples/!VIRUS/$virusDirName
shopt -s nullglob
for d in $path/*/;
do
    LogImpl_runAV_g "Working on dir: "$d
	files=($d*)
	
	LogImpl_runAV_g "Starting to prepare VM's"
	PrepareVMForFile 100 ${files[0]}
	PrepareVMForFile 101 ${files[1]}
	PrepareVMForFile 102 ${files[2]}
	PrepareVMForFile 103 ${files[3]}
	PrepareVMForFile 104 ${files[4]}
	PrepareVMForFile 105 ${files[5]}
	LogImpl_runAV_g "Finished to prepare VM's"
	
	LogImpl_runAV_g "Starting to run VM's"
	StartVM 100 ${files[0]}
	StartVM 101 ${files[1]}
	StartVM 102 ${files[2]}
	StartVM 103 ${files[3]}
	StartVM 104 ${files[4]}
	StartVM 105 ${files[5]}
	LogImpl_runAV_g "Finished to run VM's"
	
	
	LogImpl_runAV_g "Waiting for virus to execute: sleep 30m"
	sleep 30m

	LogImpl_runAV_g "Starting to stop VM's"
	StopVM 100 ${files[0]}
	StopVM 101 ${files[1]}
	StopVM 102 ${files[2]}
	StopVM 103 ${files[3]}
	StopVM 104 ${files[4]}
	StopVM 105 ${files[5]}
	LogImpl_runAV_g "Finished to stop VM's"

	LogImpl_runAV_g "Dir work finished."
done