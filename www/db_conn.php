<?php
$host="localhost";
$db_user="root";
$db_pass="root";
$db_name="test";
$db_port=3306;
$timezone="Asia/Shanghai";

$conn = mysqli_connect($host, $db_user, $db_pass, $db_name, $db_port);
// 检测连接
if ($conn->connect_error) {
    die("连接失败: " . $conn->connect_error);
} 