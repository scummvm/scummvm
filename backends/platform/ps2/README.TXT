Dear PS2 ScummVM user,

here you will find the latest and the greatest PS2 backend ever!


                   *** Changelog (0.13.1) ***

- multi media : it's now not only possible to start PS2 ScummVM
  from any media you like, but even store your game data, saved
  games and themes on any media as well: DVD, HD, USB, MC, HOST.

- remote : HOST: is a special device that represents a remote
  computer on your network where you started "ps2client listen"
  from the folder that you wish to export, which likely contains
  your ScummVM games, etc.

  Yes, you got it right! You don't need to install the games
  again if you have them already installed on your PC ;-)

- shared saved : since now you can share the games with your
  other machines, it made a lot of sense to be able to share
  saved games between them and the PS2 ScummVM backend, so
  UCL has been dropped, and voila, I was eventually able to
  finish The Dig which I started many years ago on my laptop
  in London, sitting on a coach in Miami (of course I brought
  the laptop with me, but I could have done it over internet).

- RTL : on popular demand when you are quitting a game you are
  now taken back to the main game launcher when you can choose
  another game, or quit for good. So if you are stuck in a game
  and need to refresh your ideas playing another cool ScummVM
  game, you can do it without restarting the PlayStation2 and
  reloading ScummVM.

- global caching : we enabled file caching for remote file
  access and other media (before it was only used for CD/DVD)
  this should greatly enhance ScummVM feedback response.

- enhanced NTSC/PAL detection : this should work better on
  some of the latest slim PS2


                         *** Installation ***

Some media drivers require all capitalized letter, so to be on the
safe side store the files (SCUMMVM.ELF and *.IRX) in upper case.

Here comes a full list of the required *.IRX (don't worry they are all
provided pre-compiled in the binary release ;-)

 IOMANX.IRX
 FILEXIO.IRX
 CODYVDFS.IRX
 SJPCM.IRX
 USBD.IRX
 USB_MASS.IRX
 PS2MOUSE.IRX
 RPCKBD.IRX
 POWEROFF.IRX
 PS2DEV9.IRX
 PS2ATAD.IRX
 PS2HDD.IRX
 PS2FS.IRX
 PS2IP.IRX
 PS2SMAP.IRX
 PS2HOST.IRX

Make sure they are all stored in the folder that contains your SCUMMVM.ELF!

If you are starting ScummVM from CD/DVD then it will look for ScummVM.ini
on the first MC and then on CD itself. In any other case it will look for
a ScummVM.ini on the media folder from where you started SCUMMVM.ELF.


                             *** Remote ***

You will need "PS2IP.IRX", "PS2SMAP.IRX" and "PS2HOST.IRX" drivers too
in the folder where you are starting SCUMMVM.ELF if you want to use
the remote capability.

The IP 192.168.0.10 (ps2link default) is hardcoded in the source as
"netArg" in irxboot.cpp, feel free to modify it to match your network
configuration.

If you are starting SCUMMVM.ELF over the net using ps2client then you
are already set. If you are starting it in any other way, then you
will have to "export" the folder where you are storing ScummVM games
and data by starting ps2client from inside that folder. Eg. from a
terminal:

> ./ps2client -h [your ps2 ip] listen

Do this -after- ScummVM is up and running otherwise it will fail to
establish a connection.


                           *** Gamepad ***

The mapping is:

Start Button         -   GMM
R1                   -   'y'
L1                   -   'n'
L2                   -   Numpad 0
Triangle             -   Escape
Square               -   Enter
Cross                -   Left mouse button
Circle               -   Right mouse button


                            *** Bugs ***

Though we made our best to make this release as bug free as possible
there are still a few bugs left:

 - COMI GMM : in 0.13.1 is not currently possible to load & save COMI
   games, because when calling the in-game menu (GMM) a subtle stack
   crash is triggered which kisses a goodbye to the PS2 ScummVM :-(
 
   This bug might affect other games or general stability, but if you
   are using the precompiled ELF we are providing, or compiling with
   the same set of engines and options it should only affect COMI.

   Please do let us know if it affects your gaming experience in any
   other way.


 - remote ScummVM.ini : if you are storing ScummVM.ini on your HOST
   in the network and ScummVM crashes, the content of the hard-disk,
   ehm, kidding!, the content of ScummVM.ini might get corrupted.

   Please make sure you backup it once in a while.


 - file handling speed : (not really a bug, but a nuisance) : there
   is probably space for some more optimization in the current file
   management. The only case when it really bogs me it's when you
   try to select a saved game and there is some lag.

   Some game could start faster too when this is optimized.


 - feel free to add your bug, suggestion, secret wish to this list!


Now enough reading, rush to your PlayStation2 and start playing!

Are you still reading?

Have a lot of fun,
 - Max Lingua
