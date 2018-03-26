#! /bin/bash

git clone https://github.com/qextserialport/qextserialport.git temp
cd temp
qmake-qt4
echo "Installing ... "
make > /dev/null
sudo make install > /dev/null
echo "Installed"
cd ..
rm -rf ./temp
