#**********************
#*
#* Progam Name: MP1. Membership Protocol.
#*
#* Current file: run.sh
#* About this file: Submission shell script.
#* 
#***********************
#!/bin/sh
sudo mkdir grade-dir
cd grade-dir
wget https://spark-public.s3.amazonaws.com/cloudcomputing/assignments/mp1/mp1.zip || { echo 'ERROR ... Please install wget'; exit 1; }
unzip mp1.zip || { echo 'ERROR ... Zip file not found' ; exit 1; }
cd mp1
cp ../../MP1Node.* .
make clean > /dev/null
make > /dev/null
case $1 in
	0) echo "Single failure"
	./Application testcases/singlefailure.conf > /dev/null;;
	1) echo "Multiple failure"
	./Application testcases/multifailure.conf > /dev/null;;
	2) echo "Single failure with Messages Drop"
	./Application testcases/msgdropsinglefailure.conf > /dev/null;;
	*) echo "Please enter a valid option";;
esac
cp dbg.log ../../
cd ../../
rm -rf grade-dir
