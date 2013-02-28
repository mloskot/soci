#!/bin/bash
# Run before_intsall actions for SOCI build at travis-ci.org
# Mateusz Loskot <mateusz@loskot.net>, http://github.com/SOCI
#
set -e
if [[ "$TRAVIS" != "true" ]] ; then
	echo "Running this script makes no sense outside of travis-ci.org"
	exit 1
fi
tmstamp() { echo -n "[$(date '+%H:%M:%S')]" ; }

# Install dependencies
echo "$(tmstamp) before_intsall::apt-get starting $(date)"
sudo bash -c 'echo "deb https://oss.oracle.com/debian unstable main non-free" >> /etc/apt/sources.list'
sudo apt-get update -qq
sudo apt-get install -qq \
	libboost-dev libboost-date-time-dev \
	libmyodbc unixodbc-dev odbc-postgresql \
	firebird2.5-super firebird2.5-dev \
	oracle-xe-client
echo "$(tmstamp) before_intsall::apt-get finished $(date)"

# Configure Firebird
# See: Non-interactive setup for travis-ci.org 
# http://tech.groups.yahoo.com/group/firebird-support/message/120883
#sudo dpkg-reconfigure -f noninteractive firebird2.5-super
sudo sed /ENABLE_FIREBIRD_SERVER=/s/no/yes/ -i /etc/default/firebird2.5
cat /etc/default/firebird2.5 | grep ENABLE_FIREBIRD_SERVER
sudo service firebird2.5-super start
#
# Configure ODBC
sudo odbcinst -i -d -f /usr/share/libmyodbc/odbcinst.ini
