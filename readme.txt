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

http://aminet.net/util/misc/Installer-43_3.lha

Here you can find some of the non-standard MUI MCCs required for Scalos Prefs:

http://main.aminet.net/dev/mui/MCC_Urltext.lha
http://main.aminet.net/dev/mui/MCC_Lamp.lha
http://vapor.meanmachine.ch/voyager/pophotkey_1715.lha
http://jabberwocky.amigaworld.de/downloads.php?id=30


If anything else is missing, do not hesitate to contact us
at info@scalos.noname.fr !


If you display "%pr" processor information in the screen title bar, you
will need to remove the prepended "68" to the string. For PPC support,
we had to change the string returned by "%pr", so for an 68040 processor,
now "68040" is returned instead of "040".

If you use non-English Scalos catalog, then please make sure you are using
the latest language catalog from the website. Currently, only the French
and German catalogs are continously kept up-to-date.


Installation on MorphOS 2.x machines
------------------------------------
Dragged icons will not display correctly unless both "Allow double
buffered screens" and "Allow triple buffered screen" in the
"Display Engine" preferences is disabled!


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
-------------------------------------------------------------------------------
Scalos image dithering code has been taken from libppm:

 Copyright (C) 1989, 1991 by Jef Poskanzer.

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted, provided
 that the above copyright notice appear in all copies and that both that
 copyright notice and this permission notice appear in supporting
 documentation.  This software is provided "as is" without express or
 implied warranty.
-------------------------------------------------------------------------------
Video preview plugin uses libavcodec, libavformat, and libavutil which
are distributed under GNU LESSER GENERAL PUBLIC LICENSE.
Required source code according to LGPL is available for download
from our web site.
-------------------------------------------------------------------------------
Updater.module uses libCURL.

Copyright (c) 1996 - 2009, Daniel Stenberg, <daniel@haxx.se>.
 
All rights reserved.
 
Permission to use, copy, modify, and distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
 
Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.
-------------------------------------------------------------------------------
Updater.module uses AmiSSL.


AmiSSL v1 copyright Â© 1999-2006 Andrija Antonijevic.
AmiSSL v2/v3 copyright Â© 2002-2006 Andrija Antonijevic and Stefan Burstroem.
AmiSSL is based on OpenSSL which is in turn based on SSLeay. AmiSSL is freeware.

AmiSSL IS PROVIDED "AS IS" AND ANY WARRANTIES, EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AmiSSL AUTHORS OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This product includes cryptographic software written by Eric Young (eay@cryptsoft.com). This product includes software written by Tim Hudson (tjh@cryptsoft.com).

Here are the licences for OpenSSL and SSLeay:


OpenSSL license
---------------

Copyright (c) 1998-1999 The OpenSSL Project.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

3. All advertising materials mentioning features or use of this
   software must display the following acknowledgment:
   "This product includes software developed by the OpenSSL Project
   for use in the OpenSSL Toolkit. (http://www.openssl.org/)"

4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
   endorse or promote products derived from this software without
   prior written permission. For written permission, please contact
   openssl-core@openssl.org.

5. Products derived from this software may not be called "OpenSSL"
   nor may "OpenSSL" appear in their names without prior written
   permission of the OpenSSL Project.

6. Redistributions of any form whatsoever must retain the following
   acknowledgment:
   "This product includes software developed by the OpenSSL Project
   for use in the OpenSSL Toolkit (http://www.openssl.org/)"

THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

This product includes cryptographic software written by Eric Young
(eay@cryptsoft.com).  This product includes software written by Tim
Hudson (tjh@cryptsoft.com).

Original SSLeay License
-----------------------

Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
All rights reserved.

This package is an SSL implementation written
by Eric Young (eay@cryptsoft.com).
The implementation was written so as to conform with Netscapes SSL.

This library is free for commercial and non-commercial use as long as
the following conditions are aheared to.  The following conditions
apply to all code found in this distribution, be it the RC4, RSA,
lhash, DES, etc., code; not just the SSL code.  The SSL documentation
included with this distribution is covered by the same copyright terms
except that the holder is Tim Hudson (tjh@cryptsoft.com).

Copyright remains Eric Young's, and as such any Copyright notices in
the code are not to be removed.
If this package is used in a product, Eric Young should be given attribution
as the author of the parts of the library used.
This can be in the form of a textual message at program startup or
in documentation (online or textual) provided with the package.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
   "This product includes cryptographic software written by
    Eric Young (eay@cryptsoft.com)"
   The word 'cryptographic' can be left out if the rouines from the library
   being used are not cryptographic related :-).
4. If you include any Windows specific code (or a derivative thereof) from
   the apps directory (application code) you must include an acknowledgement:
   "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"

THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

The licence and distribution terms for any publically available version or
derivative of this code cannot be changed.  i.e. this code cannot simply be
copied and put under another distribution licence
[including the GNU Public Licence.]
-------------------------------------------------------------------------------

thanks!

The Scalos Team
[info@scalos.noname.fr]
