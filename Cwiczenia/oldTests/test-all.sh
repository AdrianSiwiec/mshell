make clean -C ../shell
make -C ../shell


echo -e "\nTesting 2\n"
cd C02/test1
./prepare.sh /tmp/Test1
./gen_all.sh /bin/bash /tmp/Test1 #Only in linux
./run_all.sh ../../../shell/mshell /tmp/Test1
./clean.sh /tmp/Test1
cd ../..

echo -e "\nTesting 3\n"
cd C03/test3
./prepare.sh /tmp/Test1
./gen_all.sh /bin/bash /tmp/Test1 #Only in linux
./run_all.sh ../../../shell/mshell /tmp/Test1
./clean.sh /tmp/Test1
cd ../..

echo -e "\nTesting 4\n"
cd C04/test4
./prepare.sh /tmp/Test1
./gen_all.sh /bin/bash /tmp/Test1 #Only in linux
./run_all.sh ../../../shell/mshell /tmp/Test1
./clean.sh /tmp/Test1
cd ../..

echo -e "\nTesting 5\n"
cd C05/test5
./prepare.sh /tmp/Test1
./gen_all.sh /bin/bash /tmp/Test1 #Only in linux
./run_all.sh ../../../shell/mshell /tmp/Test1
./clean.sh /tmp/Test1
cd ../..
