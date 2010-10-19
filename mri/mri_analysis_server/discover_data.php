<html>
<head>
<title>Varian FID Reconstruction</title>
<link rel="stylesheet" href="styles.css" type="text/css">
</head>
<body bgcolor=#FFFFFF>
<?
   //
   //   Header:
   //
?>
<table border=0 cellpadding=3 cellspacing=0 width=100%>
<tr style="backgroundColor=#FFFFFF"><td>
<a href="http://tesla.cdfi.uab.edu:8200"><img border=0 src="logo.gif"></a>
</td><td align=right><div class=banner>Varian FID Reconstruction</div></td>
</tr></table>

<!--<hr color=#8caccb size=1>-->
<br>
<div class=nav>
<a href="index.php">View Studies</a>
</div><br>

<noscript>
  <div align=center><font size=+2 color=#D00000>Please enable javascript!</font></div>
</noscript>

<?
  $curdir = '/mnt/tesla/public/mark/';
  if (isset($_GET["curdir"]))
     $curdir = $_GET["curdir"];
  if (substr($curdir,strlen($curdir)-1,1) != "/")
     $curdir .= "/";

  /* CHANGE DIRECTORY FORM BELOW */ 
?>
<table width=99% border=0 align=center>
<tr><td>
<form name="chdir" method="get" action="discover_data.php">
<b>View a different directory:&nbsp;&nbsp;</b>
  <input type="text" name="curdir" size=40 value="<? echo $curdir; ?>">
  <input type="submit" value="update">
</form>
<font size="-1">
<u>How this works:</u>  The directories listed below have a study.xml file in them.  Although this file isn't used anymore (as of 10/31/2006), the presence of the file is used to identify study directories.  After a directory is selected, all runs of any kind found in the "data" subdirectory are loaded, as long as a valid 'procpar' file is found in the .fid directory.  Directory names for each run should be in the form: sequence name, underscore ("_"), run number, ".fid" (e.g. "epidw_01.fid" or "T1map2new_07.fid") in order to be recognized.
</font>
</td></tr>

<tr><td>
<div class=input1>

<table border=0 cellpadding=2 cellspacing=0 width=99%><tr height=36px><td width=35px>
  <img src="mag_glass.gif" height=32></td><td>&nbsp;&nbsp;<font size=+1><b>Discover data:</b></font></td><td align=center>
</td>
</tr>
<tr><td colspan=3><hr size=1></td></tr>
<tr><td colspan=3>
<?
  $dh = @opendir($curdir);
  while (false !== ($file = readdir($dh)))
  {
    if (substr($file,0,1) != "." && is_dir($curdir.$file))
    {
      if (file_exists($curdir.$file."/study.xml"))
        $files[] = array(filemtime($curdir.$file),$file);
      else
        $otherdirs[] = $file;
    }
  }
  closedir($dh);
  if ($files)
  {
    rsort($files);
    echo "<div style=\"font-family:veranda,sans-serif;\">\n";
    echo "<li><b>studies in:&nbsp;&nbsp;&nbsp;".$curdir."</b>&nbsp;&nbsp; - &nbsp;&nbsp;<font color=red>Select a link to load all experiments in that directory</font><br><font size=-1>&nbsp;&nbsp;&nbsp;<i>(note: to be recognized as a Varian study the directory must contain a 'study.xml' file.)</i></font>\n";
    echo "<table border=0  cellpadding=3 width=99%>\n";
    foreach ($files as $file)
    {
      echo "<tr><td>&nbsp;&nbsp;".date("M j, Y - g:i a  ",$file[0])."&nbsp;&nbsp;</td>\n";
      echo "<td><a href=\"discover_this_DIR.php?fulldir=".$curdir.$file[1]."\">$file[1]</a></td></tr>\n";
    }
    echo "</table></div>\n";
  }
  else
  {
    echo "<font color=red>Either there are no experiments in this directory, or the directory could not be read.</font><br>";
  } 

  echo "<br><hr><br>\n";

  if ($otherdirs)
  {
    echo "<div style=\"font-family:veranda,sans-serif;\">\n";
    echo "<li><b>other subdirectories of &nbsp;&nbsp;".$curdir."</b>&nbsp;&nbsp; - &nbsp;&nbsp;<font color=red>Select a link to look in that subdirectory</font>\n";
    echo "<table border=0 cellpadding=3 cellspacing=0 width=99%>\n";
    echo "<tr>\n";
    $i = 1;
    foreach ($otherdirs as $subdir)
    {
      echo "<td><a href=\"discover_data.php?curdir=".$curdir.$subdir."\">".$subdir."</a></td>\n";
      $i = $i + 1;
      if ($i > 3) {
        $i = 1;
        echo "</tr><tr>\n";
      }
    }
    echo "</tr></table>\n";
  }
?>
</td></tr>
</table>

</td>
</tr>
</table>

</body>
</html>
