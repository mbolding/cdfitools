<?
exit();
//
//  for each line in database, set the dataname field to the part of the 'filename' field with 
//  just 'epidw_XX'
//
  $conn = @mysql_connect("localhost","root","mysqlrootpassword") or die ("error: could not connect to db");
  mysql_select_db("cdfi", $conn) or die ("error: could not select db: ".mysql_error());
  $q = mysql_query("SELECT * FROM fid_files WHERE deleted='N' ORDER BY when_uploaded DESC");
  if (!$q)
    echo "query error: ".mysql_error();
  else
  {
    while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    {
      $out_filename = $row["filename"];
      $dirlist = split('/', $out_filename);
      $out_filename = $dirlist[count($dirlist)-2];
      $out_filename = ereg_replace(".fid", "", $out_filename);

      $sql = "UPDATE fid_files SET dataname=\"".$out_filename."\" where id=".$row["id"];
      mysql_query($sql);
    }
  }

?>