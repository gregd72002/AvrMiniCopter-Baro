<?php
session_start();
@include("baroload.php");

if (isset($_GET['tag'])) {
        $hash = $_GET['tag'];
        $_SESSION['hash']=$hash;
        exit(0);
}

$c[0][0] = $_POST['c0_0'];
$c[0][1] = $_POST['c0_1'];

for ($i=0;$i<count($c);$i++) {
    for ($j=0;$j<count($c[$i]);$j++)
        if (is_numeric($c[$i][$j])==FALSE) {
                echo $i." ".$j." ".$c[$i][$j]."\n";
                die("Variable incorrect. Will NOT save.");
                exit(0);
        }
}

$handle = fopen($config_path.'baro.config', "w");
for ($i=0;$i<count($c);$i++) {
    for ($j=0;$j<count($c[$i]);$j++)
        fprintf($handle,"%s\t",$c[$i][$j]);
    fprintf($handle,"\n");
}

fclose($handle);

shell_exec("/etc/init.d/S92avrbaro restart > /dev/null 2>/dev/null &");

header('Location: '.$_SERVER['HTTP_REFERER'] . '#' . $_SESSION['hash']);
?>

