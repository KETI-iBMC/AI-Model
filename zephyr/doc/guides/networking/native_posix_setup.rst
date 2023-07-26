.. _networking_with_native_posix:

Networking with native_posix board
##################################

.. contents::
    :local:
    :depth: 2

This page describes how to set up a virtual network between a (Linux) host
and a Zephyr application running in a native_posix board.

In this example, the :ref:`sockets-echo-server-sample` sample application from
the Zephyr source distribution is run in native_posix board. The Zephyr
native_posix board instance is connected to a Linux host using a tuntap device
which is modeled in Linux as an Ethernet network interface.

Prerequisites
*************

On the Linux Host, fetch the Zephyr ``net-tools`` project, which is located
in a separate Git repository:

.. code-block:: console

   git clone https://github.com/zephyrproject-rtos/net-tools


Basic Setup
***********

For the steps below, you will need three terminal windows:

* Terminal #1 is terminal window with net-tools being the current
  directory (``cd net-tools``)
* Terminal #2 is your usual Zephyr development terminal,
  with the Zephyr environment initialized.
* Terminal #3 is the console to the running Zephyr native_posix
  instance (optional).

Step 1 - Create Ethernet interface
==================================

Before starting native_posix with network emulation, a network interface
should be created.

In terminal #1, type:

.. code-block:: console

   ./net-setup.sh

You can tweak the behavior of the net-setup.sh script. See various options
by running ``net-setup.sh`` like this:

.. code-block:: console

   ./net-setup.sh --help


Step 2 - Start app in native_posix board
========================================

Build and start the ``echo_server`` sample application.

In terminal #2, type:

.. zephyr-app-commands::
   :zephyr-app: samples/net/sockets/echo_server
   :host-os: unix
   :board: native_posix
   :goals: run
   :compact:


Step 3 - Connect to console (optional)
======================================

The console window should be launched automatically when the Zephyr instance is
started but if it does not show up, you can manually connect to the console.
The native_posix board will print a string like this when it starts:

.. code-block:: console

   UART connected to pseudotty: /dev/pts/5

You can manually connect to it like this:

.. code-block:: console

   screen /dev/pts/5
