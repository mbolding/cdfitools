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
View Fids&nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;<a href="upload_fid.php">Upload Fid</a></b>
</div><br>

<script language="javascript">

  var inorout = 0;

  function select_row(whichrow)
  {
    if (inorout == 1)
      return;

    document.editfid.id.value = whichrow;
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
   //
   //   Reconstructed FIDs List:
   //
?>
<table width=99% border=0 align=center>
<tr><td>
<div class=input1>

<table border=0 cellpadding=2 cellspacing=0><tr height=36px><td width=35px>
  <img src="fid.gif" <?/*"monkey.gif" height=32*/?>></td><td>&nbsp;&nbsp;<font size=+1><b>Reconstructed fid's:</b></font></td></tr>
</table>

<table border=0 cellpadding=0 cellspacing=2 width=100%>
  <tr class=colheads><td>fid file</td><td>studyid/acct</td><td>exp. date</td><td align=right>file size</td>
    <td>&nbsp;</td>
    </tr>
<?
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $q = mysql_query("SELECT * FROM fid_files WHERE deleted='N' ORDER BY when_uploaded DESC");
  if (!$q)
    echo "query error: ".mysql_error();
  else
  {
    $i = 1;
    //$bgs = array("#F0F0F0","#D8D8D8");
    $bgs = array("#d0d0d0","#e0e0e0");
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      $b = $bgs[$i];
// yellow = FFFF40 green == CCFFCC from phpmyadmin, was using BBFFBB for green though
      echo "<tr bgcolor=".$b." onmouseover=\"setPointer(this,'over','#000000','#FFFF40');\" onmouseout=\"setPointer(this,'out','".$b."','#CCFFCC');\"";
      echo  " onmousedown=\"select_row(".$row["id"].");\"";
      echo  ">\n";
      echo "<td class=row".$i.">&nbsp;<a class=invlink href=\"javascript:select_row(".$row["id"].")\">";
      echo $row["client_ip"].":&nbsp;".$row["filename"]."</a></td>\n";
      //echo "<td align=center>".$row["shots"]."</td>\n";
      //echo "<td align=center>".$row["consecutive"]."</td>\n";
      echo "<td align=center>".$row["studyid"]."</td>\n";
      echo "<td align=center>".$row["exp_date"]."</td>\n";
      //echo "<td align=center>".$row["client_ip"]."</td>\n";
      echo "<td align=right>".$row["file_size"]."</td>\n";
      echo "<td onmouseover=\"setPointerTD(this,'over','#FF0000');\" onmouseout=\"setPointerTD(this,'out','".$b."');\"><a href=\"javascript:delete_row(" . $row["id"] . ")\"><img border=0 src=\"del.png\"></a></td>\n";
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

<form name="editfid" method="get" action="edit_fid.php">
  <input type="hidden" name="id" value="">
</form>
<form name="deletefid" method="get" action="delete_fid.php">
  <input type="hidden" name="id" value="">
</form>
</body>
</html>
