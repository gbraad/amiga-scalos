WARNING
=======

This is not a final release of the Scalos update - it is a beta snapshot
of the work done so far. It has been running stable on our machines for
weeks but all kinds of problems might surface on your own system.

This archive is NOT FOR DISTRIBUTION on any other websites, CD-ROM etc.
without out prior permission.
It is exclusive to the www.scalos.noname.fr website. If you downloaded
this file from somewhere else, please inform us at info@scalos.noname.fr

Please inform us of problems!
Use the form at http://bugs.scalos.noname.fr, and send your reports
to
	bugs@scalos.noname.fr,
	scalos@yahoogroups.com 		mailinglist

or by posting the information in the Scalos forum at

	http://Scalos.noname.fr/forums/index.php

Try to provide as much information about your configuration as possible
- *THIS IS IMPORTANT*.

General:
CPU, Graphics system, screen depth, enforcer/cyberguard hit log.

Specific:
Scalos prefs such as bob system type, what you have checked in
the "misc." options of the scalos prefs program etc. Anything you think
could be related to your problem.

Also, provide some means for us to contact you if the
bug is hard to re-create at our end also.


INSTALLATION
============

An installer script is included in the archive, and should install all
required files to the appropriate placesIf you are installing Scalos for
the first time, a default configuration is installed, too.

After installation, reboot and test away!

Due to unclear licensing conditions, we can not include the original Amiga
installer in this archive. Especially MorphOS users may not have Installer
available on their machines. They can get it from here:

http://main.aminet.net/util/misc/Installer-43_3.lha

Here you can find some of the non-standard MUI MCCs required for Scalos Prefs:

http://main.aminet.net/dev/mui/MCC_Urltext.lha
http://main.aminet.net/dev/mui/MCC_Lamp.lha
http://vapor.meanmachine.ch/voyager/pophotkey_1715.lha


If anything else is missing, do not hesitate to contact us
at info@scalos.noname.fr !


If you display "%pr" processor information in the screen title bar, you
will need to remove the prepended "68" to the string. For PPC support,
we had to change the string returned by "%pr", so for an 68040 processor,
now "68040" is returned instead of "040".

If you use non-English Scalos catalog, then please make sure you are using
the latest language catalog from the website. Currently, only the French
and German catalogs are continously kept up-to-date.


Installation on OS3.0 or OS3.1 machines
---------------------------------------
We strongly recommend that you install the improved ilbm.datatype from
Aminet, or a more recent version:

http://aminet.net/util/dtype/ilbmdt44.lha




HISTORY
=======

See the separate combined "History" file. It contains both main Scalos, 
prefs programs, and datatype sub-system history.

To find out what has been updated since this release please consult the
version history section of http://news.scalos.noname.fr


CREDITS
=======
Popupmenu.library has been included by permission of
  the author Henrik Isaksson.

Scalos image dithering code has been taken from libppm:

 Copyright (C) 1989, 1991 by Jef Poskanzer.

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted, provided
 that the above copyright notice appear in all copies and that both that
 copyright notice and this permission notice appear in supporting
 documentation.  This software is provided "as is" without express or
 implied warranty.


Video preview plugin uses libavcodec, libavformat, and libavutil which
are distributed under GNU LESSER GENERAL PUBLIC LICENSE.
Required source code according to LGPL is available for download
from our web site.


thanks!

The Scalos Team
[info@scalos.noname.fr]
