<html>
<head>
<title>Varian FID Reconstruction</title>
<link rel="stylesheet" href="styles.css" type="text/css">
</head>
<body bgcolor=#FFFFFF>
<?
require("shared.inc");
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
<br>
<div class=nav>
   <a href="discover_data.php">Discover Data</a> &nbsp; 
   <? /* | &nbsp; 
   <a href="upload_fid.php">Upload Fid</a> */ ?>
</div><br>

<script language="javascript">

  var inorout = 0;

  function select_row(whichrow)
  {
    if (inorout == 1)
      return;

    document.editfid.studyid.value = whichrow;
    document.editfid.submit();
  }
  function delete_row(whichrow)
  {
    var ans = confirm("Are you sure you want to delete this fid?");
    if (ans == false)
      return;
    document.deletefid.id.value = whichrow;
    document.deletefid.submit();
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
  // ***** TODO *****
  // CHANGE MAIN QUERY TO STUDIES TABLE (?)
  // until then, filter by subject after querying all, because subject will be removed
  // from the fid_files table eventually not that it is part of the studies table

  $filter = "";
  if (isset($_GET["subject"]))
    $filter = $_GET["subject"];
  //  $filter = " and subject=\"".$_GET["subject"]."\" ";

   //
   //   Reconstructed FIDs List:
   //
?>
<table width=99% border=0 align=center>
<tr><td>
<div class=input1>

<table border=0 cellpadding=2 cellspacing=0 width=95%><tr height=36px><td width=35px>
<img src="monkey.gif" height=32></td><td>&nbsp;&nbsp;<font size=+0><b>Studies:</b></font></td><td align=left>
<?
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());

//$q = mysql_query("SELECT subject,count(*) as numruns FROM fid_files WHERE deleted='N' GROUP BY subject");
  //$q = mysql_query("SELECT subject,count(*) as numstudies FROM studies GROUP BY subject");
  $subj_list = get_subject_list();

  echo "filter by:&nbsp;<span class=filter> ";
  //while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
  foreach ($subj_list as $subjname)
  {
    echo "<a href=index.php?subject=".$subjname.">";
    if ($filter == $subjname && strlen($subjname)) { 
      //echo "<span style='background:cyan; color:black'>".$subjname."</span>"; }
      echo "<span style='border-style:solid; border:1px solid #FF0000; padding: 0em 0.2em 0em 0.2em'>".$subjname."</span>"; }
    else
      echo $subjname;
    echo "</a>";
    echo "&nbsp;&nbsp;&nbsp;";
  }
  echo "<a href=index.php>(no filter)</a></span>";
?>
  &nbsp;&nbsp;</td></tr>
</table>

<table border=0 cellpadding=0 cellspacing=2 width=100%>
  <tr class=colheads><td>Subject</td><td>Exp. Date</td><td width=80>Runs</td><td>Description</td><td>Study/Account</td>
  </tr>
<?
  //---------
  // limiting by LENGTH(pathname)>0 selects only 'discovered' data
  $q = mysql_query("SELECT *,count(*) as numepis FROM fid_files WHERE deleted='N' AND LENGTH(pathname)>0  GROUP BY studyid DESC");
  if (!$q)
    echo "query error: ".mysql_error();
  else
  {
    $i = 1;
    $bgs = array("#d8d8e8","#e4eAe4");
    //$bgs = array("#c8c8d8","#c8d8c8");
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      $studydata = read_study_table($row["studyid"]);
      /*if (!strlen($row["comment"]))*/ $row["comment"] = $studydata["description"];
      $row["subject"] = $studydata["subject"];

      if (strlen($filter) && $row["subject"] != $filter)
        continue;

      $b = $bgs[$i];
// yellow = FFFF40 green == CCFFCC from phpmyadmin, was using BBFFBB for green though
      echo "<tr bgcolor=".$b." onmouseover=\"setPointer(this,'over','#000000','#FFFF40');\" onmouseout=\"setPointer(this,'out','".$b."','#CCFFCC');\"";
      echo  " onmousedown=\"select_row('".$row["studyid"]."');\"";
      echo  ">\n";
      //echo "<td class=row".$i.">&nbsp;<a class=invlink href=\"javascript:select_row(\"".$row["studyid"]."\")\">";
      //echo $row["client_ip"].":&nbsp;".$row["filename"]."</a></td>\n";
      echo "<td align=center>".$row["subject"]."</td>\n";
      //echo "<td align=center>".$row["shots"]."</td>\n";
      //echo "<td align=center>".$row["consecutive"]."</td>\n";
      echo "<td align=center>".$row["exp_date"]."</td>\n";
      //echo "<td align=center>".$row["client_ip"]."</td>\n";
      echo "<td align=center>".$row["numepis"]."</td>\n";
      echo "<td align=left>&nbsp;&nbsp;&nbsp;".$row["comment"]."</td>\n";
      echo "<td align=center>".$row["studyid"]."</td>\n";
      echo "</tr>\n";
      $i = 1 - $i;
    }
  }
?>
</table>
<br>
</div>
</td></tr></table>

<? /* <a href="__fixdb.php">.</a> */ ?>

<form name="editfid" method="get" action="index_study.php">  <? /* changed from index_fids.php */ ?>
  <input type="hidden" name="studyid" value="">
</form>
<form name="deletefid" method="get" action="delete_fid.php">
  <input type="hidden" name="id" value="">
</form>
</body>
</html>
