echo "------------ First Removing DISK_0 ------------"
# rm DISK_0

echo -e "=========== Testing TA's FS_partIII.sh... ==========="

../build.linux/nachos -f # if format: get error!

echo -e "\n----------- Mkdir /t0------------"
../build.linux/nachos -mkdir /t0

echo -e "\n----------- Mkdir /t1 ------------"
../build.linux/nachos -mkdir /t1
echo -e "\n----------- Mkdir /t2 ------------"
../build.linux/nachos -mkdir /t2

echo -e "\n----------- List root dir ------------"
../build.linux/nachos -l /

echo -e "\n----------- Test copy file ------------"

echo -e "------------ Copy file num_100.txt to nachOS FS /t0/f1: ------------"
# echo -e "Copy file num_100.txt to nachOS FS /t0/f1: \n"
../build.linux/nachos -cp num_100.txt /t0/f1

echo -e "\n----------- Mkdir /t0/aa ------------"
../build.linux/nachos -mkdir /t0/aa

echo -e "\n----------- Mkdir /t0/bb ------------"
../build.linux/nachos -mkdir /t0/bb

echo -e "\n----------- Mkdir /t0/cc ------------"
../build.linux/nachos -mkdir /t0/cc


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f1: ------------"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f1: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f1


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f2: ------------"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f2: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f2


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f3: ------------"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f3: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f3


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f4: ------------"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f4: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f4

echo -e "\n------------ List root dir /: ------------ "
../build.linux/nachos -l /
# echo "========================================="

echo -e "\n------------ List dir /t0: ------------ "
../build.linux/nachos -l /t0
# echo "========================================="

echo -e "\n------------ Recursive List root dir /: ------------ "
../build.linux/nachos -lr /
# echo "========================================="

echo -e "\n------------ Print content /t0/f1: ------------- "
../build.linux/nachos -p /t0/f1
# echo "========================================="

echo -e "\n------------ Print content /t0/bb/f3:------------  "
../build.linux/nachos -p /t0/bb/f3


echo -e "\n------------ Test Remove() ("-r")  --------------"
# echo "============ Test Remove() ("-r") =============="
# ../build.linux/nachos -r /t0/f1
# ../build.linux/nachos -l /

echo -e "Before removing /t0/f: "
../build.linux/nachos -l /t0

../build.linux/nachos -r /t0/f1
echo -e "After removing /t0/f: " 
../build.linux/nachos -l /t0