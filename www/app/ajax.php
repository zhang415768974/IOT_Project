<?php
session_start();
require '../db_conn.php';
$action = $_GET['action'];
if ($action == 'login') {
    $loginname = mysqli_real_escape_string($conn, trim($_POST['loginname']));
    $password = md5(trim($_POST['password']));
    $sql = "select * from tb_admin where loginname='".$loginname."' and loginpasswd='".$password."' limit 1";
    $result = mysqli_query($conn, $sql);
    if (mysqli_num_rows($result) > 0) {
        $row = mysqli_fetch_assoc($result);
        $arr['status'] = 0;
        $arr['username'] = $row['username'];
        $_SESSION["admin"] = $row['username'];
        $arr['message'] = '登陆成功';
    } else {
        $arr['status'] = 1;
        $arr['message'] = '用户名或密码错误';
    }
    $conn->close();
    echo json_encode($arr);
} elseif ($action == 'logout') {
    $_SESSION["admin"] = null;
    unset($_SESSION);
    session_destroy();
    echo "退出成功";
} elseif ($action == 'adddevice') {
	$machineid = mysqli_real_escape_string($conn, trim($_POST['machineid']));
	$model = mysqli_real_escape_string($conn, trim($_POST['model']));
	$remark = mysqli_real_escape_string($conn, trim($_POST['remark']));
	$sql = "select id from tb_device where machineid='".$machineid."' limit 1";
	$result = mysqli_query($conn, $sql);
	if (mysqli_num_rows($result) > 0) {
		$arr['status'] = 1;
		$arr['message'] = '机器码['.$machineid.']在库存中已有重复';
	} else {
		$sql = "insert into tb_device (machineid, model, remark) values ('".$machineid."','".$model."','".$remark."')";
		if (mysqli_query($conn, $sql)) {
			$arr['status'] = 0;
		} else {
			$arr['status'] = 1;
			$arr['message'] = '添加设备发生错误: ' . mysqli_error($conn);
		}
	}
	$conn->close();
	echo json_encode($arr);
} elseif ($action == 'deldevice') {
	$id = mysqli_real_escape_string($conn, trim($_POST['id']));
	$sql = "delete from tb_device where id=".$id;
	if (mysqli_query($conn, $sql)) {
		$arr['status'] = 0;
	} else {
		$arr['status'] = 1;
		$arr['message'] = '删除设备发生错误: ' . mysqli_error($conn);
	}
	$conn->close();
	echo json_encode($arr);
}