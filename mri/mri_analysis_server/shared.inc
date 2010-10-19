<?
//
//  shared functions, common utilities
//

//
//  read_study_table(studyid) assumes the cdfi db is already open, and
//  returns an assoc array with the study information from the 'studies' table.
//  if studyid not found in studies table, the return value has "notfound" == 1
//
function read_study_table($studyid)
{
  $rv["notfound"] = 1;
  $rv["studyid"] = "";
  $rv["subject"] = "";
  $rv["description"] = "";
  $rv["notes"] = "";
  $q = mysql_query("SELECT * FROM studies WHERE studyid = \"".$studyid."\"");
  if (!$q) return $rv;
  if (mysql_num_rows($q) == 0) return $rv;
  $row = mysql_fetch_array($q, MYSQL_ASSOC);
  $row["notfound"] = 0;
  return $row;
}

//
//  update an existing row in the studies table
//  function assumes cdfi db already open
//  pass in array with values: studyid, subject, description, and notes
//
function update_study_table($d)
{
  $query = "UPDATE studies SET subject=\"".mysql_escape_string($d["subject"])."\", description=\"".mysql_escape_string($d["description"])."\", notes=\"".mysql_escape_string($d["notes"])."\" WHERE studyid=\"".$d["studyid"]."\"";
  $q = mysql_query($query);
  if (strlen(mysql_error())) { echo mysql_error(); exit(); }
}

function add_or_update_study($d)
{
  $q = mysql_query("SELECT studyid FROM studies WHERE studyid=\"".$d["studyid"]."\"");
  if (strlen(mysql_error())) { echo mysql_error(); exit(); }
  if (!$q) return 1;
  if (mysql_num_rows($q) > 0)
    update_study_table($d);
  else
  {
    $query = "INSERT INTO studies SET studyid=\"".$d["studyid"]."\", subject=\"".mysql_escape_string($d["subject"])."\", description=\"".mysql_escape_string($d["description"])."\", notes=\"".mysql_escape_string($d["notes"])."\"";
    $q = mysql_query($query);
    if (strlen(mysql_error())) { echo mysql_error(); exit(); }
  }
  return strlen(mysql_error());
}

//
//  get_subject_list() returns a list of subjects in the database
//  by looking in the 'fid_files' table, later this should be changed
//  to the 'studies' table.  The cdfi db is assumed to be open.
//  todo: maybe upload_fid.php should use this function too
//
function get_subject_list()
{
  $rv = array();
  //$q = mysql_query("SELECT subject FROM fid_files WHERE deleted='N' GROUP BY subject");
  $q = mysql_query("SELECT subject FROM studies GROUP BY subject");
  if (strlen(mysql_error())) { echo mysql_error(); exit(); }
  if (!$q) return $rv;
  while ($row = mysql_fetch_array($q, MYSQL_ASSOC))
    $rv[] = $row["subject"];
  return $rv;
}

  function prepare_entire_output($output)
  {
    $entire_output = "";
    foreach ($output as $line)
    {
      if (strlen(trim($line)) > 0)
      {
        $line = str_replace("&", "&amp;", $line);
        $line = str_replace("\"", "\\\"", $line);  // escape double quotes
        $entire_output .= $line."<br>";
      }
    }
    return $entire_output;
  }
?>