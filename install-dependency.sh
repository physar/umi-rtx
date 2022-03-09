#!/bin/bash

install_debian()
{
    echo "NOTE: Make sure you have done apt-get update and apt-get upgrade prior to running this script."
    echo "Installing Debian/Ubuntu dependencies"
    echo "-------"

    sudo apt-get --yes --force-yes install gcc
    sudo apt-get --yes --force-yes install libelf-dev
    sudo apt-get --yes --force-yes install libreadline-dev
    
    echo "-------"
    echo "Done..."
}

install_redhat()
{
    echo "NOTE: Make sure you have done apt-get update and apt-get upgrade prior to running this script."
    echo "Installing RedHat/CentOS/etc dependencies"
    echo "-------"

    sudo yum -y install gcc 
    sudo yum -y install libelf-dev
    sudo yum -y install libreadline-dev

    echo "-------"
    echo "Done..."
}

get_system()
{
    if [ -e /etc/os-release ]; then
        . /etc/os-release
    fi
}

get_system
echo "You are using ${ID}"

case ${ID} in
    debian)
        install_debian
        ;;
    ubuntu)
        install_debian
        ;;
    raspbian)
        install_debian
        ;;
    redhat)
        install_redhat
        ;;
    centos)
        install_redhat
        ;;
    suse)
        install_redhat
        ;;
    *)
        echo "Your linux os ${ID} is currently unsupported."
        exit 255
        ;;
esac 
# exit 0

