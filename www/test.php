<?php
$servername = "localhost";
$username = "root";
$password = "root";
 
// 创建连接
$conn = new mysqli($servername, $username, $password, "test");
 
// 检测连接
if ($conn->connect_error) {
    die("连接失败: " . $conn->connect_error);
} 
echo "连接成功";

$sql = "select * from tb_device";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
	while ($row = $result->fetch_assoc()) {
		echo "id: ".$row["id"]."machineid: ".$row["machineid"]."<br/>";
	}
}

$conn->close();
?>