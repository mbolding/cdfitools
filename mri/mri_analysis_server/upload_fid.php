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

<br>
<div class=nav>
<a href="index.php">View Studies</a></div><br>

<script language="javascript">
  function upload_file()
  {
    document.fid.full_filename.value = document.fid.fid_file.value;
    if (document.fid.fid_file.value == "" ||
        document.fid.procpar_file.value == "")
    {
      alert("Select both files to upload first.");
      return;
    }
    document.fid.submit();  
  }
</script>

<?
   //
   //   Upload FID Form:
   //
?>
<table width=99% border=0 align=center>
<tr><td>
<div class=input1>

<table border=0 cellpadding=2 cellspacing=0>

 <tr height=36px>
  <td width=35px><img src="fid.gif"></td>
  <td>&nbsp;&nbsp;<font size=+1><b>Upload FID</b></font></td>
 </tr>
 <tr height=36px>
  <td colspan=2>
  <hr color=#808080 size=1>
  <form name="fid" enctype="multipart/form-data" action="upload_fid_submit.php" method="post"<?/* onSubmit="getfullfilename()"*/?>>
    <input type="hidden" name="MAX_FILE_SIZE" value="100000000">
    <input type="hidden" name="full_filename" value="">
<?
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());  
?>
<table cellspacing=10 cellpadding=4 border=0>
    <tr>
        <td><b>Subject:</b></td><td><select name="subject">
<?
  $q = mysql_query("SELECT subject FROM fid_files WHERE deleted='N' GROUP BY subject");
  if ($q)
  {
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      echo "<option value=\"".$row["subject"]."\">".$row["subject"];
    }
  }
?>
        </select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;or new subject:&nbsp;<input type=text name=newsubject size=11>
        </td>
    </tr>
    <tr><td><b>Block Times (s)</b></td><td>
              A:&nbsp;<input name="blocka" type="text" size=5 maxlength=5 value="20" align=right>
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
              B:&nbsp;<input name="blockb" type="text" size=5 maxlength=5 value="20" align=right> </td></tr>
<?
	      //    <tr><td>Notes:</td><td>
	      //              <textarea name="description" style="width:100%"></textarea>
?>
	      <? /* <input name="description" type="text" size=55 maxlength=254> */ ?>
<? /* </td></tr> */ ?>
  <tr><td><b>Specify fid file:</b></td><td><input name="fid_file" type="file" size=64></td></tr>
  <tr><td><b>Specify procpar file:</b></td><td><input name="procpar_file" type="file" size=64></td></tr>
    <tr><td colspan=2 align=center>
    <input type="button" value="Upload Files" onClick="javascript:upload_file();"></td></tr>
</table>
    </form>
  </td>
 </tr>
</table>

</div>
</td></tr>
</table><br>


</body>
</html>
