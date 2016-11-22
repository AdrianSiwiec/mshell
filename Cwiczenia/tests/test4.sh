make clean -C ../shell
make -C ../shell


echo -e "\nTesting 4\n"
cd C04/test4
./prepare.sh /tmp/Test1
./gen_all.sh /bin/bash /tmp/Test1 #Only in linux
./run_all.sh ../../../shell/mshell /tmp/Test1
./clean.sh /tmp/Test1
cd ../..
