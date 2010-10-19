<?
//
// * index_study.php:  this is a new version of index_fids.php
//   that organizes study by experiment type (gems, epi, T1map, etc.)
//   and provides a way to view images, create and download
//   files and 3dcalced versions of files, to get 3dToutCount info,
//   etc....
// * When it works it will probaby replace index_fids as the new
//   main way to view a Study.
// * 2005 Dec 09 - Jon Grossmann
//
require("shared.inc");
?>
<html>
<head>
<title>Varian FID Reconstruction</title>
<link rel="stylesheet" href="styles.css" type="text/css">
</head>
<body bgcolor=#FFFFFF onload="javascript:body_load();">
<script language="javascript">
<? 
   $bgs = array("#d0d0d0","#e0e0e0");

   if (isset($_REQUEST["errmsg"])) {
    echo "alert(\"".$_REQUEST["errmsg"]."\");\n";
    if (isset($_REQUEST["back"]) && $_REQUEST["back"]==1)
      echo "window.back();";
  }
?>
function body_load()
{
  document.main_form.new_comment.focus();
}
</script>
<?
  $studyid = $_REQUEST["studyid"];  // TODO: exit this page earlier if !isset($_GET["studyid"])
  $studyidmod = str_replace("/", "_", $studyid);

  $id = 0;  // id of fid row to view/process (if any)
  if (isset($_GET["id"])) $id = $_GET["id"];
  // if $id is set, then $processrow will later be set to the row in fid_files with this id
  $processrow = array();

  $other_id = 0;
  if (isset($_GET["other_id"])) $other_id = $_GET["other_id"];
  $processotherrow = array();

  $editcomment = false;
  if (isset($_GET["editcomment"]) && $_GET["editcomment"]==1)
    $editcomment = true;

  $viewwhat = "";
  if (isset($_GET["view"]))  $viewwhat = $_GET["view"];

   //
   //   Header:
   //
?>
<table border=0 cellpadding=3 cellspacing=0 width=100%>
<tr style="backgroundColor=#FFFFFF"><td>
<a href="http://tesla.cdfi.uab.edu"><img border=0 src="logo.gif"></a>
</td><td align=right><div class=banner>Varian FID Reconstruction</div></td>
</tr></table>

<!--<hr color=#8caccb size=1>-->
<div class=nav>
<a href="index.php">view studies</a>&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
<a href="studies/<? echo $studyidmod; ?>/"><font color=#CCCCF8>view directory</font></a>
<? /* <a href="upload_fid.php">Upload Fid</a> */ ?>
</div>

<script language="javascript">

  var inorout = 0;

  function edit_comment(whichrow)
  {
    document.edit_comment_form.id.value = whichrow;
    document.edit_comment_form.submit();
  }
  function save_comment()
  {
    document.save_comment_form.new_comment.value = document.main_form.new_comment.value;
    document.save_comment_form.submit();
  }
  function select_row(whichrow)
  {
    if (inorout == 1) {
      inorout = 0;
      return;
    }
    document.editfid.id.value = whichrow;
    document.editfid.submit();
  }
  function select_other_row(whichrow)
  {
    if (inorout == 1) {  // don't remember why this is here, maybe can remove?
      inorout = 0;
      return;
    }
    document.editfid.other_id.value = whichrow;
    document.editfid.submit();
  }
  function delete_row(whichrow)
  {
    var ans = confirm('Are you sure you want to delete this fid?');
    if (ans == false)
      return;
    document.deletefid.id.value = whichrow;
    document.deletefid.submit();
  }
  function delete_other_row(whichrow)  <? /* delete entry from 'other_files' table */ ?>
  {
    var ans = confirm('Are you sure you want to delete this file?');
    if (ans == false)
      return;
    alert('1');
    document.deletefid.other_id.value = whichrow;
    alert('2');
    document.deletefid.submit();
    alert('3');
  }
  function delete_feat(whichrow)
  {
    if (!confirm('Delete this feat report?')) return;
    document.deletefeat.feat_id.value = whichrow;
    document.deletefeat.submit();
  }
  function recon(whichrow, recon_action)
  {
    document.recon_form.id.value = whichrow;
    document.recon_form.recon_action.value = recon_action;
    document.recon_form.submit();
  }
  function recon_other(whichrow, recon_action)
  {
    document.recon_form.other_id.value = whichrow;
    document.recon_form.recon_action.value = recon_action;
    document.recon_form.submit();
  }
</script>
<noscript>
  <div align=center><font size=+2 color=#D00000>Please enable javascript!</font></div>
</noscript>
<script language="javascript">
//  setPointer function borrowed and crapified from phpMyAdmin/libraries/functions.js
/**
 * Sets/unsets the pointer and marker in browse mode
 *
 * @param   object    the table row
 * @param   integer  the row number
 * @param   string    the action calling this script (over, out or click)
 * @param   string    the default background color
 * @param   string    the color to use for mouseover
 * @param   string    the color to use for marking a row
 *
 * @return  boolean  whether pointer is set or not
 */
function setPointer_hide(a,b,c) { return; }
function setPointerTD(thetd, theaction, thecolor)
{
  if (theaction == "over") {
    inorout = 1; }
  else {
    inorout = 0; }
  thetd.setAttribute('bgcolor',thecolor,0);
  //var theCells = thetd.parent.getElementsByTagName('td');
  //var rowCellsCnt  = theCells.length;
  //var c = null;
  //for (c = 0; c < rowCellsCnt; c++) {
  //  theCells[c].setAttribute('bgcolor',thecolor,0); }
}
function setPointer(theRow, theAction, theDefaultColor, thePointerColor)
{
    if (inorout == 1) {
      thePointerColor = '#EEBBBB';
    }

    var theCells = null;

    // 1. Pointer and mark feature are disabled or the browser can't get the
    //    row -> exits
    if (thePointerColor == '' || typeof(theRow.style) == 'undefined') {
        return false;
    }

    // 2. Gets the current row and exits if the browser can't get it
    if (typeof(document.getElementsByTagName) != 'undefined') {
        theCells = theRow.getElementsByTagName('td');
    }
    else if (typeof(theRow.cells) != 'undefined') {
        theCells = theRow.cells;
    }
    else {
        return false;
    }
    var newColor     = null;
    var rowCellsCnt  = theCells.length;
    if (theAction == 'over') { newColor = thePointerColor; }
    if (theAction == 'out')  { newColor = theDefaultColor; }
    // 5. Sets the new color...
    if (newColor) {
        var c = null;
            for (c = 0; c < rowCellsCnt; c++) {
                theCells[c].setAttribute('bgcolor', newColor, 0);
            } // end for
    } // end 5

    return true;
} // end of the 'setPointer()' function
</script>

<?
   //
   //   Reconstructed FIDs List:
   //
?>
<table width=99% border=0 align=center>
<tr><td>
<div class=input1>

<table border=0 cellpadding=2 cellspacing=0><tr height=36px><td width=35px valign=top>
  <img src="fid.gif" <?/*"monkey.gif" height=32*/?>></td><td>&nbsp;&nbsp;<font color=000055><b><? echo $_GET["studyid"]; ?></b>&nbsp;:&nbsp;</font>
  <? /* &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Subject:&nbsp;&nbsp;&nbsp;<font color=800000> */ ?>
<?
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

  $study = read_study_table($_GET["studyid"]);

  $editon = 0;
  if (isset($_GET["edit"]) && $_GET["edit"] == 1) $editon = 1;
  $editoff = 0;
  if (isset($_GET["edit"]) && $_GET["edit"] == 0) $editoff = 1;
  // edit if the desc or subj is blank, but only if user did not say to cancel changes
  // OR edit if clicked Edit
  if ((!$editoff && (!strlen($study["description"]) || !strlen($study["subject"])))
      || $editon)
  {
?>
  <br>
  <div style="margin-left:8px; margin-top:5px; margin-bottom:5px; border-style:solid; border-color:#008000; padding:3px; width:100%; border-radius:4pt; background-color:#DDEEDD">
  <form name="study_info" action="update_studies_table_info.php" method="post">
    <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
    <table cellspacing=1 cellpadding=1 border=0>
    <tr><td>
    Subject: </td><td><select name="subject">
<?
  $subjects = get_subject_list();
  foreach ($subjects as $subjname)
  {
     echo "<option value=\"".$subjname."\"";
     if ($subjname == $study["subject"]) echo " selected=1";
     echo ">".$subjname;
  }
?>
  </select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;or new subject:&nbsp;<input type="text" name="newsubject" size=11>
  <!-- &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Type of study: <select name="type_of_study">
   <option value="functional">functional<option value="structural">structural</select> -->
   </td><td align=right>
   <input type="button" value=" Save Changes " onClick="javascript:document.study_info.submit()">
   <input type="button" value=" Cancel Changes " onClick="javascript:document.cancel_changes.submit()">
   </td></tr>
   <tr><td>
   Description: </td><td colspan=2>
      <input type="text" name="description" size=80 style="padding:1px; background-color:#FFFFFF" value="<? echo $study["description"];  ?>">
   </td></tr>
   <tr><td valign=top>
   Notes: </td><td colspan=2><textarea name="notes" rows=6 cols=80><? echo $study["notes"]; ?></textarea>
   </td></tr>
   </table>
  </form>
  </div>
<?
  }
  else // else of:  if _get[edit]==1
  {
    echo "&nbsp;&nbsp;";
    echo "<b><span style=\"color:#990000\">".$study["description"]."</span></b>\n";
    echo "&nbsp;&nbsp;&nbsp;&nbsp;";
    echo "subject = ".$study["subject"]; if (!strlen($study["subject"])) echo " ? ";
    echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    echo "<a href=\"index_study.php?studyid=".$_GET["studyid"]."&edit=1\"><i>(edit)</i></a>\n";
    echo "<pre>".$study["notes"]."</pre>\n";
  }    // end of: if _get[edit]==1
?>

  </td></tr>
</table>

<?
  //  main_form :  'action' is set to save_comment(), which submits a different form, so that user can hit
  //               enter when done editing a comment.  But to actually submit THIS form (main_form)
  //               the 'action' needs to be set to a different value before calling main_form.submit()
?>
<form name="main_form" style="margin-bottom:0px" action="javascript:save_comment()" 
      enctype="multipart/form-data" style="margin-bottom:0px">
  <input type="hidden" name="studyid" value="<? echo $studyid; ?>">
  <input type="hidden" name="MAX_FILE_SIZE" value="100000000">

<table border=0 cellpadding=0 cellspacing=1 width=100%  style="background:#555555">
  <tr class=colheads1>
    <td rowspan=2 class=colheads></td><td style="background:#bdf7d7" colspan=4>Varian fdf recon</td>
    <td style="background:#bdd7ff;" colspan=5>Custom fid2raw recon</td>
    <td rowspan=2 colspan=2 class=colheads>run</td>
    <td rowspan=2 class=colheads>comment</td>

 <td align=right rowspan=2 class=colheads>slices&nbsp;</td>
 <td align=right rowspan=2 class=colheads>vols&nbsp;</td>
 <td align=right rowspan=2 class=colheads>shots&nbsp;</td>
 <td align=right rowspan=2 class=colheads>TR/shot&nbsp;</td>
 <td align=right rowspan=2 class=colheads>TE&nbsp;&nbsp;</td>
 <td rowspan=2 class=colheads>del</td>

  </tr>
  <tr class=colheads>
  <!-- <td>&nbsp;</td> --> <!-- sel -->
  <td style="color:0; font-size:10; background:#bdf7d7" title="Varian reconstruction AFNI (HEAD/BRIK) files">AFNI</td>
  <td style="color:0; font-size:10; background:#bdf7d7" title="3dTqual - identify bad frames">q</td>
  <td style="color:0; font-size:10; background:#bdf7d7" title="Varian reconstruction Analyze (hdr/img) files">Anlyz</td>
  <td style="color:0; font-size:10; background:#bdf7df" title="Run FSL/FEAT Analysis">FSL</td> 
  <td style="color:0; font-size:10; background:#bdd7ff" title="'fid2raw' is custom reconstruction from raw fid data">fid2raw</td>
  <td style="color:0; font-size:10; background:#bdd7ff" title="Custom reconstruction using fid2raw AFNI (HEAD/BRIK) files">AFNI</td>
  <td style="color:0; font-size:10; background:#bdd7ff" title="3dTqual - identify bad frames">q</td>
  <td style="color:0; font-size:10; background:#bdd7ff" title="Custom reconstruction using fid2raw Analyze (hdr/img) files">Anlyz</td>
  <td style="color:0; font-size:10; background:#bdd7ff" title="Run FSL/FEAT Analysis">FSL</td>
<!--  <td colspan=2>run</td><td>comment</td> <td align=right>slices&nbsp;</td><td align=right>vols&nbsp;</td>
  <td align=right>shots&nbsp;</td><td align=right>TR/shot&nbsp;</td><td align=right>TE&nbsp;&nbsp;</td>
    <td>del</td>-->
    </tr>
<?

//
//  search database once for each run type ?
//  (alternative: select where studyid=id group by run_type to get types of sequences used in this study,
//   that way don't miss any unusual ones)
//

  //$q = mysql_query("SELECT * FROM fid_files WHERE run_type='epidw' AND deleted='N' AND studyid='".$_GET["studyid"]."' ORDER BY filename ASC");
// changed order from run_type to dataname:
  $q = mysql_query("SELECT * FROM fid_files WHERE deleted='N' AND length(n_no)>0 AND studyid='".$_GET["studyid"]."' ORDER BY n_no ASC, filename ASC");
  $rowsshown = 0;
  if (!$q)
    echo "query error: ".mysql_error();
  else
  {
    $i = 1;
    //echo "<tr><td colspan=4><span style=\"color:red;font-weight:bold;\">EPIs:</span></td><tr>\n";
    $rowsshown = mysql_num_rows($q);
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      $b = $bgs[$i];
      if ($id == $row["id"]) 
      {
        $b = "#ffff40"; //"#AAFFAA"; //"#20FF20";
        $processrow = $row;  // save this one, it's the one we're viewing the details of
      }
      $comment = $row["comment"];
      if (strlen($row["comment"]) == 0) $comment = "<span class='makelnk'>(edit)</span>";
      echo "<tr bgcolor=".$b." onmouseover=\"setPointer_hide(this,'over','#000000','#ccffcc');\" onmouseout=\"setPointer_hide(this,'out','".$b."','#CCFFCC');\"";  // was ffff40
      //      echo  " onmousedown=\"select_row(".$row["id"].");\"";
      echo  ">\n";
      echo "<td align=center><input name='sel_row[".$row["id"]."]' value=1 type=checkbox onclick=\"javascript:\"></td>\n";

      // FDF AFNI:
      echo "<td align=center>\n";
      if ($row["AFNI_files"] == "Y") {
        echo "<a title=\"Click to download AFNI .HEAD file\" class='makelnk' ";
        echo "href=\"download.php?what=head&id=".$row["id"]."\">.H</a>";
        echo "&nbsp;<a title=\"Click to download AFNI .BRIK file\" class='makelnk' ";
        echo "href=\"download.php?what=brik&id=".$row["id"]."\">.B</a>";
     }
      else {
        echo "<a title=\"Click to create AFNI files using to3d\" class='makelnk' href=\"javascript:recon(".$row["id"].",'afni')\">make</a>\n";
      }
      // extra td for 3dtqual here:
      echo "</td><td></td><td align=center>\n";
      // FDF ANALYZE:
      if ($row["hdrimg_files"] == "Y")
      {
        echo "<a title=\"Click to download Analyze .hdr file\" class='makelnk' ";
        echo "href=\"download.php?what=hdr&id=".$row["id"]."\">.h</a>";
        echo "&nbsp;<a title=\"Click to download Analyze .img file\" class='makelnk' ";
        echo "href=\"download.php?what=img&id=".$row["id"]."\">.i</a>";
      }
      else if ($row["AFNI_files"] == "Y")
      {
        echo "<a title=\"Click to create Analyze files using 3dAFNItoANALYZE\" class='makelnk' ";
        echo "href=\"javascript:recon(".$row["id"].",'analyze')\">make</a>\n"; 
      }
      echo "</td><td align=center>";
      if ($row["hdrimg_files"] == "Y")
      {
        if ($row["run_type"] == "epidw")
        {
          echo "<a title=\"Click to analyze data using FSL/FEAT\" class='makelnk' ";
          echo "href=\"index_study.php?view=fsl&type=fdf&studyid=".$studyid."&id=".$row["id"]."\">fsl</a>\n";
        }
        else echo "<span class='invtxt'>n/a</span>";
      }
      echo "</td><td align=center>\n";
      // FID2RAW:
      if ($row["fid2raw_files"] == "Y")
        echo "<span class='invtxt'>done</span>";
      else if ($row["run_type"] == "epidw")
        echo "<a title=\"Click for custom reconstruction of raw fid data (using fid2raw)\" class='makelnk' href=\"javascript:recon(".$row["id"].",'fid2raw')\">recon</a>\n";
      else echo "<span class='invtxt'>n/a</span>";
      echo "</td><td align=center>\n";
      // FID2RAW AFNI:
      if ($row["fid2raw_AFNI_files"] == "Y") 
      {
        echo "<a title=\"Click to download AFNI .HEAD file\" class='makelnk' ";
        echo "href=\"download.php?what=fid2rawhead&id=".$row["id"]."\">.H</a>";
        echo "&nbsp;<a title=\"Click to download AFNI .BRIK file\" class='makelnk' ";
        echo "href=\"download.php?what=fid2rawbrik&id=".$row["id"]."\">.B</a>";
      }
      else if ($row["fid2raw_files"] == "Y") 
      {
        echo "<a title=\"Click to create AFNI files using to3d\" class='makelnk' href=\"javascript:recon(".$row["id"].",'fid2raw_afni')\">make</a>\n";
      }
      // extra td for 3dtqual here:
      echo "</td><td></td><td align=center\n";
      // FID2RAW ANALYZE:
      if ($row["fid2raw_hdrimg_files"] == "Y")
      {
        echo "<a title=\"Click to download Analyze .hdr file\" class='makelnk' ";
        echo "href=\"download.php?what=fid2rawhdr&id=".$row["id"]."\">.h</a>";
        echo "&nbsp;<a title=\"Click to download Analyze .img file\" class='makelnk' ";
        echo "href=\"download.php?what=fid2rawimg&id=".$row["id"]."\">.i</a>";
      }
      else if ($row["fid2raw_AFNI_files"] == "Y")
      {
        echo "<a title=\"Click to create Analyze files using 3dAFNItoANALYZE\" class='makelnk' ";
        echo "href=\"javascript:recon(".$row["id"].",'fid2raw_analyze')\">make</a>\n"; 
      }
      echo "</td><td align=center>\n";
      if ($row["fid2raw_hdrimg_files"] == "Y")
      {
        if ($row["run_type"] == "epidw")
        {
          echo "<a title=\"Click to analyze data using FSL/FEAT\" class='makelnk' ";
          echo "href=\"index_study.php?view=fsl&type=fid2raw&studyid=".$studyid."&id=".$row["id"]."\">fsl</a>\n";
        }
        else echo "<span class='invtxt'>n/a</span>";
      }
      echo "</td>\n";
      echo "<td class=row".$i."  onmousedown=\"select_row(".$row["id"].");\">&nbsp;<a class=invlink href=\"javascript:select_row(".$row["id"].")\">";
      echo $row["dataname"]."</a></td><td onmousedown=\"select_row(".$row["id"].");\">&nbsp;".$row["n_no"]."</td>\n";

      if ($editcomment && $row["id"]==$id)
      {
        echo "<td align=left>&nbsp;&nbsp;&nbsp;\n";
        echo "<input name='new_comment' size=45 type=text value=\"".$row["comment"]."\"><input type='button' value='save' onclick='javascript:save_comment();' onchange='javascript:alert(\"change\")'>";
      }
      else
      {
        echo "<td align=left onclick=\"javascript:select_row(".$row["id"].");\">&nbsp;&nbsp;&nbsp;\n";
        echo "<a title=\"Click to edit comment\" class=invlink href=\"javascript:edit_comment(".$row["id"].")\" onmousedown=\"javascript:edit_comment(".$row["id"].")\">".$comment."</a></td>\n";
      }
      $nsl = sprintf("%d",$row["num_slices"]); if ($nsl == 0) $nsl = "&nbsp;";
      $nim = sprintf("%d",$row["num_images"]); if ($nim == 0) $nim = "&nbsp;";
      echo "<td align=right onmousedown=\"select_row(".$row["id"].");\">".$nsl."&nbsp;</td><td align=right onmousedown=\"select_row(".$row["id"].");\">".$nim."&nbsp;</td>\n";
      $nsh = sprintf("%d",$row["shots"]); if ($nsh == 0) $nsh = "&nbsp;";
      echo "<td align=right onmousedown=\"select_row(".$row["id"].");\">".$nsh."&nbsp;</td>\n";
      $tr_s = sprintf("%d",$row["tr_s"]*1000); if ($tr_s == 0) $tr_s = "&nbsp;";
      $te_s = sprintf("%.1f",$row["te_s"]*1000); if ($te_s == 0) $te_s = "&nbsp;";
      echo "<td align=right onmousedown=\"select_row(".$row["id"].");\">".$tr_s."&nbsp;</td><td align=right onmousedown=\"select_row(".$row["id"].");\">&nbsp;".$te_s."&nbsp;&nbsp;</td>\n";
      //echo "<td align=right>".$row["file_size"]."&nbsp;&nbsp;&nbsp;&nbsp;</td>\n";
      echo "<td align=center onmouseover=\"setPointerTD(this,'over','#FF0000');\" onmouseout=\"setPointerTD(this,'out','".$b."');\"><a href=\"javascript:delete_row(" . $row["id"] . ")\"><img border=0 src=\"del.png\"></a></td>\n";
      echo "</tr>\n";
      $i = 1 - $i;
    }
  }
?>
</table>
<div style="margin-top:0px; margin-bottom:15px;"></div>
<?//======================================================================================
  //
  //  TABLE OF 'OTHER FILES'
  //
  //======================================================================================

  $q = mysql_query("SELECT * from other_files WHERE studyid=\"".$studyid."\"");
  if (!$q) { echo mysql_error(); exit(); }
  $i = 1;
  if (mysql_num_rows($q) > 0)
  {
?>
<table border=0 cellpadding=0 cellspacing=1 width=100%   style="background:#555555">
  <tr class='colheads'>
    <td></td><td>type</td>
    <td>filename</td><td align=center>comment</td><td>del</td>
  </tr>
<?
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      $b = $bgs[$i];  $i = 1 - $i;
      if ($other_id == $row["other_id"]) 
      {
        $b = "#ffff40";
        $processotherrow = $row;  // save this one, it's the one we're viewing the details of
      }
?>
  <tr bgcolor=<? echo $b; ?> onmouseover="setPointer_hide(this,'over','#000000','#ccffcc');"
                             onmouseout="setPointer_hide(this,'out','<? echo $b; ?>','#CCFFCC');" style="font-size:14;">
  <td align=center><input name="other_row[<? echo $row["other_id"]; ?>]" value=1 type=checkbox></td>
  <td onmousedown="select_other_row(<? echo $row["other_id"]; ?>);">&nbsp;<? echo $row["file_type"]; ?></td>
  <td >&nbsp;<a class='invlink' href="javascript:select_other_row(<? echo $row["other_id"]; ?>);"><?
    echo $row["filename"]; ?></a><?
  if ($row["file_type"] == "AFNI" || $row["file_type"] == "3dcalc")
  {
    echo "+orig&nbsp;&nbsp;<a title=\"Click to download AFNI .HEAD file\" class='makelnk' ";
    echo "href=\"download_other.php?what=head&other_id=".$row["other_id"]."\">.H</a>";
    echo "&nbsp;&nbsp;<a title=\"Click to download AFNI .BRIK file\" class='makelnk' ";
    echo "href=\"download_other.php?what=brik&other_id=".$row["other_id"]."\">.B</a>";

    if ($row["hdrimg_files"] == "Y")
    { 
      // .hdr .img links:
      echo "&nbsp;|&nbsp;<a title=\"Click to download Analyze .hdr file\" class='makelnk' ";
      echo "href=\"download_other.php?what=hdr&other_id=".$row["other_id"]."\">.hdr</a>";
      echo "&nbsp;&nbsp;<a title=\"Click to download Analyze .img file\" class='makelnk' ";
      echo "href=\"download_other.php?what=img&other_id=".$row["other_id"]."\">.img</a>";

      // fsl link
      echo "&nbsp;|&nbsp;<a title=\"Click to analyze data using FSL/FEAT\" class='makelnk' ";
      echo "href=\"index_study.php?view=fsl&studyid=".$studyid."&other_id=".$row["other_id"]."\">fsl</a>\n";
    }
    else
    { // make analyze link
      echo "&nbsp;|&nbsp;<a title=\"Click to create Analyze hdr/img files from AFNI files\" class='makelnk' ";
      echo "href=\"javascript:recon_other(".$row["other_id"].",'other_analyze')\">make&nbsp;Anlyz</a>";
    }
  }
?>
  </td>
  <td onmousedown="select_other_row(<? echo $row["other_id"]; ?>);">&nbsp;<? echo $row["comment"]; ?></td>
  <td align=center onmouseover="setPointerTD(this,'over','#FF0000');" 
                   onmouseout="setPointerTD(this,'out','<? echo $b; ?>');">
      <a href="javascript:delete_other_row(<? echo $row["other_id"] ?>)"><img border=0 src="del.png"></td>
  </tr>
<?
    }
    echo "</table>\n";
  }
?>


<div id="fileoptionsdiv" style="border-width:1; border-style:solid; border-color:#008800; padding:6px; margin-top:14px; margin-bottom:0px; background:#f0f0fa;">

<!-- <form name="uploadfile" action="upload_file.php" method="post" style="margin-bottom:0px"> -->
<script language="javascript">
  function submit_upload_file()
  {
    bFirstFile  = document.main_form.upload_file.value  != "";
    bSecondFile = document.main_form.upload_file2.value != "";
    if (!bFirstFile) 
    { 
      alert('Please specify a file to upload first'); 
      return; 
    }
    if ((document.main_form.upload_file_type.value == "afni" ||
        document.main_form.upload_file_type.value  == "analyze")
        && !bSecondFile) 
    {
      alert('Please upload AFNI or Analyze files in pairs (either HEAD/BRIK or hdr/img)');
      return;
    }
    document.main_form.action = "upload_file.php";
    document.main_form.method = "post";
    //alert('not ready...');
    document.main_form.submit();
  }
</script>

<table cellpadding=0 cellspacing=0 border=0><tr>
<td>Upload file: </td><td><input type="file" name="upload_file" size=64>&nbsp;&nbsp;&nbsp;&nbsp;
</td><td>
type: <select name="upload_file_type">
  <option value="image">image</option>
  <option value="afni">AFNI</option>
  <option value="analyze">Analyze</option>
  <option value="other">other</option>
</select></td><td>
<input type="button" name="start_upload_btn" value=" upload " onclick="javascript:submit_upload_file();">
</td></tr><tr><td>
(2nd file if pair) </td><td><input type="file" name="upload_file2" size=64></td>
<td colspan=2>comment: <input name="upload_comment" size=35 type=text value="">
</td>
</tr></table>

<!-- </form> -->

<hr color=#b0b0ca size=1 style="margin-top:4px; margin-bottom:4px">

<!-- <form name="average_runs_form" action="average_runs.php" method="post" style="margin-bottom:0px">
  <input type="hidden" name="studyid" value="<? echo $studyid; ?>"> -->

<script language="javascript">
  function average_runs()
  {
    if (!document.main_form.avg_what[0].checked && 
        !document.main_form.avg_what[1].checked) 
    {
      alert("Average Runs: please choose which type of data to average");
      return;
    }
    document.main_form.action = "average_runs.php";
    document.main_form.method = "post";
    document.main_form.submit();
  }
</script>
<input type="button" value=" average runs " onclick="javascript:average_runs();" style="font-weight:bold;" title="uses '3dcalc' to average the runs selected above">
&nbsp;&nbsp;<span class='grplabel'>average which type of (AFNI) data:</span>
&nbsp;&nbsp;<input name="avg_what" type="radio" value="fdf" title="average data from Varian fdf reconstruction">Varian fdf reconstruction
&nbsp;&nbsp;<input name="avg_what" type="radio" value="fid2raw" title="average data from custom fid2raw reconstruction">Custom (fid2raw) reconstruction

<hr color=#b0b0ca size=1 style="margin-top:4px; margin-bottom:4px">

<input type="button" value=" zip " onclick="javascript:alert('not implemented yet');"  style="font-weight:bold;" title="select runs above, and files to the right, then 'zip' to make a gzipped tar archive" disabled>
&nbsp;&nbsp;<span class='grplabel'>zip which files:</span>
&nbsp;&nbsp;<input disabled name="fdfafni" type="checkbox" title="these are files created from the Varian .fdf images">fdf AFNI
&nbsp;&nbsp;<input disabled name="fdfhdrimg" type="checkbox" title="these are files created from the Varian .fdf images">fdf Analyze
&nbsp;&nbsp;<input disabled name="fidafni" type="checkbox" title="these are files created from the raw Varian fid data using custom reconstruction (fid2raw)">fid AFNI
&nbsp;&nbsp;<input disabled name="fidhdrimg" type="checkbox" title="these are files created from the raw Varian fid data using custom reconstruction (fid2raw)">fid Analyze

</form> <? /* END OF main_form */ ?>

</div>  <? /* end of div around upload file, average runs... */ ?>
</div>  <? /* end of big div around both files lists */ ?>
</td></tr>
<tr><td>
<?
//================================================================================================
//
//  if a fid (or file?) is selected, show info here:
//
//================================================================================================
 // no : REMINDER: THIS IS A LARGE SECTION OF CODE, search "$id != 0" to find end
// changed: no longer a long section, each subsection checks $id!=0 separately because want to allow
// $other_id!=0 as an alternative...
  if ($id != 0) 
  {
?>

<a name="details"></a>
<table cellpadding=1 cellspacing=1 border=0>
<tr><td><font color=#000088>Run:</font></td><td><b><? echo $processrow["dataname"]." - ".$processrow["n_no"]."&nbsp;&nbsp;&nbsp;&nbsp;(".$processrow["exp_date"].")"; ?></b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>
<td><font color=#000088>Matrix/FOV: </font>&nbsp;</td>
<td><? 
  $mag = 2;
  $szro = $processrow["fov_ro"]*10;
  $szpe = $processrow["fov_pe"]*10;
  echo "<b>".$processrow["mtx_ro"]." </b>x<b> ".$processrow["mtx_pe"]."&nbsp;&nbsp;</b>(<b>".$szro." </b>mm x <b>".$szpe."</b> mm)"; ?>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
</td>
<td><font color=#000088>Gain/Power: </font>&nbsp;</td>
<td><b><? echo "<b>".$processrow["gain"]." </b>/<b> ".$processrow["power"]."</b>"; ?></td>
</tr>
<tr>
<td><font color=#000088>Slices:</font>&nbsp;</td><td colspan=5>
<? echo "<b>".$processrow["num_slices"]." </b>x<b> ".$processrow["slice_thickness"]."</b> mm <b>".$processrow["orient"]."</b> slices with <b>".sprintf("%.1f",$processrow["gap"]*10)."</b> mm gap; slice offset = <b>".sprintf("%.1f",10*$processrow["slice_offset"])."</b> mm;  phase correction = <b>".strtolower($processrow["epi_pc"])."</b>";
  ?>
</td></tr>
<tr><td><font color=#000088>Data:</font>&nbsp;&nbsp;</td><td colspan=3><b><? echo $processrow["pathname"]; ?></b>
</td><td>
<? if ($viewwhat == "files" || $viewwhat == "fsl") { ?>
 <a href="index_study.php?studyid=<? echo $studyid; ?>&id=<? echo $id; ?>#details">view fdf images</a>
<? }
  echo "</td><td>";
   if ($viewwhat == "fsl" || !strlen($viewwhat)) { ?>
 &nbsp;&nbsp;&nbsp;
<a href="index_study.php?studyid=<? echo $studyid; ?>&id=<? echo $id; ?>&view=files#details">view recon results</a>
<? 
   }
?>
</td></tr>
</table>
<hr size=1 color=#CCCCCC width=99%>
<script language="javascript">
  var curimg = 1;
  var curslice = 1;
  var updating_img = 0;  // lock out new requests while processing an image update
  var imgname;

  function updateimg_onload()
  {
    document["fdf"].src = tmpimg.src;
    document.getElementById(1001).value = curimg;
    document.getElementById(1002).value = curslice;
    document.getElementById(1004).value = imgname;

    updating_img = 0;
  }

  var tmpimg = new Image();
  tmpimg.onload = updateimg_onload;

  function updateimg()
  {
    if (curimg < 1) curimg = 1;
    if (curslice < 1) curslice = 1;
<?
  $fdf = $processrow["pathname"]."/data/".$processrow["dataname"].".img/slice";
?>
    imgout = curimg;
    if (curimg < 100) { imgout = "0"+curimg; }
    if (curimg < 10)  { imgout = "00"+curimg; }
    sliceout = curslice;
    if (curslice < 100) { sliceout = "0"+curslice; }
    if (curslice < 10)  { sliceout = "00"+curslice; }
    imgname = "<? echo $fdf; ?>" + sliceout + "image" + imgout + "echo001.fdf";

    //window.stop();

    tmpimg.src = "fdf2gif.php?file=" + imgname;
  }

  var curfactor = 1;
  function updatezoom()
  {
    document["fdf"].width = <? echo ($mag*$szro); ?> * curfactor;
    document["fdf"].height = <? echo ($mag*$szpe); ?> * curfactor;
    document.getElementById(1003).value = curfactor;
  }
  function changeimage(dimg, dslice)
  {
// + operator is both addition and concatenation in js, so mult by 1 to ensure dealing with numeric types, not strings
// otherwise may end up with, e.g., curimg = "3+1" instead of curimg = 4
    if (updating_img == 1) return;
    updating_img = 1;

    curimg   = curimg*1 + dimg;
    curslice = curslice*1 + dslice;
    updateimg();
  }
  function changeimgoninput()
  {
    if (updating_img == 1) return;
    updating_img = 1;

    curimg   = document.getElementById(1001).value * 1;
    curslice = document.getElementById(1002).value * 1;
    curfactor = document.getElementById(1003).value * 1;
    updateimg(); 
    updatezoom();   
  }
  function zoomimage(factor)
  {
    if (curfactor <= 1) { 
      if (factor == -1) { curfactor /= 2.0; }
      else { curfactor *= 2.0; }
    } 
    else {
      curfactor += factor;
    }
    updatezoom();
  }
</script>
<?
  }   //END:  if ($id != 0)
//=====================================================================
//
//  VIEW RECON RESULTS:
//
//=====================================================================
  if ($viewwhat=="files" && $id!=0)
  {
?>
  <table cellpadding=1 cellspacing=1 border=0 class=recon_res>

  <tr><td colspan=3 bgcolor=#7777CC></td></tr>
  <tr ><td rowspan=3><b>to3d</b><br>Varian fdf&nbsp;</b></td>
    <td>Status:&nbsp;</td><td>&nbsp;<? echo $processrow["AFNI_files"]; ?></td></tr>
  <tr><td>Cmd:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["to3d_cmd"]; ?></td></tr>
  <tr><td>Output:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["to3d_result"]; ?></td></tr>

  <tr><td colspan=3 bgcolor=#7777CC></td></tr>
  <tr><td rowspan=3><b>AFNI -> Analyze</b><br>Varian fdf&nbsp;</b></td>
    <td>Status:&nbsp;</td><td>&nbsp;<? echo $processrow["hdrimg_files"]; ?></td></tr>
  <tr><td>Cmd:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["analyze_cmd"]; ?></td></tr>
  <tr><td>Output:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["analyze_result"]; ?></td></tr>

  <tr><td colspan=3 bgcolor=#7777CC></td></tr>
  <tr><td colspan=3 bgcolor=#7777CC></td></tr>
  <tr><td rowspan=3><b>fid2raw</b>&nbsp;&nbsp;</td>
    <td>Status:&nbsp;</td><td>&nbsp;<? echo $processrow["fid2raw_files"]; ?></td></tr>
  <tr><td>Cmd:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["fid2raw_cmd"];  ?></td></tr>
  <tr><td>Output:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["result1"]; ?></td></tr>

  <tr><td colspan=3 bgcolor=#7777CC></td></tr>
  <tr><td rowspan=3><b>to3d</b><br>custom recon&nbsp;</b></td>
    <td>Status:&nbsp;</td><td>&nbsp;<? echo $processrow["fid2raw_AFNI_files"]; ?></td></tr>
  <tr><td>Cmd:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["fid2raw_to3d_cmd"]; ?></td></tr>
  <tr><td>Output:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["result2"]; ?></td></tr>

  <tr><td colspan=3 bgcolor=#7777CC></td></tr>
  <tr><td rowspan=3><b>AFNI -> Analyze</b><br>custom recon&nbsp;</b></td>
    <td>Status:&nbsp;</td><td>&nbsp;<? echo $processrow["fid2raw_hdrimg_files"]; ?></td></tr>
  <tr><td>Cmd:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["fid2raw_analyze_cmd"]; ?></td></tr>
  <tr><td>Output:&nbsp;</td><td class=cmd2>&nbsp;<? echo $processrow["result3"]; ?></td></tr>
  <tr><td colspan=3 bgcolor=#7777CC></td></tr>

  </table>
<?
  }
  if ($viewwhat == "fsl" && ($id!=0 || $other_id !=0))
  {   
//=====================================================================
//
//  SHOW FORM FOR FSL/FEAT ANALYSIS
//
//=====================================================================
    $tr = 0;
    if ($id != 0) 
      $tr = $processrow["shots"] * $processrow["tr_s"];
    else if ($other_id != 0)
      $tr = $processotherrow["tr_s"]; 
?>
<script language="javascript">
function submit_run_fsl()
{
  if (document.run_fsl.blockA.value <= 0 || document.run_fsl.blockB.value <= 0) {
    alert("Block times must be > zero"); return;
  }
  if (document.run_fsl.TR.value <= 0) { alert("TR must be > zero"); return; }
  if (document.run_fsl.spatial_smoothing.value < 0) { alert("Smoothing kernel size must be >= zero"); return; }

  if (!confirm("This will take at least 1 minute. Please don't\nclick on anything while FSL is running.\n\nContinue?")) return;

  //document.getElementById('fsloverlay').style.position = 'fixed';
  //document.getElementById('fsloverlay').style.visibility = 'visible';
  document.run_fsl.submit();
}
</script>

<?
  $featrow = array("blockA_s"=>30, "blockB_s"=>30); // save a feat row to get last params
  $q = 0;
  if ($id != 0)
    $q = mysql_query("SELECT * FROM fsl_feat WHERE id=".$id);
  else if ($other_id != 0)
    $q = mysql_query("SELECT * FROM fsl_feat WHERE other_id=".$other_id);
  if ($id != 0 || $other_id != 0)
  {
    $i = 1;
    if (!$q) { echo mysql_error(); exit(); }
    if (mysql_num_rows($q) > 0)
    {
?>
<b>feat reports:</b><br>
<table cellpadding=1 cellspacing=1 border=0 width=100% style="font-size:14">
<tr class=colheads style="font-size:12;"><th></th>
  <th title="A block (off) / B block (on)">model</th><th>phase</th>
  <th title="spatial smoothing FWHM">smoothing</th><th>Z-thesh</th>
  <th>&nbsp;TR&nbsp;</th><th title="motion correction">&nbsp;MC&nbsp;</th>
  <th title="brain extraction">BET</th><th title="fdf=Varian recon; fid2raw=Custom recon">data type</th>
  <th align=center>zstats</th><th>feat report</th><th>del</th></tr>
<?
      while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
      {
        $featrow = $row;
        $motioncorr = $row["motion_correction"]?"Y":"-"; $bet = $row["brain_extraction"]?"Y":"-";
        $b = $bgs[$i];  $i = 1 - $i;
        echo "<tr bgcolor=".$b."><td></td><td align=center>".$row["blockA_s"]."s/".$row["blockB_s"]."s</td>\n";
        echo "<td align=center>".$row["phase_s"]." s</td>\n";
        echo "<td align=center>".$row["smoothing_mm"]." mm</td><td align=center>".$row["z_thresh"]."</td>\n";
        echo "<td align=center>".$row["tr_s"]." s</td><td align=center>".$motioncorr."</td><td align=center>".$bet."</td>\n";
        echo "<td align=center>".$row["data_type"]."</td>\n";
        echo "<td align=center>";
        $fileroot = $row["feat_report"];
        $fileroot = str_replace("report.html","stats/",$fileroot);
        echo "<a class=makelnk href='".$fileroot."zstat.hdr'>.h</a>&nbsp;<a class=makelnk href='".$fileroot."zstat.img'>.i</a>";
        echo "</td>\n"; 
        echo "<td>&nbsp;&nbsp;<a class=makelnk href=\"".$row["feat_report"]."\" target='_blank'>";
        list(,,,,,$showfeat) = explode("/",$row["feat_report"]);
        echo $showfeat."</a></td>\n";
        echo "<td align=center onmouseover=\"setPointerTD(this,'over','#FF0000');\"";
        echo " onmouseout=\"setPointerTD(this,'out','".$b."');\">\n";
        echo "<a href=\"javascript:delete_feat(".$row["feat_id"].")\"><img border=0 src='del.png'></a></td></tr>\n";
      }
?>
</table>
<?
    }
  }
?>

<br>
<form name="run_fsl" action="run_fsl.php" method="post">
<table cellpadding=0 cellspacing=0 border=0><tr><td>
<div style="border-style:solid; border-width:1; border-color:#bdc7d7;">

<input type="hidden" name="studyid" value="<? echo $studyid; ?>">
<input type="hidden" name="id" value="<? echo $id; ?>">
<input type="hidden" name="other_id" value="<? echo $other_id; ?>">
<table cellpadding=0 cellspacing=5 border=0 style="font-size:14;" bgcolor=#f0f0fa>
<tr><th colspan=4 class=colheads>FSL/FEAT Analysis</th></tr>
<tr><td><b>Model:&nbsp;&nbsp;&nbsp;</b>
   <td colspan=3>block A: <input type=text name=blockA size=5 align=right value="<? echo $featrow["blockA_s"]; ?>"> s
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;block B: <input type=text name=blockB size=5 align=right value="<? echo $featrow["blockB_s"]; ?>"> s
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;phase: <input type=text name=phase size=5 align=right value="0"> s</td>
</tr>
<tr><td><b>TR:</b></td><td><input type=text name="TR" size=5 align=right
           value="<? echo $tr; ?>"> s</td>
    <td rowspan=3 style="border-style:solid; border-width:0 0 0 0; border-color:#BBBBBB;">&nbsp;&nbsp;</td>
    <td rowspan=3>
            &nbsp;&nbsp;<input type=checkbox name="motion_correction" value=1>Motion Correction<br>
            &nbsp;&nbsp;<input type=checkbox name="brain_extraction" checked value=1>BET Brain Extraction
</tr>
<tr><td><b>Smoothing:</b></td>
    <td><input type=text name="spatial_smoothing" size=5 align=right value="5"> mm FWHM</td>
</tr>
<tr><td><b>Z threshold:</b></td>
    <td><input type=text name="z_threshold" size=5 align=right value="2.3"></td>
</tr>
<tr><td><b>Registration:</b></td>
    <td colspan=3><select name="registration">
      <option value="NONE">&nbsp;&nbsp;(no registration)&nbsp;&nbsp;</option>
    </select>
    &nbsp;&nbsp;<b>DOF:</b> <select name="dof"><option value=3>3 (translate)</option>
<option value=6>6 (rotate)</option><option value=7>7 (global scale)</option><option value=9>9 (scale)</option><option value=12>12 (shear)</option>
          </select>
    </td>
</tr>
<tr><td colspan=4><hr size=1 color=#BBBBBB width=96%></td></tr>
<tr>
<? $type = ""; if (isset($_REQUEST["type"])) $type = $_REQUEST["type"]; ?>
    <td>
<? if ($id != 0) echo "<b>which reconstruction:</b>"; ?>
    </td><td>
<? if ($id != 0) { ?>
    <select name="type">
<? if ($processrow["hdrimg_files"]=="Y") { ?>
      <option value="fdf" <? if ($type=="fdf") echo "selected"; ?>>Varian (fdf)</option>
<? } if ($processrow["fid2raw_hdrimg_files"]=="Y") { ?>
      <option value="fid2raw" <? if ($type=="fid2raw") echo "selected"; ?>>Custom (fid2raw)</option>
<? } ?>
   </select>
<? } ?>
   </td>
<td colspan=2 align=center>
      <input type="button" name="start_feat" value=" Run FSL " style="font-weight:bold"
       onclick="javascript:submit_run_fsl();"></td></tr>
</table>
</div></td></tr></table>
</form>

<input type="hidden" name="type" value="<? echo $_REQUEST["type"]; ?>">

<?
  }

  if ($viewwhat != "files" && $viewwhat != "fsl" && $id!=0)  // OTHERWISE, VIEW IMAGES
  {
//=====================================================================
//
//  FDF VIEWER:
//
//=====================================================================
?>
<form>
<table cellpadding=1 cellspacing=1 border=0 width=65%>
<tr>
<td align=center><b>Image:&nbsp;</b>
<? /* add 'readonly' to an input for read only */ ?>
<input type="button" value=" <- " onClick="javascript:changeimage(-1, 0)">
<input type="text" id=1001 value="1"  size=3 align=right style="border-width:0;" 
       onchange="javascript:changeimgoninput()">&nbsp;
<input type="button" value=" -> " onClick="javascript:changeimage(1, 0)"></td>
<td align=center><b>Slice:&nbsp;</b>
<input type="button" value=" <- " onClick="javascript:changeimage(0, -1)">
<input type="text" id=1002 value="1"  size=3 align=right style="border-width:0;"
       onchange="javascript:changeimgoninput()">&nbsp;
<input type="button" value=" -> " onClick="javascript:changeimage(0, 1)"></td>

<td align=center><b>Zoom:&nbsp;</b>
<input type="button" value=" - " onClick="javascript:zoomimage(-1)">
<input type="text" id=1003 value="1"  size=4 align=right style="border-width:0;"
       onchange="javascript:changeimgoninput()">&nbsp;
<input type="button" value=" + " onClick="javascript:zoomimage(1)"></td>
<!--
<td align=center><b>Gamma:&nbsp;</b>
<input type="button" value=" - " onClick="javascript:changegamma(-1)">
<input type="text" id=1005 value="1" size=3 align=right style="border-width:0;"
       onchange="javascript:setgamma()">&nbsp;
<input type="button" value=" + " onClick="javascript:changegamma(1)"></td> -->
</tr>
<tr><td colspan=3 align=center>
<?
  $fdf = $processrow["pathname"]."/data/".$processrow["dataname"].".img/slice001image001echo001.fdf";
  echo "<img name=fdf src=\"fdf2gif.php?file=".$fdf."\" width=".($mag*$szro)." height=".($mag*$szpe).">";
?>
<br><input type="text" readonly id=1004 value="<? echo $fdf; ?>" size=100 style="border-width:0;">
</td></tr>
</table>
</form>
<?
  }  // END, ELSE of IF view==files

  if ($other_id != 0)
  {
?>
  <br>
  <table cellpadding=1 cellspacing=1 border=0 class=recon_res>
  <tr><td valign=top><b>Command:</b>&nbsp;</font></td>
    <td><? echo $processotherrow["command"]; ?></td></tr>
  <tr><td valign=top><b>output:&nbsp;</b></td>
    <td><? echo $processotherrow["cmd_output"]; ?></td></tr>
  <tr><td><b>location:&nbsp;</b></td>
    <td><? echo $processotherrow["pathname"]; ?></td></tr>
<? if ($processotherrow["hdrimg_files"]=="Y") { ?>
  <tr><td valign=top><b>AFNI->Analyze</b>&nbsp;</td>
  <td><? echo $processotherrow["hdrimg_cmd"]; ?></td></tr>
  <tr><td valign=top><b>output:</b>&nbsp;</td>
  <td><? echo $processotherrow["hdrimg_output"]; ?></td></tr>
<? } ?>
  </table>
<?
  }   // end: if $other_id != 0
?>
</td></tr></table>
<? /* <a href="__fixdb.php">.</a> */ 
//<form name="editfid" method="get" action="edit_fid.php">
//  <input type="hidden" name="id" value="">
//</form>
?>
<form name="editfid" method="get" action="index_study.php">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <input type="hidden" name="id" value="">
  <input type="hidden" name="other_id" value="">
</form>
<form name="deletefid" method="get" action="delete_fid.php">
  <input type="hidden" name="id" value="">
  <input type="hidden" name="other_id" value="">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <input type="hidden" name="returntoindex" value=<? if ($rowsshown==1) echo "1"; else echo "0"; ?>>
</form>
<form name="deletefeat" method="get" action="delete_feat.php">
  <input type="hidden" name="feat_id" value="">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <input type="hidden" name="id" value="<? echo $id; ?>">
  <input type="hidden" name="other_id" value="<? echo $other_id; ?>">
</form>
<form name="cancel_changes" action="index_study.php" method="get">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <?/* edit==0 overrides edit on when desc or subj blank */?>
  <input type="hidden" name="edit" value=0> 
</form>
<form name="edit_comment_form" action="index_study.php" method="get">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <input type="hidden" name="id" value="">
  <input type="hidden" name="editcomment" value=1>
</form>
<form name="save_comment_form" action="save_comment.php" method="post">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <input type="hidden" name="id" value="<? echo $id; ?>">
  <input type="hidden" name="new_comment" value="">  
</form>
<form name="recon_form" action="recon_action.php" method="get">
  <input type="hidden" name="studyid" value="<? echo $_GET["studyid"]; ?>">
  <input type="hidden" name="id" value="">
  <input type="hidden" name="other_id" value="">
  <input type="hidden" name="recon_action" value="">
</form>
<form name="download_other_file" action="download_other.php" method="get">
  <input type="hidden" name="what" value="">
  <input type="hidden" name="other_id" value="">
</form>
<script language="javascript">
 function download_other(dlwhat,dlid)
 {
   document.download_other_file.what.value = dlwhat;
   document.download_other_file.other_id.value = dlid;
   document.download_other_file.submit();
 }
</script>

<div id='fsloverlay' style="position:absolute; left:0%; top:0%; width:100%; height:100%; background-color:#7777aa; opacity:0.75; border-style:none; border-color:#FFFFFF; border-width:10px; visibility:hidden; ">
<font color=#FFFF00>HELLO</font>
</div>

</body>
</html>
