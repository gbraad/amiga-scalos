<?php
    // Define variables
    $page_title = 'Scalos - About';
    $page_nav_doc = 'about';

    // Header(s)
    include('header.inc');
    include('header_script.inc');
?>

<!-- About Scalos -->
<br>
<?php make_title('about', 0, 'about.gif', 390, 30, 'About Scalos development'); ?>
<br>
<p>As of 23rd November 1999, we took over development of Scalos, the Amiga Workbench desktop replacement. This followed a release by Stefan Sommerfeld stating that he would like a group of people with 
the right talents, abilities and resources to continue development. We are that group of people and we're gonna make Scalos the best Amiga Workbench desktop replacement there is!</p><br>

<!-- What is Scalos -->
<?php make_title('whatis', 1, 'whatis.gif', 230, 30, 'What is Scalos?'); ?>

<p>First of all, Scalos is <b>NOT</b> a replacement OS, although the name suggests otherwise. It is a desktop replacement for the Amiga. Want to see some of the features of Scalos? Simply 
<a href="features.php" target="mainframe">click here</a>. It is used instead of <a href="http://www.amiga.com/3.5/" target="_blank">Workbench</a> or 
<a href="http://www.gpsoft.com.au" target="_blank">Directory Opus Magellan</a>.<br><br>
The original Workbench gets older every day and is being left behind by other desktop systems. Scalos's original author, Stefan Sommerfeld, decided that there had to be something new, and so Scalos 
was born.<br><br>
Scalos's goal is to integrate a lot of additional functionality and an enhanced look without losing any of the look-and-feel we all love about the real Workbench. Yes, Scalos is like Directory Opus 
Magellan, but comparing these two isn't really fair. Scalos provides much better emulation of the real Workbench (&quot;the Workbench experience&quot;), and you won't notice any difference at first 
appearance. Because of this, we can't include a screenshot (look at your current Workbench for one!). Up to now Directory Opus Magellan just provides more functions.</p>
<?php back_to_top(); ?>

<!-- History of Scalos -->
<?php make_title('histscalos', 1, 'scaloshistory.gif', 280, 30, 'The History of Scalos'); ?>

<p>Scalos has been available to Amiga users for many years. It was originally written by Stefan Sommerfeld of ALiENDESiGN. His aim was to create an expandable, functional replacement for Workbench 
without losing any of the style and grace that we love about the Amiga. Stefan has since passed the reins over to us. We are now developing this exceptional software. Stay tuned!<br><br>
Some interesting info on why it's called Scalos... It was named after a planet from the original series of Star Trek (TM &amp; Copyright respected). Check out these links:<br>
<a href="http://www.ufp-terminal.de/ufp_terminal/geography/planets/scalos.html" target="_blank">http://www.ufp-terminal.de/ufp_terminal/geography/planets/scalos.html</a><br>
<a href="http://www.geocities.com/ussmunchkin/Tos67.htm" target="_blank">http://www.geocities.com/ussmunchkin/Tos67.htm</a><br>
<a href="http://trekweb.com/tos/missions/068.html" target="_blank">http://trekweb.com/tos/missions/068.html</a></p>
<?php back_to_top(); ?>

<!-- Registered users -->
<?php make_title('registered', 1, 'registered.gif', 400, 30, 'What about registered users?'); ?>

<p>Current registered users will be able to use upgrades to V1.x for free from us. V2.x will require some sort of upgrade fee - but this will include BIG changes and additional software with it, 
hopefully. We have no idea of how long this could take!<br><br>
In V1.x updates, we plan to kill the bugs as soon as possible, but we also plan to add as many new, exciting, useful features as we can. Please see the <a href="features.php">features</a> page for 
more details.</p>
<?php back_to_top(); ?>

<!-- The PowerPC -->
<?php make_title('powerpc', 1, 'powerpc.gif', 350, 30, 'What about the PowerPC?'); ?>
<br><br>

<table cellspacing="0" cellpadding="0" border="0" summary="PowerPC">
  <tr>
    <td width="105" valign="top" align="right"><img src="gfx/misc/ppcchip.gif" width="100" height="115" alt="PowerPC chip"></td>
    <td width="100%">
      <p>We are considering adding PPC support for V2.x of Scalos. However, we need to see how much this will affect the performance. If it makes only a slight improvement, we'll <b>STILL</b> do it. 
      At least it then depends on others to make it even faster by porting the 68k libraries to PPC (both the OS and third-party ones). If it reduces performance due to the 68k&lt;-&gt;PPC context 
      switches, then we'll shelve the idea. Depending on how we go, we will either use the <a href="http://ftp.plig.org/pub/aminet/dev/asm/PPC680x0V1.10.readme" target="_blank">68k-&gt;PPC Assembler</a> 
      (as seen on Aminet) -or- work on converting the source code into 'C' and then compile it using VBCC for <a href="http://www.haage-partner.com/wu_e.htm" target="_blank">WarpUP</a> only!<br><br>
      If H&amp;P don't achieve a native PPC implementation of the Amiga OS ROM libraries, we will also hope to support the PPC MorphOS project. This system runs natively on the PPC and allows existing 
      68k libraries to be emulated on the PPC. The 68k is not used at all. Please visit the <a href="http://www.morphos.de/" target="_blank">MorphOS website</a> for more info.</p>
    </td>
  </tr>
</table><br>

<?php
    include('footer.inc');
?>