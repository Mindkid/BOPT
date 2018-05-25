#
# This is the script to run
# to install all necessary tools.
# but first please clone the repos
# to thiis folder 
#

# PMDK
# https://github.com/pmem/pmdk.git

# NDCTL
# https://github.com/pmem/ndctl.git

# JSON-C
# https://github.com/json-c/json-c.git

# LIBREFABRIC
# https://github.com/ofiwg/libfabric.git  

# KMOD
# https://github.com/lucasdemarchi/kmod.git

#
# This are some depencies that are not 
# specified on the github pages but
# are necessary (Probably they are more)
#

sudo apt-get install autoconf
sudo apt-get install pkg-config
sudo apt-get install uuid-dev


for D in `find -P .  -maxdepth 1 ! -path . -type d`
do
	cd D
	./autogen.sh
	./configure
	make && sudo make install
done


