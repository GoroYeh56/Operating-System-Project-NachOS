echo "======== Testing BonusI.sh ============="
../build.linux/nachos -f


echo -e "\n----- Mkdir /dir1 -----"
../build.linux/nachos -mkdir /dir1

echo -e "\n----- Before copy num_1000000.txt: -----"
../build.linux/nachos -l /

echo -e "\n----- After copy num_1000000.txt to /dir1/bonus1  -----"

../build.linux/nachos -cp num_1000000.txt /dir1/bonus1
../build.linux/nachos -lr /

echo -e "\n----- Print the content in bonus1 -----"
../build.linux/nachos -p /dir1/bonus1

echo -e "\n----- Done testing FS_bonusI.sh -----"

