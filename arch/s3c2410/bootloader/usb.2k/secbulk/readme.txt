Installing the Driver
---------------------

1.) Make an 'OEM installation diskette' or equivalent installation 
 source directory for Bulkusb.sys by copying Bulkusb.sys and Bulkusb.inf into it.
 The sample BulkUsb.Inf is in the DDK BulkUsb\Sys source directory. 

2. ) Make sure your device has been programmed with the device VID/PID in
 the Bulkusb.inf file. If not, edit the device VID, PID, and description 
 text to match your test board/device.

3.) Depending on the operating system you are using:

On Windows NT 5.0:

  When you plug in the device, the "Found New Hardware Wizard" dialog box will appear with
  the subheading "Install Hardware Device Drivers". Hit the radio button labelled "Search
  for a suitable driver for your device (Recommended)" and then hit the "Next" button.
  The following screen has you specify your installation source floppy or directory. 
  Do so, then hit the "Next" button. The next screen should indicate that Windows NT
  is ready to install the driver.  Near the middle of the box, you should see the full
  source path to BulkUsb.Inf.  Hit the "Next" button. You'll briefly see a "copying Files"
  message box, then once again the "Found New Hardware Wizard" box, now displaying the subheading,
  "Hardware Install: The hardware installation is complete". Hit the "Finish" button.
  You should now have a copy of BulkUsb.Sys in your \System32\Drivers directory, a BulkUsb.Inf 
  in your \Winnt\Inf directory, and a newly-created BulkUsb.Pnf file, which is a precompiled 
  setup info file that NT creates.  If the final "Add New Hardware Wizard" box indicates any error, 
  or if the OS says you must reboot to finish  installation of this device, something has gone wrong.
  Check your Inf file, Install directory, or driver  code, follow the instructions in the below 
  section on simulating a 'first-time' install, and start over.
 
On Win98: 

  Plug in the device.  The "Add New Hardware Wizard" dialog box will appear, indicating that  
  "This wizard searches for new drivers for: [your deviceName as programmed into your board's hardware].
  Hit the "Next" button. On the following screen, hit the radio button labelled "Search for the best
  driver for your device (Recommended)" . Hit the "Next" button.  The next screen has you specify your
  installation source floppy or directory.  Do so, then hit the "Next" button.  The next screen should 
  indicate that Windows is ready to install the driver. You should see the full source path to BulkUsb.Inf
  under the label:   "Location of Driver".  Hit the "Next" button.  You will see the 
  "Building Driver Information Database" message box, then, if the  installation and loading of the
  driver have succeeded, you will see the final wizard box  saying " Windows has finished installing 
  the software that your new hardware device requires."  Hit the "Finish" button.  If the final
  "Add New Hardware Wizard" box indicates any error, or if the OS says you must reboot to finish 
  installation of this device, something has gone wrong. Check your Inf file, Install directory,
  or driver  code, follow the below instructions on simulating a 'first-time' install, and start over.


Updating the Driver
-------------------

To install a new version of your driver  after a successful initial installation, 
simply replace the binary in \System32\Drivers. If the  initial or last installation 
failed for any reason, follow the below instructions on simulating a 'first-time' install.

Simulating a 'First-time' Install 
---------------------------------

If you want to test Inf file or installation program modifications, 
or if your first install failed for any reason and you need to do, 
in effect, a new 'first-time'  driver installation without reinstalling
 a fresh Win98 or NT5, do the following:

1) Delete BulkUsb.inf from the \Windows\Inf  or \Winnt\Inf directory. On Winnt,
   also delete BulkUsb.PNF from the \Winnt\Inf directory.

2) Delete BulkUsb.Sys from the \System32\Drivers directory.

3) Using RegEdit on Win98, or RegEdt32 on NT5, purge the registry of the following driver information:

For Windows 98: 
	Delete the following registry key:
	\LocalMachine\System\Enum\USB\[the key with your device ID/PID], 
        ( in the case of our sample as published, this would be:
        \LocalMachine\System\Enum\USB\VID_045E&PID_930A )

For Windows NT 5.0:
	Delete the following registry key:
	\LocalMachine\System\CurrentControlSet\Enum\USB\[the key with your deviceId/PID]
        On NT5, you must restart the system before reinstalling the driver; this is not neccesary on Win98.


BulkUsb.Inf is in the Sys subdirectory.

It allows you to set registry overrides for:

1) "DebugLevel" debug verbosity level, where 0 == no debug output, 1 == default ,
higher == more verbose. 

2) "MaximumTransferSize" The 'chunk size' ( default 4k ) large IO requests are broken up into by the driver.









