# vcu_gst_app
The vcu_gst_app is a command-line multi-threaded Linux application that uses the vcu_gst_lib interface. The command-line application requires an input configuration file (input.cfg) to be provided in the plain text

Dependencies:
vcu_gst_lib, vcu_apm_lib

# This forked version
This fork/branch hold the modifications done in order to integrate the VCU application into ours f/w.
The changes were done on the zynqmp_vcu_trd_v2021.2 branch code as used in Petalinux-2021.2 SDK.
The vcu_xxx forked branches were created in order to comply with the LGPL license - giving the changes to whoever request them.
As so, the branched were never built using their native project, but by the makefile project of ours app.
- The build was in Petlinux-2021.2
- The SDK used was the one built for ours MpSOC based board.
- The code 
here is given for reference only.

