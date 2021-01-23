echo -e "=========== Testing FS_bonusIII.sh... ===========\n"
echo -e "\n----------- Formatting disk... ------------\n"
../build.linux/nachos -f

echo -e "\n----------- Mkdir /t0------------\n"
../build.linux/nachos -mkdir /t0

echo -e "\n----------- Mkdir /t1 ------------\n"
../build.linux/nachos -mkdir /t1
echo -e "\n----------- Mkdir /t2 ------------\n"
../build.linux/nachos -mkdir /t2

echo -e "\n----------- Recursive List  to check root dir ------------\n"
../build.linux/nachos -lr /

echo -e "\n----------- Test copy file ------------\n"

echo -e "\------------ Copy file num_100.txt to nachOS FS /t0/f1: ------------\n"
echo -e "Copy file num_100.txt to nachOS FS /t0/f1: \n"
../build.linux/nachos -cp num_100.txt /t0/f1

echo -e "\n----------- Mkdir /t0/aa ------------\n"
../build.linux/nachos -mkdir /t0/aa

echo -e "\n----------- Mkdir /t0/bb ------------\n"
../build.linux/nachos -mkdir /t0/bb

echo -e "\n----------- Mkdir /t0/cc ------------\n"
../build.linux/nachos -mkdir /t0/cc


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f1: ------------\n"
echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f1: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f1


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f2: ------------\n"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f2: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f2


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f3: ------------\n"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f3: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f3


echo -e "\n------------ Copy file num_100.txt to nachOS FS /t0/bb/f4: ------------\n"
# echo -e "Copy file num_100.txt to nachOS FS /t0/bb/f4: \n"
../build.linux/nachos -cp num_100.txt /t0/bb/f4

echo -e "\n------------ Recursive List root dir /: ------------ \n"
../build.linux/nachos -lr /

echo -e "\n------------ Recursive Remove /t0:  --------------\n"
../build.linux/nachos -rr /t0

echo -e "\n------------ After Recursive Remove /t0: --------------\n" 
echo "The root dir woule be:"
../build.linux/nachos -lr /