<?php
error_reporting(E_ALL);
ini_set('display_errors', True);
@include "../config.php";
$f = $config_path.'baro.config';
$c = [];

function csv_to_array($filename='', $delimiter=',')
{
    if(!file_exists($filename) || !is_readable($filename))
        return FALSE;

    $data = array();
    if (($handle = fopen($filename, 'r')) !== FALSE)
    {
        while (($row = fgetcsv($handle, 1000, $delimiter)) !== FALSE)
        {
		if ($row[count($row)-1]=="") array_pop($row);
		array_push($data,$row);
        }
        fclose($handle);
    }
    return $data;
}

$c = csv_to_array($f,"\t");

?>

