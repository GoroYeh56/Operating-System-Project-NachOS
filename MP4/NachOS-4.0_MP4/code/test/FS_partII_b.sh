../build.linux/nachos -f    # format disk.
../build.linux/nachos -cp num_100.txt /100
../build.linux/nachos -cp num_1000.txt /1000	

echo -e "\n----- Recursive List after copy two files -----"

../build.linux/nachos -lr /

../build.linux/nachos -p /1000  # print file /1000
echo "========================================="
../build.linux/nachos -p /100   # print file /100
echo "========================================="
../build.linux/nachos -l /  # list directory

# -f threads/kernel.cc
# -cp, -p :printfilename, -l : at threads/main.cc