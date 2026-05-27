> /Users/adarshnanu/Downloads/allcode.txt
for i in `find /Users/adarshnanu/drogon/build/payment_app -name "*.h"`
do
	echo "-----------$i-----------" >>/Users/adarshnanu/Downloads/allcode.txt
	cat $i >>/Users/adarshnanu/Downloads/allcode.txt
done
for i in `find /Users/adarshnanu/drogon/build/payment_app -name "*.cc"`
do
	echo "-----------$i-----------" >>/Users/adarshnanu/Downloads/allcode.txt
	cat $i >>/Users/adarshnanu/Downloads/allcode.txt
done
